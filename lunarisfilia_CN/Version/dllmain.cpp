#include "Hijack.h"
#include "Tools.h"
#include "detours.h"

BOOL writeTable = TRUE;

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
		BYTE flag[1]{};
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
	lplf->lfCharSet = 0x86;
	strcpy_s(lplf->lfFaceName, "SimHei");
	return orgCreateFontIndirectA(lplf);
}

LPVOID WINAPI newVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtec)
{
	if (dwSize == 0x00545000) //alloc size of sarcheck.dll
	{
		DWORD patchAddr = (DWORD)0x00B58D09; //entrypoint rva
		BYTE entryPoint[3]{};
		ReadMemory((LPVOID)patchAddr, entryPoint, sizeof(entryPoint));
		if (entryPoint[0] == 0x51 && entryPoint[1] == 0xB9 && entryPoint[2] == 0x06) // check entrypoint
		{
			BYTE patchCode[] = { 0xC2,0x0C,0x00 };
			WriteMemory((LPVOID)patchAddr, patchCode, sizeof(patchCode));
			DetourTransactionBegin();
			DetourDetach(&(PVOID&)orgVirtualAlloc, newVirtualAlloc);
			DetourTransactionCommit();
		}
	}

	return orgVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtec);
}

VOID StartHook()
{
	CreateThread(NULL, NULL, WriteTable, NULL, NULL, NULL);
	orgVirtualAlloc = VirtualAlloc;
	orgCreateFontIndirectA = CreateFontIndirectA;

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
