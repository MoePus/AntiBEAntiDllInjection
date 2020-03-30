#include <iostream>
#include "Client.h"
#include "minhook/include/MinHook.h"

extern "C"
{
	NTSYSAPI NTSTATUS NTAPI NtDuplicateObject(
		IN HANDLE SourceProcessHandle,
		IN HANDLE SourceHandle,
		IN HANDLE TargetProcessHandle,
		OUT PHANDLE TargetHandle,
		IN ACCESS_MASK DesiredAccess,
		ULONG Attributes,
		ULONG Options
		);
}

bool EnableDebugPrivileges()
{
	HANDLE hToken = 0;
	TOKEN_PRIVILEGES newPrivs;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return FALSE;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &newPrivs.Privileges[0].Luid))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	newPrivs.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
	newPrivs.PrivilegeCount = 1;

	if (AdjustTokenPrivileges(hToken, FALSE, &newPrivs, 0, NULL, NULL))
	{
		CloseHandle(hToken);
		return TRUE;
	}
	else
	{
		CloseHandle(hToken);
		return FALSE;
	}

}

HANDLE RemoteCreateSection(HANDLE hFile)
{
	Client client("127.0.0.1", 94521);
	if (client.init())
	{
		byte op = 0;
		client.send(&op, 1);
		auto recv1 = client.receive();
		struct ServerInfo {
			size_t addr;
			DWORD pid;
		};
		ServerInfo si = *(ServerInfo*)recv1.second.data();
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, si.pid);
		HANDLE remoteFile;
		DuplicateHandle((HANDLE)-1, hFile, hProcess, &remoteFile, 0, 0, DUPLICATE_SAME_ACCESS);
		WriteProcessMemory(hProcess, (void*)si.addr, &remoteFile, 8, 0);
		op = 1;
		client.send(&op, 1);
		auto recv2 = client.receive();
		NTSTATUS ntstatus = *(NTSTATUS*)recv2.second.data();
		HANDLE hSection = 0;
		HANDLE remoteSection;
		ReadProcessMemory(hProcess, (void*)(si.addr + 8), &remoteSection, 8, 0);
		DuplicateHandle(hProcess, remoteSection, (HANDLE)-1, &hSection, 0, 0, DUPLICATE_SAME_ACCESS);
		return hSection;
	}
	return 0;
}

void* OriNtCreateSection;
NTSTATUS NtCreateSectionHook(
	OUT PHANDLE             SectionHandle,
	IN ULONG                DesiredAccess,
	IN PVOID				ObjectAttributes OPTIONAL,
	IN PLARGE_INTEGER       MaximumSize OPTIONAL,
	IN ULONG                PageAttributess,
	IN ULONG                SectionAttributes,
	IN HANDLE               FileHandle OPTIONAL)
{
	if (DesiredAccess == 15 && !ObjectAttributes && !MaximumSize && FileHandle && SectionHandle)
	{
		HANDLE h = RemoteCreateSection(FileHandle);
		if (h)
		{
			*SectionHandle = h;
			return 0;
		}
	}

	using FN_Type = NTSTATUS(*)(
		OUT PHANDLE             SectionHandle,
		IN ULONG                DesiredAccess,
		IN PVOID				ObjectAttributes OPTIONAL,
		IN PLARGE_INTEGER       MaximumSize OPTIONAL,
		IN ULONG                PageAttributess,
		IN ULONG                SectionAttributes,
		IN HANDLE               FileHandle OPTIONAL);
	return FN_Type(OriNtCreateSection)(SectionHandle,
		DesiredAccess,
		ObjectAttributes,
		MaximumSize,
		PageAttributess,
		SectionAttributes,
		FileHandle);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	auto ntdll = GetModuleHandleA("ntdll.dll");
	auto NtCreateSection = GetProcAddress(ntdll, "NtCreateSection");
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MH_Initialize();
		MH_CreateHook(NtCreateSection, &NtCreateSectionHook, &OriNtCreateSection);
		MH_EnableHook(NtCreateSection);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		MH_DisableHook(NtCreateSection);
		MH_RemoveHook(NtCreateSection);
		MH_Uninitialize();
	}
	return TRUE;
}
