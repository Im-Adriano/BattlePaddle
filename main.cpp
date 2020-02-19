#include "BPHelper.hpp"

#if defined(_WIN32) || defined(WIN32)
#include "windows.h"
#include <iostream>
#endif

using namespace std;

#ifdef __unix__

int main() {
    BPHelper bpHelper = BPHelper();
    bpHelper.requestActionThread();
    bpHelper.Receive();
}

#elif defined(_WIN32) || defined(WIN32)

HANDLE thr;

extern "C" __declspec(dllexport) DWORD WINAPI StartMain(LPVOID data) {
BPHelper bpHelper = BPHelper();
bpHelper.requestActionThread();
bpHelper.Receive();
return 0;
}

__declspec(dllexport) BOOL WINAPI DllMain(IN HINSTANCE hModule,
        IN DWORD  ul_reason_for_call,
        IN LPVOID lpReserved
) {
switch (ul_reason_for_call) {
case DLL_PROCESS_ATTACH:
CreateThread(NULL, 0, &StartMain, (LPVOID)NULL, 0, NULL);
break;
case DLL_THREAD_ATTACH:
break;
case DLL_THREAD_DETACH:
case DLL_PROCESS_DETACH:
break;
}

return TRUE;
}

#endif


