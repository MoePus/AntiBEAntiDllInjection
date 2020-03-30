# AntiBEAntiDllInject
## Inject DLL into BE protected process without disturb its minifilter

## Concept
* LoadLibrary calls NtCreateSection which raises IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION in minifilter.
* BE checks the dll through its minifilter.
* So we can inject any dll just by remove NtCreateSection from LoadLibrary.

## Usage
* Compile the project
* Mannulmap Client.dll to your BEd process.
* Open Server.exe
* Now normally Inject your dll.

## Reference
* https://bbs.pediy.com/thread-226525.htm

## Info
**This project is only a POC**

*AND I Know minifilter can be bypass with a kernel hijacking or a ring3 hooking.*