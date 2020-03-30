#include <iostream>
#include "Server.h"

HANDLE g_hFile;
HANDLE g_hSection;

extern "C"
{
	typedef struct _UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		_Field_size_bytes_part_(MaximumLength, Length) PWCH Buffer;
	} UNICODE_STRING, * PUNICODE_STRING;

	typedef struct _OBJECT_ATTRIBUTES
	{
		ULONG Length;
		HANDLE RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG Attributes;
		PVOID SecurityDescriptor; // PSECURITY_DESCRIPTOR;
		PVOID SecurityQualityOfService; // PSECURITY_QUALITY_OF_SERVICE
	} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

	NTSYSAPI NTSTATUS NTAPI NtDuplicateObject(
		IN HANDLE SourceProcessHandle,
		IN HANDLE SourceHandle,
		IN HANDLE TargetProcessHandle,
		OUT PHANDLE TargetHandle,
		IN ACCESS_MASK DesiredAccess,
		ULONG Attributes,
		ULONG Options
		);

	NTSYSAPI NTSTATUS NtOpenSection(
		PHANDLE            SectionHandle,
		ACCESS_MASK        DesiredAccess,
		POBJECT_ATTRIBUTES ObjectAttributes
		);

	NTSYSAPI NTSTATUS NTAPI NtCreateSection(
		OUT PHANDLE             SectionHandle,
		IN ULONG                DesiredAccess,
		IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
		IN PLARGE_INTEGER       MaximumSize OPTIONAL,
		IN ULONG                PageAttributess,
		IN ULONG                SectionAttributes,
		IN HANDLE               FileHandle OPTIONAL); 
}

int main()
{
    Server server("127.0.0.1", 94521);
    server.init();

	std::cout << std::hex << (size_t)&g_hFile << std::endl;
    while (true)
    {
        auto remote = server.wait_remote();
		while (remote.remote_vaild())
		{
			auto received = remote.receive();
			if (received.first)
			{
				if (received.second.size() > 0)
				{
					uint8_t op = *received.second.data();
					switch (op) {
					case 0:
					{
						struct ServerInfo {
							size_t addr;
							DWORD pid;
						};
						ServerInfo si{ (size_t)&g_hFile , GetCurrentProcessId()};
						remote.send((uint8_t*)&si, sizeof(si));
						break;
					}
					case 1:
					{
						auto ntstatus = NtCreateSection(&g_hSection, SECTION_ALL_ACCESS, 0, 0, PAGE_EXECUTE, 0x1000000, g_hFile);
						remote.send((uint8_t*)&ntstatus, sizeof(ntstatus));
						remote.Close();
						break;
					}
					default:
					{
					}
					}
				}
			}
		}
    }
    std::cout << "Hello World!\n";
}

