WinCore
=======

C++ framework for x86 Windows to ease modding

=======

I like the .Net framework. Not because of easy memory management that comes with managed code, but because of the nice API it comes with. Process, Thread, File, and a lot more classes provide some great functionality. When writing C++, I felt I was still stuck writing boiler plate code every time...

No more! The goal of WinCore is to combine the power of native code with the sweetness of a pre-built API. The focus is on making complex stuff (hooking a function, calling functions in other processes, etc) easy.

I'll let the code do the talking. An example how the frameworks makes hooking a function easy:
	
	// Target function
	int __cdecl add(int a, int b)
	{
		return a + b;
	}
	
	// Our detour
	DetourRet __stdcall add_detour(DetourArgs* args)
	{
		args->Arguments->at(0) = (void*)4;
			
		return DETOUR_ARGCHANGED;
	}

	// ...
	
	// Initialize Function
	Function* target = new Function(add, CDECL_CALLCONV, 2, DWORD_SIZE);
	
	// Initialize Hook
	Hook* hook = Hook::CreateHook(target, new wstring(L"add"));
	
	// Register detour
	Detour* our_detour = hook->RegisterDetour(add_detour, DETOUR_PRE);
	
	hook->Enable();
	
	// Output: "9"
	cout << add(2, 5) << endl;								
	
=======
