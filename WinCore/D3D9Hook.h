/*
Copyright (c) 2013 Stijn "tcpie" Hinterding (contact: contact at tcpie dot eu)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/// @file D3D9Hook.h
/// @author tcpie
/// @brief Contains code relevant to the D3D9Hook class.

#ifndef _D3D9HOOK_H_
#define _D3D9HOOK_H_

#include "Hook.h"

#include <windef.h>
#include <vector>
#include <d3d9.h>

namespace tcpie { namespace wincore {

class D3D9Hook;

/// @class IDirect3DDevice9
/// @brief The Direct 3D 9 device.
///
/// For documentation, consult the DirectX SDK.

/// @struct D3DPRESENT_PARAMETERS
/// @brief The D3D present parameters.
///
/// For documentation, consult the DirectX SDK.

/// @brief Base class to inherit from when registering for D3D9 callbacks
///
/// This class's methods will be called when the right D3D9 functions are called.
/// You need to inherit from this class, to be able to register a D3D9 detour.
class __declspec(dllexport) ID3D9CallbackClass
{
	friend class D3D9Hook;

private:
	bool is_registered;

public:
	ID3D9CallbackClass() { this->is_registered = false; }

	/// @brief	Gets if the class instance is a registered detour.
	/// @return Whether or not the instance is a registered detour.
	bool IsRegistered() { return this->is_registered; }

	/// @brief	This function is called when Reset() is called on the D3D9 device.
	/// @param	pDevice						A pointer to the D3D9 device.
	/// @param	pD3DPRESENT_PARAMETERS		The D3D present parameters.
	///
	/// Executed once during service start up, and for every time the device is reset
	/// Create resources that will live through a reset here (i.e. D3DPOOL_DEFAULT)
	virtual void OnDeviceReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pD3DPRESENT_PARAMETERS) = 0;

	/// @brief	This function is called when the D3D device is lost.
	/// @param	pDevice						A pointer to the D3D9 device.
	///
	/// Executed once during service shut down, and for every time the device is lost
	/// Release/delete you created in OnResetDevice here
	virtual void OnDeviceLost(IDirect3DDevice9* pDevice) = 0;

	/// @brief	This function is called when the D3D device is released.
	/// @param	pDevice						A pointer to the D3D9 device.
	///
	/// Executed once during service shut down
	/// Release/delete you created in OnCreateDevice here
	virtual void OnDeviceReleased(IDirect3DDevice9* pDevice) = 0;

	/// @brief	This function is called when EndScene() is called.
	/// @param	pDevice						A pointer to the D3D9 device.
	///
	/// Executed for every frame that is rendered. If you would like to render graphics,
	/// do it in this function.
	virtual void OnEndScene(IDirect3DDevice9* pDevice) = 0;
};

/// @class D3D9Hook
/// @brief Class which handles the hooking of D3D9
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
	/// \fn			IsD3D9Present
	/// @brief		Gets whether d3d9.dll is present
	/// @return		A value indicating if d3d9.dll is present
	static bool IsD3D9Present();

	/// \fn			RegisterDetour
	/// @brief		Allows one to register a detour to D3D9
	/// @param		Callback	The callback class instance to call when the hooked D3D9 functions are executed.
	/// @return		A value indicating if detour registration was successful. This may fail for several reasons:
	///				- D3D9 may not be present
	///				- The detour was already registered
	///				- D3D9 hooking failed
	static bool RegisterDetour(ID3D9CallbackClass* Callback);

	/// \fn			UnregisterDetour
	/// @brief		Allows one to unregister a detour to D3D9
	/// @param		Callback	The callback class instance to unregister
	/// @return		A value indicating whether unregistration was successful. This may fail if the detour was not yet registered.
	static bool UnregisterDetour(ID3D9CallbackClass* Callback);
};

} }

#endif
