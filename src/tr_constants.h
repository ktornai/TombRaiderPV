#pragma once

//#include "info.h"

template<TRGame>
struct TRConstants { };

template<>
struct TRConstants<TRGame::TombRaider_2>
{
	enum class MemAddr: uint32_t
	{
		PlayerBase__		= 0x005207BC,
		Oxygen				= 0x005206F6,
		LevelNamesOffset	= 0x00521EC4,
		LevelCurrentOffset	= 0x004D9EB0,
		Time_Taken			= 0x0051EE00,
		Secrets				= 0x0051EE12,
		KillCount			= 0x0051EE10,
		AmmoUsed			= 0x0051EE04,
		HitCount			= 0x0051EE08,
		HealthPacksUsed		= 0x0051EE13,
		Distance_Travelled	= 0x0051EE0C,
	};

	enum AnimState
	{
	/*
		00 - step forward
		01 - running forward
		02 - standing
		05 - hop back
		06 - turn right
		07 - turn left
		08 - dead (falling/other)
		09 - falling
		0A - hanging on the edge
		0D - water no action
		0F - gaining momentum to jump
		10 - step back
		11 - swimming underwater
		12 - stopping swin underwater
		13 - climbing up
		14 - turning
		15 - step right
		16 - step left
		18 - sliding forward
		19 - jump backward
		1A - side jump right
		1B - side jump left
		20 - sliding backward
		21 - floating in water
		22 - swim forward
		2C - dead (drowned)
		2F - swim backward
		41 - walking in water
	*/
	};

	struct PlayerInfo
	{
		uint32_t	__unused_0;
		uint32_t	__unused_4;
		uint32_t	__unused_8;
		uint16_t	__unused_C;
		uint8_t		animState; //0xE
		uint8_t		__unused_F;
		uint32_t	__unused_10;
		uint32_t	__unused_14;
		uint32_t	__unused_18;
		uint16_t	__unused_1C;
		uint16_t	hacceleration; //0x1E
		uint8_t		vacceleration; //0x20
		uint8_t		vaccelerationDir; //0x21
		int16_t		health;		//0x22
		uint32_t	__unused_24;
		uint32_t	__unused_28;
		uint32_t	__unused_2C;
		uint32_t	__unused_30;
		int32_t		positionX;	//0x34
		int32_t		positionY;	//0x38
		int32_t		positionZ;	//0x3C
		uint16_t	vrotation;	//0x40
		uint16_t	hrotation;	//0x42

	};
};
