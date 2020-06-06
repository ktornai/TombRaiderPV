#include "pch.h"
#include "winapi.h"

namespace WinAPI
{
	//process
	TRProcess GetRunningGameProc()
	{
		TRProcess trProc;
		HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if(snapshot == INVALID_HANDLE_VALUE)
			return trProc;

		PROCESSENTRY32 procEntry;
		ZeroMemory(&procEntry, sizeof(procEntry));
		procEntry.dwSize = sizeof(procEntry);

		const std::vector<std::wstring> tombProcNames = 
		{
			std::wstring(L"Tomb1.exe"),
			std::wstring(L"Tomb2.exe"),
			std::wstring(L"Tomb3.exe")
		};

		if(::Process32First(snapshot, &procEntry))
		{
			do
			{
				uint32_t idx = 1; //TRGame::TombRaider_1 == 1
				for(auto& procName: tombProcNames)
				{
					if(std::wstring(procEntry.szExeFile) == procName)
					{
						trProc.game = static_cast<TRGame>(idx);
						trProc.pid = procEntry.th32ProcessID;
						break;
					}

					idx++;
				}
			} while(::Process32Next(snapshot, &procEntry));
		}

		::CloseHandle(snapshot);

		return trProc;
	}

	bool OpenTRProcess(TRProcess& trProc)
	{
		assert(trProc.game != TRGame::Unknown && trProc.pid != 0);
		HANDLE procHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, trProc.pid);

		if(procHandle != INVALID_HANDLE_VALUE)
		{
			trProc.procHandle = procHandle;
			return true;
		}

		return false;
	}

	void CloseTRProcess(const TRProcess& trProc)
	{
		if(trProc.procHandle != INVALID_HANDLE_VALUE)
			::CloseHandle(trProc.procHandle);
	}

	bool ReadTRProcessMemory(const TRProcess& trProc, uint32_t address, uint8_t* buffer, uint32_t size)
	{
		assert(trProc.procHandle != INVALID_HANDLE_VALUE);

		SIZE_T numberOfBytesRead;
		BOOL r = ::ReadProcessMemory(trProc.procHandle, reinterpret_cast<LPCVOID>(address), reinterpret_cast<LPVOID>(buffer), static_cast<SIZE_T>(size), &numberOfBytesRead);

		if(r == FALSE || numberOfBytesRead != size)
			return false;

		return true;
	}

	bool IsTRStillRunning(const TRProcess& trProc)
	{
		assert(trProc.procHandle != INVALID_HANDLE_VALUE);

		DWORD exitCode;
		BOOL ret = ::GetExitCodeProcess(trProc.procHandle, &exitCode);
		
		return (ret != FALSE && exitCode == STILL_ACTIVE);
	}

	//console
	void ClearConsole()
	{
		const HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut == INVALID_HANDLE_VALUE)
			return;

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		std::cout.flush();

		if(::GetConsoleScreenBufferInfo(hOut, &csbi) != FALSE)
		{
			DWORD len = csbi.dwSize.X * csbi.dwSize.Y;
			DWORD written = 0;
			COORD topLeft { 0, 0 };

			::FillConsoleOutputCharacter(hOut, 32, len, topLeft, &written);
			::FillConsoleOutputAttribute(hOut, csbi.wAttributes, len, topLeft, &written);
			::SetConsoleCursorPosition(hOut, topLeft);
		}
	}
}