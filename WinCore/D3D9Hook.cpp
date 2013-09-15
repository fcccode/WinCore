#include "stdafx.h"

#include "D3D9Hook.h"

#pragma comment(lib, "d3d9.lib")

// VMT Index defines
#define VMTINDEX_RELEASE					2
#define VMTINDEX_BEGINSCENE					41
#define VMTINDEX_ENDSCENE					42
#define VMTINDEX_RESET						16
#define VMTINDEX_PRESENT					17
#define VMTINDEX_DRAWINDEXEDPRIMITIVE		82
#define VMTINDEX_SETTRANSFORM				44
#define VMTINDEX_GETTRANSFORM				45

namespace tcpie { namespace wincore {

std::vector<ID3D9CallbackClass*>* D3D9Hook::callbacks = new std::vector<ID3D9CallbackClass*>();

Hook* D3D9Hook::ResetHook = NULL;
Hook* D3D9Hook::LostHook = NULL;
Hook* D3D9Hook::ReleasedHook = NULL;
Hook* D3D9Hook::EndSceneHook = NULL;

DetourRet D3D9Hook::ResetDetour(DetourArgs* args)
{
	for (size_t i = 0; i < D3D9Hook::callbacks->size(); i++)
	{
		D3D9Hook::callbacks->at(i)->OnDeviceReset((IDirect3DDevice9*)args->Arguments->at(0), (D3DPRESENT_PARAMETERS*)args->Arguments->at(1));
	}

	return DETOUR_NOCHANGE;
}

DetourRet D3D9Hook::LostDetour(DetourArgs* args)
{
	if ((HRESULT)args->CustomReturnValue != D3DERR_DEVICELOST)
	{
		return DETOUR_NOCHANGE;
	}

	for (size_t i = 0; i < D3D9Hook::callbacks->size(); i++)
	{
		D3D9Hook::callbacks->at(i)->OnDeviceLost((IDirect3DDevice9*)args->Arguments->at(0));
	}

	return DETOUR_NOCHANGE;
}

DetourRet D3D9Hook::ReleasedDetour(DetourArgs* args)
{
	for (size_t i = 0; i < D3D9Hook::callbacks->size(); i++)
	{
		D3D9Hook::callbacks->at(i)->OnDeviceReleased((IDirect3DDevice9*)args->Arguments->at(0));
	}

	return DETOUR_NOCHANGE;
}

DetourRet D3D9Hook::EndSceneDetour(DetourArgs* args)
{
	for (size_t i = 0; i < D3D9Hook::callbacks->size(); i++)
	{
		D3D9Hook::callbacks->at(i)->OnEndScene((IDirect3DDevice9*)args->Arguments->at(0));
	}

	return DETOUR_NOCHANGE;
}

HWND D3D9Hook::CreateBogusWindow()
{
	return CreateWindowA("BUTTON", "TempWindow", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL);
}

HRESULT D3D9Hook::CreateDevice(IDirect3D9* D3D9Interface, HWND WindowHandle, D3DDISPLAYMODE DisplayMode, IDirect3DDevice9** CreatedDevice)
{
	// We are going to do the easiest thing: use software vertex processing :P						
	D3DPRESENT_PARAMETERS d3dpp;					// Struct to hold device info

	ZeroMemory(&d3dpp, sizeof(d3dpp));				// Clear struct for use
	d3dpp.Windowed = TRUE;	
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = WindowHandle;	
	d3dpp.BackBufferFormat = DisplayMode.Format;
	d3dpp.BackBufferWidth = 300;		
	d3dpp.BackBufferHeight = 300;

	IDirect3DDevice9* found_dev = NULL;

	HRESULT res = D3D9Interface->CreateDevice(D3DADAPTER_DEFAULT,	D3DDEVTYPE_HAL, WindowHandle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &found_dev);
		
	*CreatedDevice = found_dev;
	return res;
}

IDirect3DDevice9* D3D9Hook::GetD3D9Device()
{
	// Get pointer to D3D interface
	LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == NULL)
	{
		printf("Failed to get D3D9 interface\n");
		return NULL;
	}

	// Let's first create a temporary window
	HWND hWnd = D3D9Hook::CreateBogusWindow();

	if (hWnd == NULL)
	{
		printf("Failed to create bogus window\n");
		return NULL;
	}

	// We get the display mode
	D3DDISPLAYMODE d3dDispMode;
	HRESULT res = d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dDispMode);

	if (FAILED(res))
	{
		printf("Failed to get adapter display mode\n");
		DestroyWindow(hWnd);
		return NULL;
	}

	// We create the device
	IDirect3DDevice9* d3ddev = NULL;
	res = D3D9Hook::CreateDevice(d3d, hWnd, d3dDispMode, &d3ddev);

	d3d->Release();
	DestroyWindow(hWnd);

	if (FAILED(res))
	{
		printf("Failed to create device");
		// return res;
		return NULL;
	}
		
	return d3ddev;
}

bool D3D9Hook::CreateHooks()
{
	if (D3D9Hook::ResetHook != NULL)
	{
		return true;
	}

	IDirect3DDevice9* device = D3D9Hook::GetD3D9Device();
		
	if (device == NULL)
	{
		return false;
	}

	DWORD* vmt = (DWORD*)*(DWORD*)device;

	device->Release();

	Function* reset_fn = new Function((void*)vmt[VMTINDEX_RESET], STDCALL_CALLCONV, 2, DWORD_SIZE);
	D3D9Hook::ResetHook = Hook::CreateHook(reset_fn, new std::wstring(L"Reset"));
	D3D9Hook::ResetHook->RegisterDetour(&D3D9Hook::ResetDetour, DETOUR_PRE);

	Function* lost_fn = new Function((void*)vmt[VMTINDEX_PRESENT], STDCALL_CALLCONV, 5, DWORD_SIZE);
	D3D9Hook::LostHook = Hook::CreateHook(lost_fn, new std::wstring(L"Lost"));
	D3D9Hook::LostHook->RegisterDetour(&D3D9Hook::LostDetour, DETOUR_POST);

	Function* released_fn = new Function((void*)vmt[VMTINDEX_RELEASE], STDCALL_CALLCONV, 1, DWORD_SIZE);
	D3D9Hook::ReleasedHook = Hook::CreateHook(released_fn, new std::wstring(L"Released"));
	D3D9Hook::ReleasedHook->RegisterDetour(&D3D9Hook::ReleasedDetour, DETOUR_PRE);

	Function* endscene_fn = new Function((void*)vmt[VMTINDEX_ENDSCENE], STDCALL_CALLCONV, 1, DWORD_SIZE);
	D3D9Hook::EndSceneHook = Hook::CreateHook(endscene_fn, new std::wstring(L"EndScene"));
	D3D9Hook::EndSceneHook->RegisterDetour(&D3D9Hook::EndSceneDetour, DETOUR_PRE);

	if (D3D9Hook::ResetHook != NULL)
	{
		return true;
	}

	return false;
}

bool D3D9Hook::IsD3D9Present()
{
	HMODULE hmod = GetModuleHandleW(L"d3d9.dll");

	if (hmod != NULL)
	{
		CloseHandle(hmod);

		return true;
	}

	return false;
}

bool D3D9Hook::RegisterDetour(ID3D9CallbackClass* Callback)
{
	if (!D3D9Hook::IsD3D9Present() || Callback->IsRegistered())
	{
		return false;
	}

	if (D3D9Hook::ResetHook == NULL)
	{
		bool ret = D3D9Hook::CreateHooks();

		if (!ret)
		{
			return false;
		}
	}

	D3D9Hook::callbacks->push_back(Callback);
	Callback->is_registered = true;

	return true;
}

bool D3D9Hook::UnregisterDetour(ID3D9CallbackClass* Callback)
{
	if (!D3D9Hook::IsD3D9Present() || !Callback->IsRegistered())
	{
		return false;
	}

	for (size_t i = 0; i < D3D9Hook::callbacks->size(); i++)
	{
		if (D3D9Hook::callbacks->at(i) == Callback)
		{
			D3D9Hook::callbacks->erase(D3D9Hook::callbacks->begin() + i);

			Callback->is_registered = false;

			return true;
		}
	}

	return false;
}

} }
