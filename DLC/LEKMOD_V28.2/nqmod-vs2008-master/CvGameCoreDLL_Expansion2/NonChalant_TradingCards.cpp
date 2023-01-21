


#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"


void PlaceRandomSpy(CvPlayer* pPlayer)
{

}


FDataStream& operator <<(FDataStream& kStream, const TradingCardTypes& data)
{
	kStream << (int)data;
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, TradingCardTypes& data)
{
	int type;
	kStream >> type;
	data = (TradingCardTypes)type;
	return kStream;
}
string TradingCard::GetName(TradingCardTypes type, CvPlayer* pOwner)
{
	return "GetName";
}
string TradingCard::GetDesc(TradingCardTypes type, CvPlayer* pOwner)
{
	return "GetDesc";
}
bool TradingCard::IsPassive(TradingCardTypes type)
{
	switch(type)
	{
	case CARD_NAVAL_MOVES: return true;
	case CARD_FISH_GOLD: return true;
	case CARD_RANDOM_SPY: return false;
	default: return true;
	};
	return true;
}
bool TradingCard::TryActivate(TradingCardTypes type, CvPlayer* pActivatingPlayer)
{
	if (IsPassive(type))
	{
		return false;
	}

	pActivatingPlayer->GetTreasury()->ChangeGold(100);

	switch (type)
	{
	case CARD_RANDOM_SPY: PlaceRandomSpy(pActivatingPlayer); return true;
	default: return true;
	};

	return true;
}
