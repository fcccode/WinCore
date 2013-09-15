#ifndef _D3D9HOOK_H_
#define _D3D9HOOK_H_

#include "Hook.h"

#include <vector>
#include <d3d9.h>

namespace tcpie { namespace wincore {

class D3D9Hook;

class __declspec(dllexport) ID3D9CallbackClass
{
	friend class D3D9Hook;

private:
	bool is_registered;

public:
	bool IsRegistered() { return this->is_registered; }

	// Executed once during service start up, and for every time the device is reset
	// Create resources that will live through a reset here (i.e. D3DPOOL_DEFAULT)
	virtual void OnDeviceReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pD3DPRESENT_PARAMETERS) = 0;

	// Executed once during service shut down, and for every time the device is lost
	// Release/delete you created in OnResetDevice here
	virtual void OnDeviceLost(IDirect3DDevice9* pDevice) = 0;

	// Executed once during service shut down
	// Release/delete you created in OnCreateDevice here
	virtual void OnDeviceReleased(IDirect3DDevice9* pDevice) = 0;

	// Executed for every frame that is rendered
	virtual void OnEndScene(IDirect3DDevice9* pDevice) = 0;
};

class __declspec(dllexport) D3D9Hook
{
private:
	static std::vector<ID3D9CallbackClass*>* callbacks;

	static HWND CreateBogusWindow();
	static HRESULT CreateDevice(IDirect3D9* D3D9Interface, HWND WindowHandle, D3DDISPLAYMODE DisplayMode, IDirect3DDevice9** CreatedDevice);
	static IDirect3DDevice9* GetD3D9Device();

	static Hook* ResetHook;
	static Hook* LostHook;
	static Hook* ReleasedHook;
	static Hook* EndSceneHook;

	static DetourRet __stdcall ResetDetour(DetourArgs* args);
	static DetourRet __stdcall LostDetour(DetourArgs* args);
	static DetourRet __stdcall ReleasedDetour(DetourArgs* args);
	static DetourRet __stdcall EndSceneDetour(DetourArgs* args);

	static bool CreateHooks();

public:
	static bool IsD3D9Present();
	static bool RegisterDetour(ID3D9CallbackClass* Callback);
	static bool UnregisterDetour(ID3D9CallbackClass* Callback);
};

} }

#endif
