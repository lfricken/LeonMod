
#include "CvDllWrap.h"
#include "CvGlobals.h"
#include "CvGameCoreDLLPCH.h"
#include "CvGameCoreUtils.h"

// Only the 'active' (local human) player
void DllWrapper::PublishActivePlayerTurnStart(PlayerTypes ePlayer)
{
	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::ActivePlayerTurnStart";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishActivePlayerTurnStart();
}
// Only the 'active' (local human) player
void DllWrapper::PublishActivePlayerTurnEnd(PlayerTypes ePlayer)
{
	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::ActivePlayerTurnEnd";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishActivePlayerTurnEnd();
}
void DllWrapper::PublishPlayerTurnStatus(PlayerTypes ePlayer, CvDLLInterfaceIFaceBase::TURN_STATUS_TYPE eStatus, const char* pszTag)
{
	string status = "ERROR";
	if (eStatus == CvDLLInterfaceIFaceBase::TURN_STATUS_TYPE::TURN_START)
		status = "START";
	else if (eStatus == CvDLLInterfaceIFaceBase::TURN_STATUS_TYPE::TURN_END)
		status = "END";
	else if (eStatus == CvDLLInterfaceIFaceBase::TURN_STATUS_TYPE::TURN_STEP)
		status = "STEP";

	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::PlayerTurnStatus(" << status << ")";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishPlayerTurnStatus(eStatus, ePlayer, pszTag);
}
void DllWrapper::PublishRemotePlayerTurnStart(PlayerTypes ePlayer)
{
	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::RemotePlayerTurnStart";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishRemotePlayerTurnStart();
}
void DllWrapper::PublishRemotePlayerTurnEnd(PlayerTypes ePlayer)
{
	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::RemotePlayerTurnEnd";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishRemotePlayerTurnEnd();
}
void DllWrapper::PublishEndTurnDirty(PlayerTypes ePlayer)
{
	stringstream ss;
	ss << "PUBLISH Player(" << ePlayer << ")::EndTurnDirty";
	netMessageDebug(NET_MESSAGE_PLAYER_EVENTS, ss.str());
	DLLUI->PublishEndTurnDirty();
}

