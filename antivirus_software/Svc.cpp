#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "sample.h"
#include <UserEnv.h>
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#pragma comment(lib, "advapi32.lib")


#define SVCNAME TEXT("SvcName")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);


int __cdecl _tmain(int argc, TCHAR* argv[])
{
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        //return;
    }
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };
    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent((LPWSTR)TEXT("StartServiceCtrlDispatcher"));
    }
}

VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    TCHAR szPath[MAX_PATH];
    StringCchPrintf(szPath, MAX_PATH, TEXT("\"%s\""));
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }


    schService = CreateService(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    gSvcStatusHandle = RegisterServiceCtrlHandler(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent((LPWSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_SESSIONCHANGE|SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

    gSvcStatus.dwCurrentState =SERVICE_RUNNING;
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);

    //std::wofstream log("C:\\service_log.txt");

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.

    SvcInit(dwArgc, lpszArgv);
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv) {
    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL) {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    while (1) {
        // Открываем доступ к сессиям пользователей
        HANDLE hToken;
        DWORD sessionID = WTSGetActiveConsoleSessionId();

        if (WTSQueryUserToken(sessionID, &hToken)) {
            // Получаем информацию о пользователе
            TOKEN_PRIVILEGES tp;

            if (GetTokenInformation(hToken, TokenPrivileges, &tp, sizeof(tp), NULL)) {
                // Создаем процесс от имени пользователя
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);

                if (CreateProcessAsUser(hToken,
                    L"antivirus_software.exe",
                    NULL,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi)) {
                    CloseHandle(hToken);
                }
            }
        }

        if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) {
            ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
            return;
        }
    }
}


VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//DWORD WINAPI SvcCtrlHandler(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
//    
//    /*auto wtsSession = wtsSessions[i].SessionID;
//    if (wtsSession != 0) {
//        HANDLE userToken;
//        if (WTSQueryUserToken(wtsSession, &userToken)) {
//            log << L"WTSQueryUserToken TRUE" << std::endl;
//            log << L"Starting calc.exe" << std::endl;
//            WCHAR commandLine[MAX_PATH];
//            StringCchPrintf(commandLine, MAX_PATH, L"\"C:\\Windows\\System32\\calc.exe\"");
//            PROCESS_INFORMATION pi{};
//            STARTUPINFO si{};
//            si.cb = sizeof(STARTUPINFO);
//            if (CreateProcessAsUser(userToken, nullptr, commandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
//                log << L"Process started pid=" << pi.dwProcessId << std::endl;
//                CloseHandle(pi.hProcess);
//                CloseHandle(pi.hThread);
//            }
//            else {
//                log << L"CreateProcessAsUser error: " << GetLastError() << std::endl;
//            }
//            CloseHandle(userToken); // Close the user token handle when done using it
//        }
//        else {
//            log << L"WTSQueryUserToken FALSE" << std::endl;
//        }
//    }*/
//    DWORD result = ERROR_CALL_NOT_IMPLEMENTED;
//    switch (dwCtrl) {
//    case SERVICE_CONTROL_STOP:
//        result = NO_ERROR;
//        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
//
//        // Signal the service to stop.
//
//        SetEvent(ghSvcStopEvent);
//        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
//        break;
//    case SERVICE_CONTROL_SHUTDOWN:
//        gSvcStatus.dwCurrentState = SERVICE_STOPPED;
//        result = NO_ERROR;
//        break;
//    /*case SERVICE_CONTROL_SESSIONCHANGE:
//        if (dwEventType == WTS_SESSION_LOGON) {
//            WTSSESSION_NOTIFICATION* sessionNotification = static_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
//            //Add code to handle the session change event
//        }
//        break;*/
//    case SERVICE_CONTROL_INTERROGATE:
//        result = NO_ERROR;
//        break;
//    }
//    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
//    return result;
//}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.

        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

 
    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }

}

VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            SVC_ERROR,           // event identifier
            NULL,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}
void StartUiProcessInSession(DWORD wtsSession)
{
    HANDLE userToken;
    if (WTSQueryUserToken(wtsSession, &userToken))
    {

        WCHAR commandLine[] = L"\"ui.exe\"user";
        WCHAR localSystemSddl[] = L"O:SYG:SYD:";
        PROCESS_INFORMATION pi{};
        STARTUPINFO si{};

        SECURITY_ATTRIBUTES processSecurityAttributes{};
        processSecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        processSecurityAttributes.bInheritHandle = TRUE;

        SECURITY_ATTRIBUTES threadSecurityAttributes{};
        threadSecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        threadSecurityAttributes.bInheritHandle = TRUE;

        PSECURITY_DESCRIPTOR psd = nullptr;

       /*if (ConvertStringSecurityDescriptorToSecurityDescriptorW(localSystemSddl, SDDL_REVISION_1, SDDL_ & psd, ))
        {
            processSecurityAttributes.lpSecurityDescriptor = psd;
            threadSecurityAttributes.lpSecurityDescriptor = psd;

            if (CreateProcessAsUserW(
                userToken,
                NULL,
                commandLine,
                &processSecurityAttributes,
                &threadSecurityAttributes,
                FALSE,
                0,
                NULL
            ))
        }*/
    }

}


/*
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "sample.h"
#include <UserEnv.h>
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#pragma comment(lib, "advapi32.lib")


#define SVCNAME TEXT("SvcName")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);


int __cdecl _tmain(int argc, TCHAR* argv[])
{
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        //return;
    }
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };
    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent((LPWSTR)TEXT("StartServiceCtrlDispatcher"));
    }
}

VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".
    TCHAR szPath[MAX_PATH];
    StringCchPrintf(szPath, MAX_PATH, TEXT("\"%s\""));
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent((LPWSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.

    SvcInit(dwArgc, lpszArgv);
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv) {
    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL) {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    while (1) {
        // ????????? ?????? ? ??????? ?????????????
        HANDLE hToken;
        DWORD sessionID = WTSGetActiveConsoleSessionId();

        if (WTSQueryUserToken(sessionID, &hToken)) {
            // ???????? ?????????? ? ????????????
            TOKEN_PRIVILEGES tp;

            if (GetTokenInformation(hToken, TokenPrivileges, &tp, sizeof(tp), NULL)) {
                // ??????? ??????? ?? ????? ????????????
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);

                if (CreateProcessAsUser(hToken,
                    L"antivirus_software.exe",
                    NULL,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi)) {
                    CloseHandle(hToken);
                }
            }
        }

        if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) {
            ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
            return;
        }
    }
}








VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.

        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }

}

VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            SVC_ERROR,           // event identifier
            NULL,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}*/