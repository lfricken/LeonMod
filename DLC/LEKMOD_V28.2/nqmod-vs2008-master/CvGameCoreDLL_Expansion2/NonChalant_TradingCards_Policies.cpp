
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
	case CARD_ANCIENT_UNITS_MILITIA: return "POLICY_CARD_ANCIENT_UNITS_MILITIA_ACTIVE";
	case CARD_ANCIENT_UNITS_SLAVES: return "POLICY_CARD_ANCIENT_UNITS_SLAVES_ACTIVE";
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
	case CARD_ANCIENT_UNITS_FAVORABLE_WINDS: return "POLICY_CARD_ANCIENT_UNITS_FAVORABLE_WINDS_PASSIVE";
	case CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES: return "POLICY_CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES_PASSIVE";
	case CARD_ANCIENT_UNITS_OBSIDIAN_ARROWS: return "POLICY_CARD_ANCIENT_UNITS_OBSIDIAN_ARROWS_PASSIVE";
	case CARD_ANCIENT_UNITS_WAY_OF_THE_WARRIOR: return "POLICY_CARD_ANCIENT_UNITS_WAY_OF_THE_WARRIOR_PASSIVE";
	case CARD_ANCIENT_UNITS_SCOUTING: return "POLICY_CARD_ANCIENT_UNITS_SCOUTING_PASSIVE";
	case CARD_ANCIENT_UNITS_GREEK_FIRE: return "POLICY_CARD_ANCIENT_UNITS_GREEK_FIRE_PASSIVE";
	case CARD_ANCIENT_UNITS_BRONZE_CLAD_HULL: return "POLICY_CARD_ANCIENT_UNITS_BRONZE_CLAD_HULL_PASSIVE";
	case CARD_ANCIENT_UNITS_SPOKED_WHEELS: return "POLICY_CARD_ANCIENT_UNITS_SPOKED_WHEELS_PASSIVE";
	case CARD_ANCIENT_UNITS_SHIELD_WALL: return "POLICY_CARD_ANCIENT_UNITS_SHIELD_WALL_PASSIVE";
	case CARD_ANCIENT_UNITS_VALOUR: return "POLICY_CARD_ANCIENT_UNITS_VALOUR_PASSIVE";	
	
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
	
	int playerEra = player.GetCurrentEra();
	int ancientEra = 1;
	int classicalEra = 2;
	int medievalEra = 3;
	int renaissanceEra = 4;
	int industrialEra = 5;
	int modernEra = 6;
	int atomicEra = 7;
	int informationEra = 8;
	int futureEra = 9;
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
	case CARD_ANCIENT_UNITS_FAVORABLE_WINDS:
	{
		if (playerEra > classicalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES:
	{
		if (playerEra > classicalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_UNITS_VALOUR:
	{
		if (playerEra > classicalEra)
		{
			return false;
		}
	}
	default: return true;
	}
	;
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
