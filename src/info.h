#pragma once

enum class TRGame
{
	Unknown = 0,
	TombRaider_1,
	TombRaider_2,
	TombRaider_3,
};

struct TRProcess
{
	TRGame game{ TRGame::Unknown };
	DWORD pid{ 0 };
	HANDLE procHandle{ INVALID_HANDLE_VALUE };
};