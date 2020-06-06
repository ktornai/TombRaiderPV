#pragma once

namespace WinAPI
{
	//process
	TRProcess GetRunningGameProc();
	bool OpenTRProcess(TRProcess& trProc);
	void CloseTRProcess(const TRProcess& trProc);

	bool ReadTRProcessMemory(const TRProcess& trProc, uint32_t address, uint8_t* buffer, uint32_t size);

	bool IsTRStillRunning(const TRProcess& trProc);

	//console
	void ClearConsole();
}