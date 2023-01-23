
#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"


string TradingCard::GetActivePolicy(TradingCardTypes type)
{
	switch (type)
	{
	case CARD_NAVAL_MOVES: return "CARD_NAVAL_MOVES_ACTIVE";
	case CARD_RANDOM_SPY: return "CARD_RANDOM_SPY";
	default: return "";
	};
	return "";
}
string TradingCard::GetPassivePolicy(TradingCardTypes type)
{
	switch (type)
	{
	case CARD_NAVAL_MOVES: return "CARD_NAVAL_MOVES_PASSIVE";
	case CARD_FISH_GOLD: return "CARD_FISH_GOLD";
	default: return "";
	};
	return "";
}
int TradingCard::GetEstimatedValue(TradingCardTypes type)
{
	const bool should_AI_Accept_Any_Deal = true; // switch to true so AI will not value cards and trade no matter what
	if (should_AI_Accept_Any_Deal)
	{
		return 1;
	}
}
void TradingCard::TryApplyPassiveEffects(TradingCardTypes type, CvPlayer* player, int delta)
{
	// apply additional non policy effects

}
void PlaceRandomSpy(CvPlayer* pPlayer)
{

}
bool TradingCard::TryActivate(TradingCardTypes type, CvPlayer* player)
{
	switch (type)
	{
	case CARD_RANDOM_SPY: PlaceRandomSpy(player); return true;
	default: return true;
	};

	return true;
}
