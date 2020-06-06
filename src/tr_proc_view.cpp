#include "pch.h"

using TR2ConstAddress = TRConstants<TRGame::TombRaider_2>::MemAddr;

TRProcess trProc;

template<typename T>
std::optional<T> ReadFromProcessMemory(TR2ConstAddress e)
{
	T value;

	if(!WinAPI::ReadTRProcessMemory(trProc, static_cast<uint32_t>(e), reinterpret_cast<uint8_t*>(&value), sizeof(T)))
		return std::nullopt;

	return { value };
}

uint32_t GetDistanceTravelled()
{
	const auto distanceFromTRproc = ReadFromProcessMemory<uint32_t>(TR2ConstAddress::Distance_Travelled);
	if(!distanceFromTRproc.has_value())
		return false;

	const uint32_t distanceValue = distanceFromTRproc.value();
	uint32_t distanceTravelled;

	//MSVC always emits imul instead of mul
	__asm
	{
		mov eax, 0x93459BE7
		mul distanceValue
		shr edx, 8
		mov distanceTravelled, edx
	}

	return distanceTravelled;
}

std::string GetTimeTakenFormatted()
{
	const auto timeTakenTRproc = ReadFromProcessMemory<uint32_t>(TR2ConstAddress::Time_Taken);
	if(!timeTakenTRproc.has_value())
		return {};

	const uint32_t timeTakenValue = timeTakenTRproc.value();
	uint32_t timeTakenSeconds;

	__asm
	{
		mov eax, 0x88888889
		mul timeTakenValue
		mov ecx, edx
		mov eax, 0x91A2B3C5
		shr ecx, 4
		imul ecx
		add edx, ecx

		mov timeTakenSeconds, ecx
	}

	std::stringstream ss;
	ss << std::setfill('0') << std::setw(2) << (timeTakenSeconds / 3600) << ":" 
		<< std::setfill('0') << std::setw(2) << (timeTakenSeconds / 60) << ":" 
		<< std::setfill('0') << std::setw(2) << (timeTakenSeconds % 60);

	return std::string(ss.str());
}

std::string GetCurrentLevelName()
{
	const auto namesOffsetAddr = ReadFromProcessMemory<uint32_t>(TR2ConstAddress::LevelNamesOffset);
	if(!namesOffsetAddr.has_value())
		return {};

	const auto currentLevelIdx = ReadFromProcessMemory<uint32_t>(TR2ConstAddress::LevelCurrentOffset);

	if(!currentLevelIdx.has_value())
		return {};

	const uint32_t addrOfLevelNamePtr = namesOffsetAddr.value() + sizeof(uint32_t) * currentLevelIdx.value();
	uint32_t addrOfLevelNameStr;

	if(!WinAPI::ReadTRProcessMemory(trProc, addrOfLevelNamePtr, reinterpret_cast<uint8_t*>(&addrOfLevelNameStr), sizeof(uint32_t)))
		return {};

	const size_t NAME_BUFFER_SIZE = 32;
	uint8_t currentLevelNameBuffer[NAME_BUFFER_SIZE];
	if(!WinAPI::ReadTRProcessMemory(trProc, addrOfLevelNameStr, currentLevelNameBuffer, sizeof(uint8_t) * NAME_BUFFER_SIZE))
		return {};

	const char* rawCharOfString = reinterpret_cast<char*>(currentLevelNameBuffer);
	const size_t levelNameLen = std::strlen(rawCharOfString);
	return std::string(rawCharOfString, levelNameLen);
}

void PrintSecrets()
{
	const auto secretsValue = ReadFromProcessMemory<uint16_t>(TR2ConstAddress::Secrets);
	if(!secretsValue.has_value())
		return;

	const bool secretA = (secretsValue.value() & 0x01);
	const bool secretB = (secretsValue.value() & 0x02);
	const bool secretC = (secretsValue.value() & 0x04);

	std::cout << "Secrets Found:";

	if(secretA) std::cout << " [GOLD]";
	if(secretB) std::cout << " [JADE]";
	if(secretC) std::cout << " [STONE]";

	if(!secretA && !secretB && !secretC)
		std::cout << " None\n";
	else
		std::cout << std::endl;
}

void PrintDebug()
{
	const auto baseAddr = ReadFromProcessMemory<uint32_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::PlayerBase__);
	if(!baseAddr.has_value())
		return;

	std::cout << std::hex << "Player info base addr: 0x" << baseAddr.value() << std::endl;

	TRConstants<TRGame::TombRaider_2>::PlayerInfo playerInfo;
	if(!WinAPI::ReadTRProcessMemory(trProc, baseAddr.value(), reinterpret_cast<uint8_t*>(&playerInfo), sizeof(playerInfo)))
		return;

	//clamp health to 0-1000 (0x03E8)
	playerInfo.health = std::max(std::min(playerInfo.health, 1000i16), 0i16);

	uint32_t healthPercentInt;

	__asm
	{
		mov eax, 0x66666667
		movzx ecx, playerInfo.health
		imul ecx
		sar edx, 2
		mov ecx, edx
		shr ecx, 0x1F
		add edx, ecx
		mov healthPercentInt, edx
	}

	std::cout << "Health: " << std::dec << healthPercentInt << "% ( " << playerInfo.health << " )\n";

	std::cout << std::fixed << std::setprecision(2);

	const float hrotation = ((playerInfo.hrotation > 0) ? (playerInfo.hrotation / 65535.0f * 360.0f) : 0.0f);
	const float vrotation = ((playerInfo.vrotation > 0) ? (playerInfo.vrotation / 65535.0f * 360.0f) : 0.0f);
	const float positionX = (playerInfo.positionX / 1024.0f);
	const float positionY = (playerInfo.positionY / 1024.0f);
	const float positionZ = (playerInfo.positionZ / 1024.0f);

	std::cout << "HRotation: " << hrotation << std::endl;
	std::cout << "VRotation: " << vrotation << std::endl;
	std::cout << "Position X: " << positionX << std::endl;
	std::cout << "Position Y: " << positionY << std::endl;
	std::cout << "Position Z: " << positionZ << std::endl;
	std::cout << "HAcceleration: " << playerInfo.hacceleration << std::endl;

	const bool reverseAcceleration{ playerInfo.vaccelerationDir == 0xFF };
	const int16_t vacceleration_s = static_cast<int16_t>(reverseAcceleration ? -playerInfo.vacceleration: playerInfo.vacceleration);

	std::cout << "VAcceleration: " << vacceleration_s << std::endl;
	std::cout << "Current anim: " << std::hex << (int)playerInfo.animState << std::endl;

	//oxygen
	const auto oxygenValue = ReadFromProcessMemory<int16_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::Oxygen);
	if(!oxygenValue.has_value())
		return;

	//clamp oxygen value to 0-1800 (0x708)
	const int16_t oxygenClamped = std::max(std::min(oxygenValue.value(), 1800i16), 0i16);
	
	uint32_t oxygenPercent;
	__asm
	{
		movsx eax, oxygenClamped
		lea eax, dword ptr [eax + eax * 4]
		lea ecx, dword ptr [eax + eax * 4]
		mov eax, 0x91A2B3C5
		shl ecx, 2
		imul ecx
		add edx, ecx
		sar edx, 0xA
		mov ecx, edx
		shr ecx, 0x1F
		add edx, ecx
		mov oxygenPercent, edx
	}

	//float oxygenFloat = (value / 18.0f);
	std::cout << "Oxygen: " << std::dec << oxygenPercent << "% ( " << oxygenClamped << " )\n";
}

void PrintInfoScreen()
{
	WinAPI::ClearConsole();
	std::cout << "Tomb Raider " << static_cast<uint32_t>(trProc.game) << " is running (PID: " << trProc.pid << ")\n";

	std::cout << std::dec;
	std::cout << std::endl;

	std::cout << "===== Statistics =====\n";

	//Level name
	const std::string levelName = GetCurrentLevelName();
	std::cout << levelName << std::endl;

	//Time taken
	const std::string timeTakenStr = GetTimeTakenFormatted();
	if(timeTakenStr.length() > 0)
		std::cout << "Time Taken: " << timeTakenStr.c_str() << std::endl;

	PrintSecrets();

	//Kills
	const auto kills = ReadFromProcessMemory<uint32_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::KillCount);
	if(kills.has_value())
		std::cout << "Kills: " << (kills.value() & 0xFFFF) << std::endl;

	//Ammo used
	const auto ammo = ReadFromProcessMemory<uint32_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::AmmoUsed);
	std::cout << "Ammo Used: " << ammo.value() << std::endl;

	//Hits
	const auto hits = ReadFromProcessMemory<uint32_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::HitCount);
	std::cout << "Hits: " << hits.value() << std::endl;

	//Health packs used
	const auto hpUsed = ReadFromProcessMemory<uint32_t>(TRConstants<TRGame::TombRaider_2>::MemAddr::HealthPacksUsed);
	if(hpUsed.has_value())
		std::cout << std::setw(1) << std::setprecision(1) << "Health packs used: " << ((hpUsed.value() & 0xFF) * 0.5f) << std::endl;

	//Distance travelled
	const uint32_t distanceTravelled = GetDistanceTravelled();
	std::cout << "Distance Travelled: ";

	if(distanceTravelled < 1000)
		std::cout << distanceTravelled << "m\n";
	else
		std::cout << std::fixed << std::setprecision(3) << (distanceTravelled * 0.001) << "km\n";

	std::cout << std::endl;
	PrintDebug();
}

void TRPCMain(int32_t updateIntervalMillis)
{
	trProc = WinAPI::GetRunningGameProc();

	if(trProc.game == TRGame::TombRaider_2 && WinAPI::OpenTRProcess(trProc))
	{
		while(1)
		{
			if(!WinAPI::IsTRStillRunning(trProc))
			{
				std::cout << "Game is not running anymore!\n";
				break;
			}

			PrintInfoScreen();

			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(updateIntervalMillis));
		}
	
		WinAPI::CloseTRProcess(trProc);
	}
	else
	{
		std::cout << "No Tomb Raider game is running!\n";
	}
}