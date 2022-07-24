#include "HijackDLL.h"
#include "DefinedFunc.h"
#include "detours.h"

VOID WINAPI WriteTable()
{
	BYTE table[] =
	{
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
	};

	while (true)
	{
		DWORD* flag = (DWORD*)0x005A11C4;
		if (*flag == 0x77073096)
		{
			memcpy((void*)0x005A1160, table, sizeof(table));
			break;
		}
		Sleep(1000);
	}
}

HANDLE WINAPI newCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	static LPCWSTR tarFileName1 = L".\\pac\\update1.ypf";
	static LPCWSTR tarFileName2 = L".\\pac\\bn.ypf";
	static LPCWSTR repFileName1 = L".\\Version.pack1";
	static LPCWSTR repFileName2 = L".\\Version.pack2";
	if (!wcscmp(tarFileName1, lpFileName))
	{
		lpFileName = repFileName1;
	}
	if (!wcscmp(tarFileName2, lpFileName))
	{
		lpFileName = repFileName2;
	}

	return rawCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HFONT WINAPI newCreateFontIndirectA(LOGFONTA* lplf)
{
	lplf->lfCharSet = 0x86;
	strcpy_s(lplf->lfFaceName, "SimHei");
	return rawCreateFontIndirectA(lplf);
}

LPVOID WINAPI newVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtec)
{
	if (dwSize == 0x00545000) //alloc size of sarcheck.dll
	{
		if (*(DWORD*)0x00B58D09 == 0x0006B951) // check entrypoint
		{
			*(DWORD*)0x00B58D09 = 0x90000CC2; // entrypoint => ret 0xC
			DetourTransactionBegin();
			DetourDetach(&(PVOID&)rawVirtualAlloc, newVirtualAlloc);
			DetourTransactionCommit();
		}
	}

	return rawVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtec);
}

VOID StartHook()
{
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WriteTable, NULL, NULL, NULL);

	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)rawCreateFontIndirectA, newCreateFontIndirectA);
	DetourAttach(&(PVOID&)rawVirtualAlloc, newVirtualAlloc);
	DetourAttach(&(PVOID&)rawCreateFileW, newCreateFileW);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		StartHook();
		CreateHijack();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		FreeHijack();
		break;
	}
	return TRUE;
}
