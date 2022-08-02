#include <windows.h>
#include <iostream>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

#pragma comment(lib, "Dxva2")

#define RELEASE

int wmain(int argc, wchar_t* argv[])
{
	HWND fgWin = GetForegroundWindow();
	HMONITOR monitor = MonitorFromWindow(fgWin, MONITOR_DEFAULTTONEAREST);
	DWORD physLen;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(monitor, &physLen))
	{
		std::wcout << L"Error querying monitors" << std::endl;
		return 1;
	}
	LPPHYSICAL_MONITOR physArray = (LPPHYSICAL_MONITOR)malloc(physLen * sizeof(PHYSICAL_MONITOR));
	if (!GetPhysicalMonitorsFromHMONITOR(monitor, physLen, physArray))
	{
		std::wcout << L"Error querying physical monitors" << std::endl;
		return 1;
	}

	if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, VK_F9) &&
		RegisterHotKey(NULL, 2, MOD_ALT | MOD_NOREPEAT, VK_F10))
	{
		std::wcout << L"Hotkeys registered, using MOD_NOREPEAT flag" << std::endl;
	}
	else
	{
		std::wcout << L"Error registering hotkey" << std::endl;
		return 1;
	}

#ifdef RELEASE
	if (!FreeConsole())
	{
		std::wcout << L"Could not detach from console" << std::endl;
	}
#endif

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_HOTKEY)
		{
			std::wcout << L"WM_HOTKEY received" << std::endl;

			DWORD minBr, currBr, maxBr;
			GetMonitorBrightness(physArray[0].hPhysicalMonitor, &minBr, &currBr, &maxBr);
			std::wcout << physArray[0].szPhysicalMonitorDescription << " (#0 of " << physLen << ") has brightness " << currBr << std::endl;

			switch (msg.wParam)
			{
			case 1:
				SetMonitorBrightness(physArray[0].hPhysicalMonitor, max(minBr, currBr - 1));
				break;
			case 2:
				SetMonitorBrightness(physArray[0].hPhysicalMonitor, min(currBr + 1, maxBr));
				break;
			}

			std::wcout << L"Brightness modified" << std::endl;
		}
	}

	DestroyPhysicalMonitors(physLen, physArray);
	free(physArray);

	return 0;
}
