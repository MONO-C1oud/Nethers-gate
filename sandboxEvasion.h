#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cstdlib>
#include <filesystem>  // Requires C++17

// Windows-specific headers
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include <tchar.h>
#pragma comment(lib, "Advapi32.lib")
#include <intrin.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <winternl.h>


// Linker directives: Ensure these libraries are linked
// For Visual Studio: wbemuuid.lib, Shlwapi.lib, psapi.lib

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "rpcrt4.lib")


#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")  // Needed for GetScaleFactorForMonitor()

using namespace std;
namespace fs = std::filesystem;

// ----- Stub Implementations for Missing Functions -----

vector<string> getDrivePaths() {
    vector<string> drives;

    // Get the size needed for the drive strings buffer using the ANSI version.
    DWORD bufferSize = GetLogicalDriveStringsA(0, NULL);
    if (bufferSize == 0)
        return drives; // error or no drives found

    char* buffer = new char[bufferSize];
    if (GetLogicalDriveStringsA(bufferSize, buffer)) {
        char* drive = buffer;
        while (*drive) {
            drives.push_back(string(drive));
            drive += strlen(drive) + 1;
        }
    }
    delete[] buffer;
    return drives;
}



BOOL IsRunningAsAdministrator() {
    BOOL isAdmin = FALSE;
    PSID pAdminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    // Create a SID for the Administrators group.
    if (AllocateAndInitializeSid(&NtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdminGroup)) {
        // Check if the token of the calling process is a member of the Administrators group.
        if (!CheckTokenMembership(NULL, pAdminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(pAdminGroup);
    }
    return isAdmin;
}


BOOL InitWMI(IWbemServices** pSvc, IWbemLocator** pLoc, const TCHAR* namespaceStr)
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return FALSE;

    hr = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)pLoc
    );
    if (FAILED(hr)) return FALSE;

    hr = (*pLoc)->ConnectServer(
        _bstr_t(namespaceStr), NULL, NULL, 0, NULL, 0, 0, pSvc
    );
    if (FAILED(hr)) return FALSE;

    // Optionally set proxy blanket, etc.
    // If all succeeded, return TRUE
    return TRUE;
}


BOOL ExecWMIQuery(IWbemServices** pSvc, IWbemLocator** pLoc,
    IEnumWbemClassObject** pEnumerator, const TCHAR* query)
{
    if (!pSvc || !(*pSvc)) {
        return FALSE;
    }

    // Convert TCHAR* to BSTR if needed
    // Or use the same logic you have in other WMI calls.
    HRESULT hr = (*pSvc)->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        pEnumerator
    );
    if (FAILED(hr)) {
        return FALSE;
    }

    // If we get here, *pEnumerator should now be valid.
    return TRUE;
}


// ----- Function Definitions -----

//sandbox evasion
void driveSpaceCheck() {

    ULARGE_INTEGER totalFreeBytes = { 0 };
    ULARGE_INTEGER maxFreeBytes = { 0 };
    ULARGE_INTEGER total_bytes;
    ULARGE_INTEGER free_bytes;
    uint64_t total_space = 0;

    string max_volume = "";
    vector<string> drives = getDrivePaths();


    for (string rootPath : drives) {
        if (GetDiskFreeSpaceExW(std::wstring(rootPath.begin(), rootPath.end()).c_str(), &free_bytes, &total_bytes, &totalFreeBytes)) {
            total_space += total_bytes.QuadPart;
        }
        else {
            std::cerr << "Error getting disk space for drive " << rootPath << std::endl;
        }
    }

    // Print the drive with the maximum free space
    uint64_t space_in_gbs = total_space / (1024 * 1024 * 1024);
    cout << "Total disk space: " << space_in_gbs << " GB" << endl;

    if (space_in_gbs < 100) {
        cout << "[-] Running a rat race... (Disk space)" << endl;
        exit(0);
        return;
    }
    else {
        cout << "[+] Woohooooo! (Disk Space)" << endl;
        return;
    }

    return;
}

void CheckWMIError(HRESULT hr, const std::string& errorMessage) {
    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcerr << L"Error: " << errorMessage.c_str() << L" - " << err.ErrorMessage() << std::endl;
        exit(1);
    }
}


void getCpuTemperature() {
    //cout << "[+] called getCpuTemperature... " << endl;
    double temp = 0;

    if (IsRunningAsAdministrator()) {
        std::cout << "The program is running with administrator privileges." << std::endl;
    }
    else {
        std::cout << "The program is not running with administrator privileges. Skipping CPU temperature check" << std::endl;
        return;
    }

    HRESULT hr;

    // Initialize COM
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    CheckWMIError(hr, "Failed to initialize COM");
    cout << "[+] COM initialized successfully!" << endl;

    // Set general COM security levels
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    CheckWMIError(hr, "Failed to initialize COM security");
    cout << "[+] COM security initialized successfully!" << endl;

    // Obtain initial locator to WMI
    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc
    );
    CheckWMIError(hr, "Failed to create IWbemLocator object");
    cout << "[+] Created IWebmLocator object successfully!" << endl;

    // Connect to WMI
    IWbemServices* pSvc = NULL;
    hr = pLoc->ConnectServer(
        _bstr_t(L"root\\wmi"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );
    CheckWMIError(hr, "Could not connect to WMI");
    cout << "[+] Connected to WMI successfully!" << endl;

    // Set security levels on the proxy
    hr = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    CheckWMIError(hr, "Could not set proxy blanket");
    cout << "[+] Proxy blanket set successfully!" << endl;

    // Query for CPU temperature
    IEnumWbemClassObject* pEnumerator = NULL;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("select CurrentTemperature from MSAcpi_ThermalZoneTemperature"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    CheckWMIError(hr, "Query for temperature probe failed");
    cout << "[+] CPU temperature query probe succeeded!" << endl;

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        cout << "[+] Got the next item in the query" << endl;
        if (0 == uReturn) {
            cout << "[-] Failed to get the next item in the query" << endl;
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            // The temperature is in tenths of Kelvin, so convert to Celsius
            temp = (static_cast<double>(vtProp.uintVal) - 2731.0) / 10.0;
            std::wcout << L"Current Temperature : " << temp << L" C" << std::endl;
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    if (temp == 0) {
        cout << "[-] Running a rat race... (CPU Temperature)" << endl;
        exit(0);
    }
    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return;
}


//sandbox evasion
void checkDllNames() {
    // Define sandbox DLLs as a vector of strings
    //SbieDll.dll, Dbghelp.dll, Api_log.dll, or Dir_watch.dll 
    vector<string> sandboxDLLs = { 
        // Sandboxing
        "sbiedll.dll", "api_log.dll", "dir_watch.dll", "snxhk.dll", 
        // VMware DLLs
        "unity.dll", "vm3dgl.dll", "vm3dgl64.dll", "vm3dum.dll", "vmappcfg.dll", "vmcryptolib.dll",
        "vmguestlib.dll", "vmguestlibjava.dll", "vmhgfs.dll", "vmnetbridge.dll", "vmwarebase.dll",
        "vmx_fb.dll", "vmx_mode.dll", "vnetinst.dll", "vnetlib.dll", "vsocklib.dll",
        // VirtualBox DLLs
        "VBoxGuest.dll", "VBoxSF.dll", "VBoxTray.dll", "VBoxOGL.dll"
    };

    DWORD loadedProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    // Get all PIDs
    if (!EnumProcesses(loadedProcesses, sizeof(loadedProcesses), &cbNeeded)) {
        printf("[---] Could not get all PIDs, exiting.\n");
        getchar();
        exit(-1);
    }

    // Calculate how many PIDs returned
    cProcesses = cbNeeded / sizeof(DWORD);

    // Check all loaded DLLs
    HANDLE hProcess;
    int evidenceCount = 0;

    for (i = 0; i < cProcesses; i++) {
        HMODULE hMods[1024];

        // Get a handle to the process.
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, loadedProcesses[i]);
        if (hProcess != NULL) {
            // Get a list of all the modules in this process.
            if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                for (unsigned int j = 0; j < (cbNeeded / sizeof(HMODULE)); j++) {
                    CHAR szModName[MAX_PATH];
                    // Get the full path to the module's file.
                    if (GetModuleFileNameExA(hProcess, hMods[j], szModName, sizeof(szModName))) {
                        // Iterate through the sandbox DLLs to check for matches
                        for (const auto& dll : sandboxDLLs) {
                            if (strstr(szModName, dll.c_str())) {
                                CHAR processName[MAX_PATH];
                                GetProcessImageFileNameA(hProcess, processName, MAX_PATH);
                                cout << "Process name: " << processName << endl;
                                cout << "DLL loaded: " << szModName << endl;
                                ++evidenceCount;
                                break; // Found a match, no need to check other DLLs
                            }
                        }
                    }
                }
            }
            CloseHandle(hProcess);
        } // if hProcess != NULL     
    } // for each process

    if (evidenceCount == 0) {
        cout << "[+] Woohooooo! (DLL Names)" << endl;
    }
    else {
        cout << "[-] Running a rat race... (DLL Names)" << endl;
        exit(0);
    }
    return;
}

//sandbox evasion
void CheckProcessNames() {
    const std::vector<std::string> sandboxProcesses = {
        "vmsrvc", "tcpview", "wireshark", "visual basic", "fiddler",
        "vbox", "process explorer", "autoit", "vboxtray",
        "vmtools", "vmrawdsk", "vmusbmouse", "vmvss", "vmscsi",
        "vmxnet", "vmx_svga", "vmmemctl", "df5serv", "vboxservice",
        "vmhgfs"
    };
    /*
    vmware - authd.exe
    vmware - usbarbitrator64.exe
    vmware.exe
    vmware - tray.exe
    vmware - unity - helper.exe
    vmware - vmx.exe
    */

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not create snapshot, exiting.\n";
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Could not retrieve information about processes, exiting.";
        CloseHandle(hProcessSnap);
        return;
    }

    int evidenceCount = 0;
    do {
        std::string processName = std::string(pe32.szExeFile, pe32.szExeFile + _tcslen(pe32.szExeFile));

        for (const auto& sandboxProcess : sandboxProcesses) {
            if (processName.find(sandboxProcess) != std::string::npos) {
                std::cout << processName << std::endl;
                ++evidenceCount;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    if (evidenceCount == 0) {
        cout << "[+] Woohooooo! (Process Names)" << endl;
    }
    else {
        cout << "[-] Running a rat race... (Process Names)" << endl;
        exit(0);
    }

    CloseHandle(hProcessSnap);
    return;
}
//sandbox evasion
void checkFilePaths() {
    vector<string> filePaths = {
        // VMware files
        "C:\\windows\\System32\\Drivers\\Vmmouse.sys",
        "C:\\windows\\System32\\Drivers\\vm3dgl.dll",
        "C:\\windows\\System32\\Drivers\\vmdum.dll",
        "C:\\windows\\System32\\Drivers\\vm3dver.dll",
        "C:\\windows\\System32\\Drivers\\vmtray.dll",
        "C:\\windows\\System32\\Drivers\\vmci.sys",
        "C:\\windows\\System32\\Drivers\\vmusbmouse.sys",
        "C:\\windows\\System32\\Drivers\\vmx_svga.sys",
        "C:\\windows\\System32\\Drivers\\vmxnet.sys",
        "C:\\windows\\System32\\Drivers\\VMToolsHook.dll",
        "C:\\windows\\System32\\Drivers\\vmhgfs.dll",
        "C:\\windows\\System32\\Drivers\\vmmousever.dll",
        "C:\\windows\\System32\\Drivers\\vmGuestLib.dll",
        "C:\\windows\\System32\\Drivers\\VmGuestLibJava.dll",
        "C:\\windows\\System32\\Drivers\\vmscsi.sys",

        // VirtualBox files
        "C:\\windows\\System32\\Drivers\\VBoxMouse.sys",
        "C:\\windows\\System32\\Drivers\\VBoxGuest.sys",
        "C:\\windows\\System32\\Drivers\\VBoxSF.sys",
        "C:\\windows\\System32\\Drivers\\VBoxVideo.sys",
        "C:\\windows\\System32\\vboxdisp.dll",
        "C:\\windows\\System32\\vboxhook.dll",
        "C:\\windows\\System32\\vboxmrxnp.dll",
        "C:\\windows\\System32\\vboxogl.dll",
        "C:\\windows\\System32\\vboxoglarrayspu.dll",
        "C:\\windows\\System32\\vboxoglcrutil.dll",
        "C:\\windows\\System32\\vboxoglerrorspu.dll",
        "C:\\windows\\System32\\vboxoglfeedbackspu.dll",
        "C:\\windows\\System32\\vboxoglpackspu.dll",
        "C:\\windows\\System32\\vboxoglpassthroughspu.dll",
        "C:\\windows\\System32\\vboxservice.exe",
        "C:\\windows\\System32\\vboxtray.exe",
        "C:\\windows\\System32\\VBoxControl.exe"
    };

    int evidenceCount = 0;

    for (const auto& path : filePaths) {
        wstring widePath(path.begin(), path.end());  // Convert string to wstring

        if (PathFileExistsW(widePath.c_str())) {
            cout << "[!] Found Suspicious File: " << path << endl;
            ++evidenceCount;
        }
    }

    // Improved decision logic
    if (evidenceCount == 0) {
        cout << "[+] No suspicious VM-related files found. (checkFilePaths)" << endl;
    }
    else if (evidenceCount <= 2) {
        cout << "[!] Some VM-related files found, but likely a normal system. Continuing execution." << endl;
    }
    else {
        cout << "[-] Multiple VM indicators found! This is likely a virtual environment. Exiting... (checkFilePaths)" << endl;
        exit(0);
    }
}

BOOL get_services(_In_ SC_HANDLE hServiceManager, _In_ DWORD serviceType, _Out_ ENUM_SERVICE_STATUS_PROCESS** servicesBuffer, _Out_ DWORD* serviceCount)
{
    DWORD serviceBufferSize = 1024 * sizeof(ENUM_SERVICE_STATUS_PROCESS);
    ENUM_SERVICE_STATUS_PROCESS* services = static_cast<ENUM_SERVICE_STATUS_PROCESS*>(malloc(serviceBufferSize));

    if (serviceCount) //assume failure
        *serviceCount = 0;

    if (services) {

        SecureZeroMemory(services, serviceBufferSize);

        DWORD remainderBufferSize = 0;
        DWORD resumeHandle = 0;
        if (EnumServicesStatusEx(hServiceManager, SC_ENUM_PROCESS_INFO, serviceType, SERVICE_STATE_ALL, (LPBYTE)services, serviceBufferSize, &remainderBufferSize, serviceCount, &resumeHandle, NULL) != 0)
        {
            // success and we enumerated all the services
            *servicesBuffer = services;
            return TRUE;
        }

        DWORD lastError = GetLastError();
        if (lastError == ERROR_MORE_DATA)
        {
            // we didn't get all the services, so we'll just re-enumerate all to make things easy
            serviceBufferSize += remainderBufferSize;

            ENUM_SERVICE_STATUS_PROCESS* tmp;

            tmp = static_cast<ENUM_SERVICE_STATUS_PROCESS*>(realloc(services, serviceBufferSize));
            if (tmp) {
                services = tmp;
                SecureZeroMemory(services, serviceBufferSize);
                if (EnumServicesStatusEx(hServiceManager, SC_ENUM_PROCESS_INFO, serviceType, SERVICE_STATE_ALL, (LPBYTE)services, serviceBufferSize, &remainderBufferSize, serviceCount, NULL, NULL) != 0)
                {
                    *servicesBuffer = services;
                    return TRUE;
                }
            }
        }
        else
        {
            printf("ERROR: %u\n", lastError);
        }

        free(services);

    }
    return FALSE;
}

//sandbox evasion
void checkRegistryKeys() {
    HKEY hKey;
    int evidenceOfSandbox = 0;

    const char* sandboxStrings[5] = { "VMWare", "virtualbox", "vbox", "qemu", "xen" };

    const char* HKLM_Keys_To_Check_Exist[7] = { "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0\\Identifier",
        "SYSTEM\\CurrentControlSet\\Enum\\SCSI\\Disk&Ven_VMware_&Prod_VMware_Virtual_S",
        "SYSTEM\\CurrentControlSet\\Control\\CriticalDeviceDatabase\\root#vmwvmcihostdev",
        "SYSTEM\\CurrentControlSet\\Control\\VirtualDeviceDrivers",
        "SOFTWARE\\VMWare, Inc.\\VMWare Tools",
        "SOFTWARE\\Oracle\\VirtualBox Guest Additions",
        "HARDWARE\\ACPI\\DSDT\\VBOX_" };

    const char* HKLM_Keys_With_Values_To_Parse[6][2] = {
    { "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "0" },
    { "HARDWARE\\Description\\System", "SystemBiosInformation" },
    { "HARDWARE\\Description\\System", "VideoBiosVersion" },
    { "HARDWARE\\Description\\System\\BIOS", "SystemManufacturer" },
    { "HARDWARE\\Description\\System\\BIOS", "SystemProductName" },
    { "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0", "Logical Unit Id 0" }
    };

    for (int i = 0; i < 7; ++i) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, HKLM_Keys_To_Check_Exist[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            printf("%s\n", HKLM_Keys_To_Check_Exist[i]);
            RegCloseKey(hKey);
            ++evidenceOfSandbox;
        }
    }

    for (int i = 0; i < 6; ++i) {
        HKEY hKey;
        TCHAR buff[1024] = { 0 };
        DWORD buffSize = 1024;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, HKLM_Keys_With_Values_To_Parse[i][0], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, HKLM_Keys_With_Values_To_Parse[i][1], NULL, NULL, (LPBYTE)buff, &buffSize) == ERROR_SUCCESS) {
                for (int j = 0; j < 5; ++j) {
                    std::wstring wSandboxString(sandboxStrings[j], sandboxStrings[j] + strlen(sandboxStrings[j]));
                    if (StrStrIW((LPWSTR)buff, wSandboxString.c_str()) != NULL) {
                        printf("%s\\%s --> %s \n", HKLM_Keys_With_Values_To_Parse[i][0], HKLM_Keys_With_Values_To_Parse[i][1], buff);
                        ++evidenceOfSandbox;
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }

    if (evidenceOfSandbox == 0) {
        cout << "[+] Woohooooo! (checkRegistryKeys)" << endl;
    }
    else {
        cout << "[-] Running a rat race... (checkRegistryKeys)" << endl;
        exit(0);
    }

    return;
}

extern "C" void callcpuid();
#pragma intrinsic(__rdtsc)

//sandbox evasion
void CpuIdAndRdtsc() {
    unsigned long long int time1, time2, sum = 0;
    const unsigned char avg = 100;
    INT CPUInfo[4] = { -1 };

    auto start = chrono::high_resolution_clock::now(); // Start timing

    for (int i = 0; i < avg; i++) {
        time1 = __rdtsc();  // Read CPU cycles before CPUID
        __cpuid(CPUInfo, 1); // Execute CPUID instruction
        time2 = __rdtsc();  // Read CPU cycles after CPUID
        sum += time2 - time1;
    }

    auto end = chrono::high_resolution_clock::now(); // End timing
    chrono::duration<double> elapsed = end - start; // Compute time difference

    sum = sum / avg; // Compute average cycles per CPUID execution

    cout << "Ticks on average: " << sum << endl;
    cout << "Elapsed Time: " << elapsed.count() << " seconds" << endl;

    // New dynamic thresholds to reduce false positives
    if ((sum < 700 && elapsed.count() < 0.00005) || (sum < 500)) {
        cout << "[-] Running a rat race... (CpuIdAndRdtsc - Sandbox Detected)" << endl;
        exit(0);
    }
    else {
        cout << "[+] Woohooooo! (CpuIdAndRdtsc - Normal System)" << endl;
    }
}

//sandbox evasion
void cpuid_is_hypervisor() {
    INT CPUInfo[4] = { -1 };
    bool isHypervisor = false;

    /* Query CPUID with EAX = 1 to check hypervisor bit */
    __cpuid(CPUInfo, 1);

    if ((CPUInfo[2] >> 31) & 1) {
        isHypervisor = true;
    }

    // Additional check: Get hypervisor vendor string (EAX = 0x40000000)
    char hyperVendor[13] = { 0 };
    __cpuid(CPUInfo, 0x40000000);
    memcpy(hyperVendor, &CPUInfo[1], 4);
    memcpy(hyperVendor + 4, &CPUInfo[2], 4);
    memcpy(hyperVendor + 8, &CPUInfo[3], 4);

    // Fix: Allow Hyper-V but detect VMware, VirtualBox, or Xen
    if (isHypervisor) {
        cout << "[!] Hypervisor Detected: " << hyperVendor << endl;

        if (strcmp(hyperVendor, "Microsoft Hv") == 0) {
            cout << "[+] Hyper-V detected, but running on a real machine. No sandbox." << endl;
        }
        else {
            cout << "[-] Running a rat race... (cpuid_is_hypervisor)" << endl;
            exit(0);
        }
    }
    else {
        cout << "[+] Woohooooo! (cpuid_is_hypervisor)" << endl;
    }
}


//sandbox evasion
void checkScreenResolution() {
    // Get the screen resolution
    /*RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetClientRect(hDesktop, &desktop);

    int screenWidth = desktop.right;
    int screenHeight = desktop.bottom;*/

    // Get the full screen resolution (not just the client area)
   // Get the screen width and height using GetDeviceCaps
    // Make the application DPI-aware
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    // Get monitor handle
    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);

    DEVICE_SCALE_FACTOR scaleFactor;
    if (GetScaleFactorForMonitor(hMonitor, &scaleFactor) != S_OK) {
        std::cerr << "Failed to get DPI scale factor!" << std::endl;
        return;
    }

    // Get actual screen resolution (physical pixels)
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Debugging: Print detected screen resolution
    std::cout << "Detected Screen Resolution: " << screenWidth << "x" << screenHeight << std::endl;

    /*
    -    1920x1080    -    1920x1200    -    1920x1600   -   1920x900
    -    2560x1080    -    2560x1200    -    2560x1600
    -    1440x1080    -    1440x1200    -    1440x1600

    */

    // Check if the resolution is 800x600 or 1024x768
    //1366x768
    if ((screenWidth == 1920 && screenHeight == 900) ||
        (screenWidth == 1920 && screenHeight == 1080) ||
        (screenWidth == 1920 && screenHeight == 1200) ||
        (screenWidth == 1920 && screenHeight == 1600) ||
        (screenWidth == 2560 && screenHeight == 1080) ||
        (screenWidth == 2560 && screenHeight == 1200) ||
        (screenWidth == 2560 && screenHeight == 1600) ||
        (screenWidth == 1440 && screenHeight == 1080) ||
        (screenWidth == 1440 && screenHeight == 1200) ||
        (screenWidth == 1440 && screenHeight == 1600) ||
        (screenWidth == 1366 && screenHeight == 768)
        ) {
        std::cout << "[+] Woohooooo! (checkScreenResolution)" << std::endl;
    }
    else {
        cout << "[-] Running a rat race... (checkScreenResolution)" << endl;
        exit(0);
    }
    return;
}

void ConvertToWideString(const char* input, wchar_t* output, int outputSize) {
    MultiByteToWideChar(CP_ACP, 0, input, -1, output, outputSize);
}

//sandbox evasion
void checkVMDriverServices()
{
    const int KnownServiceCount = 10;
    bool is_fishy = false;

    // Use wide strings explicitly
    const wchar_t* KnownVMServices[KnownServiceCount] = {
        L"VBoxWddm",
        L"VBoxSF",      // VirtualBox Shared Folders
        L"VBoxMouse",   // VirtualBox Guest Mouse
        L"VBoxGuest",   // VirtualBox Guest Driver
        L"vmhgfs",      // VMWare Host Guest Control Redirector
        L"vmmouse",
        L"vmmemctl",    // VMWare Guest Memory Controller Driver
        L"vmusbmouse",
        L"vmx_svga",
        L"vmxnet"
    };

    SC_HANDLE hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCM != NULL)
    {
        ENUM_SERVICE_STATUS_PROCESS* services = NULL;
        DWORD serviceCount = 0;
        if (get_services(hSCM, SERVICE_DRIVER, &services, &serviceCount))
        {
            bool ok = true;
            for (DWORD i = 0; i < serviceCount; i++)
            {
                // Convert `lpServiceName` to wide string if needed
                wchar_t wideServiceName[256];
                MultiByteToWideChar(CP_ACP, 0, services[i].lpServiceName, -1, wideServiceName, 256);

                for (int s = 0; s < KnownServiceCount; s++)
                {
                    if (StrCmpIW(wideServiceName, KnownVMServices[s]) == 0)
                    {
                        wcout << L"[!] Detected VM Service: " << KnownVMServices[s] << endl;
                        ok = false;
                        break;
                    }
                }
            }
            free(services);
            is_fishy = !ok;
        }
        else
        {
            printf("Failed to get services list.\n");
            // Treat failure to enumerate as suspicious
            is_fishy = true;
        }
        CloseServiceHandle(hSCM);
    }
    else
    {
        printf("Failed to get SCM handle.\n");
        is_fishy = true;
    }

    if (!is_fishy) {
        cout << "[+] Woohooooo! (checkVMDriverServices)" << endl;
    }
    else {
        cout << "[-] Running a rat race... (checkVMDriverServices)" << endl;
        exit(0);
    }
}

//sandbox evasion
void checkCPUCores()
{
    int cores = 8;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);  // Retrieve CPU information

    INT detectedCores = sysInfo.dwNumberOfProcessors;

    std::cout << "[INFO] Detected CPU Cores: " << detectedCores << std::endl;

    if (detectedCores <= cores) {
        std::cout << "[-] Running a rat race... (Low CPU Core Count)" << std::endl;
        exit(0);  // Exit the program if cores are too low
    }
    else {
        std::cout << "[+] Woohooooo! (CPU Cores OK)" << std::endl;
    }
}

//sandbox evasion
void checkDebugger()
{
    PPEB pPEB = (PPEB)__readgsqword(0x60); // Access PEB (Process Environment Block)

    if (pPEB->BeingDebugged)  // Check if BeingDebugged flag is set
    {
        std::cout << "[!] A debugger is present. Exiting..." << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "[+] No debugger detected. Proceeding..." << std::endl;
    }
}


