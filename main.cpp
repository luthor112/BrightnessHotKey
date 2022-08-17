#include <windows.h>
#include <iostream>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <vector>
#include <utility>

#pragma comment(lib, "Dxva2")

#define RELEASE
//#define SINGLE

BOOL MonitorEnumProc(HMONITOR monitor, HDC, LPRECT, LPARAM allPhysArrays)
{
	DWORD physLen;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(monitor, &physLen))
	{
		std::wcout << L"Error querying monitors" << std::endl;
	}
	LPPHYSICAL_MONITOR physArray = (LPPHYSICAL_MONITOR)malloc(physLen * sizeof(PHYSICAL_MONITOR));
	if (!GetPhysicalMonitorsFromHMONITOR(monitor, physLen, physArray))
	{
		std::wcout << L"Error querying physical monitors" << std::endl;
	}

	((std::vector<std::pair<DWORD, LPPHYSICAL_MONITOR>>*)allPhysArrays)->push_back(std::pair<DWORD, LPPHYSICAL_MONITOR>(physLen, physArray));

	return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
	std::vector<std::pair<DWORD, LPPHYSICAL_MONITOR>> allPhysArrays;

#ifdef SINGLE
	HWND fgWin = GetForegroundWindow();
	HMONITOR monitor = MonitorFromWindow(fgWin, MONITOR_DEFAULTTONEAREST);
	MonitorEnumProc(monitor, NULL, NULL, (LPARAM)&allPhysArrays);
#else
	if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&allPhysArrays))
	{
		std::wcout << L"Error enumerating monitors" << std::endl;
		return 1;
	}
#endif
	
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

			for (auto& current : allPhysArrays)
			{
				DWORD physLen = current.first;
				LPPHYSICAL_MONITOR physArray = current.second;

				for (int i = 0; i < physLen; ++i)
				{
					DWORD minBr, currBr, maxBr;
					GetMonitorBrightness(physArray[i].hPhysicalMonitor, &minBr, &currBr, &maxBr);
					std::wcout << physArray[i].szPhysicalMonitorDescription << " has brightness " << currBr << std::endl;

					switch (msg.wParam)
					{
					case 1:
						SetMonitorBrightness(physArray[i].hPhysicalMonitor, max(minBr, currBr - 1));
						break;
					case 2:
						SetMonitorBrightness(physArray[i].hPhysicalMonitor, min(currBr + 1, maxBr));
						break;
					}

					std::wcout << L"Brightness modified" << std::endl;
				}
			}
		}
	}

	for (auto& current : allPhysArrays)
	{
		DestroyPhysicalMonitors(current.first, current.second);
		free(current.second);
	}

	return 0;
}
