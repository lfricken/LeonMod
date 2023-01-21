
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

