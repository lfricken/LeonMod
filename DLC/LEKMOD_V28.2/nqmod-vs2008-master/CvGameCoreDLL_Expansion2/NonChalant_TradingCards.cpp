


#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"


void PlaceRandomSpy(CvPlayer* pPlayer)
{

}


FDataStream& operator <<(FDataStream& kStream, const TradingCard& data)
{
	kStream << (int)data.GetType();
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, TradingCard& data)
{
	int type;
	kStream >> type;
	data = TradingCard((TradingCardTypes)type);
	return kStream;
}
TradingCard::TradingCard(const TradingCardTypes type)
{
	m_type = type;
}
TradingCardTypes TradingCard::GetType() const
{
	return m_type;
}
bool TradingCard::IsPassive() const
{
	switch(m_type)
	{
	case CARD_NAVAL_MOVES: return true;
	case CARD_FISH_GOLD: return true;
	case CARD_RANDOM_SPY: return false;
	default: return true;
	};
	return true;
}
bool TradingCard::TryActivate(CvPlayer* pActivatingPlayer) const
{
	if (IsPassive())
	{
		return false;
	}

	pActivatingPlayer->GetTreasury()->ChangeGold(100);

	switch (m_type)
	{
	case CARD_RANDOM_SPY: PlaceRandomSpy(pActivatingPlayer); return true;
	default: return true;
	};

	return true;
}
