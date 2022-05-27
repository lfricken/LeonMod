#pragma once

#include <cstddef>
#include "CvEnums.h"
#include "ICvDLLUserInterface.h"


// wraps DLLUI for certain calls to make debugging easier
class DllWrapper
{
public:
	static DllWrapper* get()
	{
		static DllWrapper* s_instance = new DllWrapper();
		return s_instance;
	}
	// Only the 'active' (local human) player
	void PublishActivePlayerTurnStart(PlayerTypes ePlayer);
	// Only the 'active' (local human) player
	void PublishActivePlayerTurnEnd(PlayerTypes ePlayer);
	// multiplayer publish turn state
	void PublishPlayerTurnStatus(PlayerTypes ePlayer, CvDLLInterfaceIFaceBase::TURN_STATUS_TYPE eStatus, const char* pszTag = NULL);
	// Only remote human players
	void PublishRemotePlayerTurnStart(PlayerTypes ePlayer);
	// Only remote human players
	void PublishRemotePlayerTurnEnd(PlayerTypes ePlayer);


	void PublishEndTurnDirty(PlayerTypes ePlayer);
};

// Wraps the DLLUI
#define GuiDllWrap (DllWrapper::get())

