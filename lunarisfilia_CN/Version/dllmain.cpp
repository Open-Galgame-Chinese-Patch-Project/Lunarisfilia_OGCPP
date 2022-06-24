#include "Hijack.h"
#include "Tools.h"
#include <detours.h>

HMODULE exeHmodule;
BOOL writeTable = TRUE;

typedef int (WINAPI* pMultiByteToWideChar)(
	UINT                              CodePage,
	DWORD                             dwFlags,
	_In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
	int                               cbMultiByte,
	LPWSTR                            lpWideCharStr,
	int                               cchWideChar
	);
pMultiByteToWideChar orgMultiByteToWideChar;

typedef int (WINAPI* pWideCharToMultiByte)(
	UINT                               CodePage,
	DWORD                              dwFlags,
	_In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr,
	int                                cchWideChar,
	LPSTR                              lpMultiByteStr,
	int                                cbMultiByte,
	LPCCH                              lpDefaultChar,
	LPBOOL                             lpUsedDefaultChar
);
pWideCharToMultiByte orgWideCharToMultiByte;

typedef HFONT(WINAPI* pCreateFontIndirectA)(
	const LOGFONTA* lplf
);
pCreateFontIndirectA orgCreateFontIndirectA;

typedef LPVOID(WINAPI* pVirtualAlloc)(
	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  flAllocationType,
	DWORD  flProtec
	);
pVirtualAlloc orgVirtualAlloc;

DWORD WINAPI WriteTable(LPVOID lpParameter)
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
		BYTE flag[1];
		ReadMemory((LPVOID)0x005A11C4, flag, 1);
		if (flag[0] == 0x96)
		{
			WriteMemory((LPVOID)0x005A1160, table, sizeof(table));
			break;
		}
		Sleep(1000);
	}

	return 0;
}

HFONT WINAPI newCreateFontIndirectA(LOGFONTA* lplf)
{
	//if (writeTable)
	//{
	//	WriteTable();
	//	writeTable = FALSE;
	//}

	lplf->lfCharSet = 0x86;
	strcpy_s(lplf->lfFaceName, "SimHei");
	return orgCreateFontIndirectA(lplf);
}

INT WINAPI newMultiByteToWideChar(UINT CodePage, DWORD dwFlags,PCHAR lpMultiByteStr, int cbMultiByte,LPWSTR lpWideCharStr, int cchWideChar)
{
	//std::cout << "newMultiByteToWideChar" << std::hex << CodePage << std::endl;
	if (CodePage == 0x3A4)
	{
		CodePage = 0x3A8;
	}

	return orgMultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

INT WINAPI newWideCharToMultiByte(UINT CodePage, DWORD dwFlags, PWCHAR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, PCHAR lpDefaultChar, PBOOL lpUsedDefaultChar)
{
	//std::cout << "newWideCharToMultiByte:" << std::hex << CodePage << std::endl;
	if (CodePage == 0x3A4)
	{
		CodePage = 0x3A8;
	}

	return orgWideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

LPVOID WINAPI newVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtec)
{
	LPVOID addr = orgVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtec);
	if (addr == (LPVOID)0x10000000)
	{
		BYTE patchCode[] = { 0xC2,0x0C,0x00,0x90,0x90,0x90 };
		WriteMemory((LPVOID)0x00B58D09, patchCode, sizeof(patchCode));
		DetourTransactionBegin();
		DetourDetach(&(PVOID&)orgVirtualAlloc, newVirtualAlloc);
		DetourTransactionCommit();
	}
	return addr;
}

VOID StartHook()
{
	CreateThread(NULL, NULL, WriteTable, NULL, NULL, NULL);
	orgVirtualAlloc = VirtualAlloc;
	orgCreateFontIndirectA = CreateFontIndirectA;
	//orgMultiByteToWideChar = MultiByteToWideChar;
	//orgWideCharToMultiByte = WideCharToMultiByte;

	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)orgCreateFontIndirectA, newCreateFontIndirectA);
	DetourAttach(&(PVOID&)orgVirtualAlloc, newVirtualAlloc);

	DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//SetConsole();
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
