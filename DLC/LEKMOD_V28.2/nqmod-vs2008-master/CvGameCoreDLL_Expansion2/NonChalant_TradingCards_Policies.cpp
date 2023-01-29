
#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"


void PlaceRandomSpy(CvPlayer* pPlayer)
{

}
int DeerPlots(const CvPlot& p)
{
	if (p.HasResource("RESOURCE_DEER"))
		return 1;
	else
		return 0;
}
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
bool TradingCard::IsConditionSatisfied(TradingCardTypes type, const CvPlayer* pPlayer, bool isActive)
{
	const CvPlayer& player = *pPlayer;
	switch (type)
	{
	case CARD_NAVAL_MOVES:
	{
		int numDeerPlotsOwned = player.CountOwnedPlots(DeerPlots);
		const bool satisfied = true;
		if (!satisfied)
		{
			return false;
		}
	}
	case CARD_FISH_GOLD:
	{

	}
	default: return true;
	};
	return true;
}
void TradingCard::ApplyPassiveEffects(TradingCardTypes type, CvPlayer* player,
	// (-1 remove the benefit, +1 apply a copy of the benefit)
	int delta)
{
	if (delta == 0)
	{
		return;
	}

	// apply additional non policy effects

}
bool TradingCard::ApplyActiveEffects(TradingCardTypes type, CvPlayer* player)
{
	// apply active non policy effects
	switch (type)
	{
	case CARD_RANDOM_SPY: PlaceRandomSpy(player); return true;
	default: return true;
	};

	return true;
}
