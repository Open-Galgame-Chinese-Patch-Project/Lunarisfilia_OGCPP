#pragma once
#include "Lib.h"

static FILE* streamconsole;

DWORD WINAPI SetConsoleTop(LPVOID lpParameter)
{
	WCHAR consoleTitle[256] = { 0 };

	while (true)
	{
		GetConsoleTitleW(consoleTitle, 256);
		HWND hConsole = FindWindowW(NULL, (LPWSTR)consoleTitle);
		if (hConsole != NULL)
		{
			SetWindowPos(hConsole, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			break;
		}
	}

	return 0;
}

VOID WINAPI SetConsole()
{
	AllocConsole();
	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	freopen_s(&streamconsole, "CONIN$", "r+t", stdin);
	freopen_s(&streamconsole, "CONOUT$", "w+t", stdout);
	SetConsoleTitleW(L"Hijack Test!");

	CreateThread(NULL, NULL, SetConsoleTop, NULL, NULL, NULL);

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & ~ENABLE_QUICK_EDIT_MODE);

	std::locale::global(std::locale(""));
}

VOID WriteHookCode(DWORD oldAddr, DWORD tarAddr, DWORD lenWritten, BOOL modeSwitch)
{
	DWORD oldProtect;
	VirtualProtect((LPVOID)oldAddr, lenWritten, PAGE_EXECUTE_READWRITE, &oldProtect);

	DWORD rawAddr = tarAddr - oldAddr - 5;
	BYTE code[5] = { 0 };

	if (modeSwitch)
	{
		code[0] = 0xE9;
	}
	else
	{
		code[0] = 0xE8;
	}

	memcpy(&code[1], &rawAddr, 4);
	memcpy((void*)oldAddr, code, 5);

	BYTE nop = 0x90;
	for (size_t i = 5; i < lenWritten; i++)
	{
		memcpy((void*)(oldAddr + i), &nop, sizeof(nop));
	}
}

DWORD MemSearch(DWORD beginAddr, VOID* searchCode, INT lenOfCode, BOOL clockWise)
{
	for (beginAddr; 1; )
	{
		if (beginAddr < 0x7FFF0000 && beginAddr > 0x00010000)
		{
			DWORD oldProtect = 0;
			VirtualProtect((LPVOID)beginAddr, lenOfCode, PAGE_EXECUTE_READWRITE, &oldProtect);
			if (!memcmp(searchCode, (void*)beginAddr, lenOfCode))
			{
				return beginAddr;
			}

			if (clockWise)
			{
				beginAddr++;
			}
			else
			{
				beginAddr--;
			}
		}
		else
		{
			return 0;
		}
	}
}

VOID WriteMemory(LPVOID lpAddress, LPCVOID lpBuffer, SIZE_T nSize)
{
	DWORD oldProtect;

	if (!VirtualProtectEx(GetCurrentProcess(),lpAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		MessageBoxW(NULL, L"VirtualProtectEx Failed!!", NULL, MB_OK);
	}
	else if (!WriteProcessMemory(GetCurrentProcess(), lpAddress, lpBuffer, nSize, NULL))
	{
		MessageBoxW(NULL, L"WriteProcessMemory Failed!!", NULL, MB_OK);
	}
}

VOID ReadMemory(LPVOID lpAddress, LPVOID lpBuffer, SIZE_T nSize)
{
	DWORD oldProtect;

	if (!VirtualProtectEx(GetCurrentProcess(), lpAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		MessageBoxW(NULL, L"VirtualProtectEx Failed!!", NULL, MB_OK);
	}

	if (!ReadProcessMemory(GetCurrentProcess(), lpAddress, lpBuffer, nSize, NULL))
	{
		MessageBoxW(NULL, L"ReadMemory Failed!!", NULL, MB_OK);
	}
}