
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
int TundraTiles(const CvPlot& p)
{
	if (p.HasTerrain(TERRAIN_TUNDRA))
		return 1;
	else
		return 0;
}
int DesertTiles(const CvPlot& p)
{
	if (p.HasTerrain(TERRAIN_DESERT))
		return 1;
	else
		return 0;
}
int StonePlots(const CvPlot& p)
{
	if (p.HasResource("RESOURCE_STONE"))
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
	case CARD_ANCIENT_RESOURCES_INUITS: return "POLICY_CARD_ANCIENT_RESOURCES_INUITS_PASSIVE";
	case CARD_ANCIENT_RESOURCES_BEDOUINS: return "POLICY_CARD_ANCIENT_RESOURCES_BEDOUINS_PASSIVE";
	case CARD_ANCIENT_RESOURCES_ATLATL: return "POLICY_CARD_ANCIENT_RESOURCES_ATLATL_PASSIVE";
	case CARD_ANCIENT_RESOURCES_FLINT_KNAPPING: return "POLICY_CARD_ANCIENT_RESOURCES_FLINT_KNAPPING_PASSIVE";
	case CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS: return "POLICY_CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS_PASSIVE";
	case CARD_ANCIENT_RESOURCES_SPEAR_FISHING: return "POLICY_CARD_ANCIENT_RESOURCES_SPEAR_FISHING_PASSIVE";
	case CARD_ANCIENT_RESOURCES_DIVINE_CREATION: return "POLICY_CARD_ANCIENT_RESOURCES_DIVINE_CREATION_PASSIVE";
	case CARD_ANCIENT_BUILDINGS_SHIPS_OF_THE_DESERT: return "POLICY_CARD_ANCIENT_BUILDINGS_SHIPS_OF_THE_DESERT_PASSIVE";
	case CARD_ANCIENT_BUILDINGS_FORCED_LEVY: return "POLICY_CARD_ANCIENT_BUILDINGS_FORCED_LEVY_PASSIVE";
	case CARD_ANCIENT_POLITICAL_ORTHODOXY: return "POLICY_CARD_ANCIENT_POLITICAL_ORTHODOXY_PASSIVE";
	case CARD_ANCIENT_POLITICAL_PROGRESSIVE: return "POLICY_CARD_ANCIENT_POLITICAL_PROGRESSIVE_PASSIVE";
	case CARD_ANCIENT_POLITICAL_AGGRESIVE: return "POLICY_CARD_ANCIENT_POLITICAL_AGGRESIVE_PASSIVE";
	case CARD_ANCIENT_POLITICAL_EXCLUSIVE: return "POLICY_CARD_ANCIENT_POLITICAL_EXCLUSIVE_PASSIVE";
	case CARD_ANCIENT_POLITICAL_PROTECTIVE: return "POLICY_CARD_ANCIENT_POLITICAL_PROTECTIVE_PASSIVE";
	case CARD_ANCIENT_BUILDINGS_DRUIDS: return "POLICY_CARD_ANCIENT_BUILDINGS_DRUIDS_PASSIVE";
	case CARD_ANCIENT_BUILDINGS_HARBORMASTER: return "POLICY_CARD_ANCIENT_BUILDINGS_HARBORMASTER_PASSIVE";
	case CARD_CLASSICAL_RESOURCE_LIMESTONE: return "POLICY_CARD_CLASSICAL_RESOURCE_LIMESTONE_PASSIVE";
	case CARD_CLASSICAL_UNITS_INDUSTRIOUS: return "POLICY_CARD_CLASSICAL_UNITS_INDUSTRIOUS_PASSIVE";
	case CARD_CLASSICAL_UNITS_FORCED_MARCH: return "POLICY_CARD_CLASSICAL_UNITS_FORCED_MARCH_PASSIVE";
	case CARD_CLASSICAL_UNITS_SIEGE_ENGINEERS: return "POLICY_CARD_CLASSICAL_UNITS_SIEGE_ENGINEERS_PASSIVE";
	case CARD_CLASSICAL_UNITS_SUPPLIES: return "POLICY_CARD_CLASSICAL_UNITS_SUPPLIES_PASSIVE";
	case CARD_CLASSICAL_UNITS_FLETCHING: return "POLICY_CARD_CLASSICAL_UNITS_FLETCHING_PASSIVE";
	case CARD_CLASSICAL_WONDERS_PHEIDIAS: return "POLICY_CARD_CLASSICAL_WONDERS_PHEIDIAS_PASSIVE";
	case CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE: return "POLICY_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE_PASSIVE";
	case CARD_CLASSICAL_BUILDINGS_GLADITORIAL_GAMES: return "POLICY_CARD_CLASSICAL_BUILDINGS_GLADITORIAL_GAMES_PASSIVE";
	case CARD_CLASSICAL_BUILDINGS_CORINTHIAN_ORDER: return "POLICY_CARD_CLASSICAL_BUILDINGS_CORINTHIAN_ORDER_PASSIVE";

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
	
	case CARD_ANCIENT_RESOURCES_ATLATL:
	{
		int numDeerPlotsOwned = player.CountOwnedPlots(DeerPlots);
		if (numDeerPlotsOwned < 6)		
		{
			return false;
		}
	}
	case CARD_ANCIENT_UNITS_FAVORABLE_WINDS:
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES:
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_UNITS_VALOUR:
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_RESOURCES_INUITS:
	{
		int numTundraPlotsOwned = player.CountOwnedPlots(TundraTiles);		
		if (numTundraPlotsOwned < 10)
		{
			return false;
		}
	}
	case CARD_ANCIENT_RESOURCES_BEDOUINS:
	{
		int numDesertPlotsOwned = player.CountOwnedPlots(DesertTiles);
		if (numDesertPlotsOwned < 10)
		{
			return false;
		}
	}
	case CARD_ANCIENT_RESOURCES_SPEAR_FISHING:
	{
		if (playerEra > classicalEra)
		{
			return false;
		}
	}
	case CARD_ANCIENT_POLITICAL_ORTHODOXY:
	{
		int numTraditionPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numTraditionPolicies < 3)		
		{
			return false;
		}
	}
	case CARD_ANCIENT_POLITICAL_PROGRESSIVE:
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numLibertyPolicies < 3)
		{
			return false;
		}
	}
	case CARD_ANCIENT_POLITICAL_AGGRESIVE:
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numHonorPolicies < 3)
		{
			return false;
		}
	}
	case CARD_ANCIENT_POLITICAL_EXCLUSIVE:
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numLibertyPolicies == 0) && (numPolicies >= 5));
		if (!satisfied)
		{
			return false;
		}
	}
	case CARD_ANCIENT_POLITICAL_PROTECTIVE:
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numHonorPolicies == 0) && (numPolicies >= 5));
		if (!satisfied)
		{
			return false;
		}
	}
	case CARD_CLASSICAL_RESOURCE_LIMESTONE:
	{
		int numStonePlotsOwned = player.CountOwnedPlots(StonePlots);
		if (numStonePlotsOwned < 3)
		{
			return false;
		}
	}
	case CARD_CLASSICAL_UNITS_INDUSTRIOUS:
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case CARD_CLASSICAL_UNITS_FORCED_MARCH:
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case CARD_CLASSICAL_UNITS_SIEGE_ENGINEERS:
	{
		if (playerEra >= industrialEra)
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
