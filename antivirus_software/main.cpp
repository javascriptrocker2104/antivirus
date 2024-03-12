/*#pragma comment(lib,"Credui.lib")
#include<Windows.h>
#include<iostream>
#include<wincred.h>

std::wstring GetLastErrorAsString()
{
	//
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::wstring();//
	}

	LPWSTR messageBuffer = nullptr;
//
//

size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,// | FORMAT_MESSAGE,
	NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

//Copy the error message into a std::string.
std::wstring message(messageBuffer, size);

//Free the Win32's string's buffer.
LocalFree(messageBuffer);

return message;
}

bool Authenticate_ADMIN_User(std::wstring caption, std::wstring msg, int maxReAsks = 0)
{
	std::wcin.imbue(std::locale(".866"));
	std::wcout.imbue(std::locale(".866"));

	CREDUI_INFOW credui = {};
	credui.cbSize = sizeof(credui);
	credui.hwndParent = nullptr;
	credui.pszMessageText = msg.c_str();
//
	//
	//
	ULONG authPackage = 0,
	   outCredSize = 0;
	LPVOID outCredBuffer = nullptr;
	BOOL save = false;

	DWORD err = 0;
	int tries = 0;

	bool reAsk = false;

	do
	{
		tries++;

		if (CredUIPromptForWindowsCredentialsW(&credui,
			err,
			&authPackage,
			nullptr,
			0,
			&outCredBuffer,
			&outCredSize,
			&save,
			CREDUIWIN_ENUMERATE_ADMINS)

			!= ERROR_SUCCESS)
			return false;


		ULONG cchUserName = 0;
		ULONG cchPassword = 0;
		ULONG cchDomain = 0;
		ULONG cchNeed, cchAllocated = 0;

		static volatile UCHAR guz = 0;

		PWSTR stack = (PWSTR)alloca(guz);
		PWSTR szUserName = nullptr, szPassword = nullptr, szDomainName = nullptr;

		BOOL ret;

		do {
			if (cchAllocated < (cchNeed = cchUserName + cchPassword + cchDomain))
			{
				szUserName = (PWSTR)alloca((cchNeed - cchAllocated) * sizeof(WCHAR));
				cchAllocated = (ULONG)(stack - szUserName);
				szPassword = szUserName + cchUserName;
				szDomainName = szPassword + cchPassword;
			}

			ret = CredUnPackAuthenticationBuffer(
				CRED_PACK_PROTECTED_CREDENTIALS, outCredBuffer, outCredSize, szUserName, &cchUserName,
				szDomainName, &cchDomain, szPassword,
				&cchPassword);

		} while (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER);


		SecureZeroMemory(outCredBuffer, outCredSize);
		CoTaskMemFree(outCredBuffer);

		HANDLE token = nullptr;

		if (LogonUser(szUserName,
			szDomainName,
			szPassword,
			LOGON32_LOGON_INTERACTIVE,
			LOGON32_PROVIDER_DEFAULT,
			&token))
		{

			if (ImpersonateLoggedOnUser(token))
			{
				WCHAR commandLine[] = L"\"ui.exe\"user";
				PROCESS_INFORMATION pi{};
				STARTUPINFO si{};

				if (CreateProcessWithLogonW(
					szUserName,
					szDomainName,
					szPassword,
					0,
					NULL,
					commandLine,
					0,
					NULL,
					NULL,
					&si,
					&pi)
					//
					//
					//
					//
					//
					//
					//
					//
					//
					//
					//
					)
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
				else
				{
					std::wcout << L"Не удалось запустить процесс: " << GetLastError() << L": " << GetLastErrorAsString;
				}
				RevertToSelf();
			}
			//
			//
			//
			//

			CloseHandle(token);
			return true;
			}

			else
			{
				err = ERROR_LOGON_FAILURE;
				reAsk = true;
			}


	}while (reAsk && tries < maxReAsks);

	return false;
}

int main(int argc, char** argv)
{
	Authenticate_ADMIN_User(L"caption", L"msg", 1);
	return 0;
}

void f()
{
	CREDUI_INFO cui;
	TCHAR pszName[CREDUI_MAX_USERNAME_LENGTH + 1];
	TCHAR pszPwd[CREDUI_MAX_PASSWORD_LENGTH + 1];
	BOOL fSave;
	DWORD dwErr;

	cui.cbSize = sizeof(CREDUI_INFO);
	cui.hwndParent = NULL;
	//
	//
	cui.pszMessageText = TEXT("Enter administration account information");
	cui.pszCaptionText = TEXT("CredUITest");
	cui.hbmBanner = NULL;
	fSave = FALSE;
	SecureZeroMemory(pszName, sizeof(pszName));
	SecureZeroMemory(pszPwd, sizeof(pszPwd));

	dwErr = CredUICmdLinePromptForCredentialsW(TEXT("TheServer"),
		//
		NULL,
		0,
		pszName,
		CREDUI_MAX_USERNAME_LENGTH + 1,
		pszPwd,
		CREDUI_MAX_PASSWORD_LENGTH + 1,
		& fSave,
		0);

#if 0
		dwErr = CredUICmdLinePromptForCredentials(
			&cui,
			TEXT("TheService"),
			//
			NULL,
			0,
			pszName,
			CREDUI_MAX_USERNAME_LENGTH + 1,
			pszPwd,
			CREDUI_MAX_PASSWORD_LENGTH + 1,
			&fSave,
			CREDUI_FLAGS_GENERIC_CREDENTIALS |
			CREDUI_FLAGS_ALWAYS_SHOW_UI |
			CREDUI_FLAGS_DO_NOT_PERSIST);
#endif
		if (!dwErr)
		{
			dwErr == ERROR_INVALID_PARAMETER;
			//
			std::wcout << L"Hello " << pszName;
		}
}*/






/*int main(int argc, char** argv)
{
	for (int i = 0;i < argc;++i)
		std::cout << argv[i] << " ";
	std::cout << std::endl;
	system("whoami/ALL");
	system("pause");
	//return 0;
	int InstallService(std::wstring sn, std::wstring sp) {}
	int wmain(int argv, wchar_t** argv)
	{
		for (int i = 0;i < argc;++i)
			std::wcout << L"\"" << argv[i] << L"\";
			if (argc > 1)
			{
				if (std::wstring(argv[1]) == L"--install")
				{
					InstallService(serviceName, argv[0]);
					return 0;
				}
			}
		SERVICE_TABLE_ENTRYW ServiceTable[2];
		ServiceTable[0].lpServiceName = serviceName;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		ServiceTable[1].lpServiceName = NULL;
		ServiceTable[1].lpServiceProc = NULL;

		if (!StartServiceDispatcherW(ServiceTable)) {
			std::cerr << "Error:StartServiceCtrlDispatcher:" << GetLastError();
		}
	*/
	/*//winlogon.exe
   {
	   auto wtsSession = wtsSessions[i].SessionID;
	   if (wtsSession != 0)
	   {
		   HANDLE userToken;
		   if (WTSQueryUserToken(wtsSession, &userToken))
		   {
			   log << L"WTSQueryUserToken TRUE" << std::endl;
			   log << L"Starting calc.exe" << std::endl;
			   WCHAR commandLine[] = L"\"hdjhwek";
			   PROCESS_INFORMATION pi{};
			   STARTUPINFO si{};
			   if (CreateProcessW(
				   NULL,
				   commandLime,
				   NULL,
				   NULL,
				   FALSE,
				   0,
				   NULL,
				   NULL,
				   &si,
				   &pi)
				   )
			   {
				   log << L"Process started pid=" << pi.dwProcessId << std::endl;
			   }
			   else
			   {
				   log << L"CreateProcessAsUserW error: " << GetListError() << std::endl;
			   }
		   }
	   }
   }*/

		/*{
	std::wcout.imbue(std::locale(".866"));
	std::wcerr.imbue(std::locale(".866"));
	std::wcin.imbue(std::locale(".866"));

	if (argc - 1 == 0)
	{
		auto tracer = std::static_pointer_cast <ITracer>(std::make_shared<ConsoleTracer>());
		AntimalwareServiceFactory serviceFactory(tracer);
		InitService(tracer, serviceFactory);
	}
	else 
	{
		ConsoleTracer tracer;
		ServiceManager sm{ tracer,u"AntimalwareService",reinterpret_cast<char16_t*>(argv[0]) };
		if (wcscmp(argv[argv - 1], L"--install") == 0) {
			sm.InstallService();
		}
		else if (wcscmp(argv[argv - 1], L"--uninstall") == 0) {
			sm.UninstallService();
		}
}*/