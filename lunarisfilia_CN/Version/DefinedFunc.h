#pragma once
#include <Windows.h>

typedef HANDLE(WINAPI* pCreateFileW)(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile);
pCreateFileW rawCreateFileW = CreateFileW;

typedef HFONT(WINAPI* pCreateFontIndirectA)(
	const LOGFONTA* lplf);
pCreateFontIndirectA rawCreateFontIndirectA = CreateFontIndirectA;

typedef LPVOID(WINAPI* pVirtualAlloc)(
	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  flAllocationType,
	DWORD  flProtec);
pVirtualAlloc rawVirtualAlloc = VirtualAlloc;