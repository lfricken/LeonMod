
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

	case 142: // POLICY_CARD_ANCIENT_RESOURCES_ATLATL_PASSIVE
	{
		int numDeerPlotsOwned = player.CountOwnedPlots(DeerPlots);
		return (numDeerPlotsOwned < 6);
	}
	case 128: // POLICY_CARD_ANCIENT_UNITS_FAVORABLE_WINDS_PASSIVE
	{
		return (playerEra >= medievalEra);
	}
	case 129: // POLICY_CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES_PASSIVE
	{
		return (playerEra >= medievalEra);
	}
	case 139: // POLICY_CARD_ANCIENT_UNITS_VALOUR_PASSIVE
	{
		return (playerEra >= medievalEra);
	}
	case 140: // POLICY_CARD_ANCIENT_RESOURCES_INUITS_PASSIVE
	{
		int numTundraPlotsOwned = player.CountOwnedPlots(TundraTiles);
		return (numTundraPlotsOwned < 10);
	}
	case 141: // POLICY_CARD_ANCIENT_RESOURCES_BEDOUINS_PASSIVE
	{
		int numDesertPlotsOwned = player.CountOwnedPlots(DesertTiles);
		return (numDesertPlotsOwned < 10);
	}
	case 145: // POLICY_CARD_ANCIENT_RESOURCES_SPEAR_FISHING_PASSIVE
	{
		return (playerEra > classicalEra);
	}
	case 149: // POLICY_CARD_ANCIENT_POLITICAL_ORTHODOXY_PASSIVE
	{
		int numTraditionPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		return (numTraditionPolicies < 3);
	}
	case 150: // POLICY_CARD_ANCIENT_POLITICAL_PROGRESSIVE_PASSIVE
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		return (numLibertyPolicies < 3);
	}
	case 151: // POLICY_CARD_ANCIENT_POLITICAL_AGGRESIVE_PASSIVE
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		return (numHonorPolicies < 3);
	}
	case 152: // POLICY_CARD_ANCIENT_POLITICAL_EXCLUSIVE_PASSIVE
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numLibertyPolicies == 0) && (numPolicies >= 5));
		return (!satisfied);
	}
	case 155: // POLICY_CARD_ANCIENT_POLITICAL_PROTECTIVE_PASSIVE
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numHonorPolicies == 0) && (numPolicies >= 5));
		return (!satisfied);
	}
	case 156: // POLICY_CARD_CLASSICAL_RESOURCE_LIMESTONE_PASSIVE
	{
		int numStonePlotsOwned = player.CountOwnedPlots(StonePlots);
		return (numStonePlotsOwned < 3);
	}
	case 157: // POLICY_CARD_CLASSICAL_UNITS_INDUSTRIOUS_PASSIVE
	{
		return (playerEra >= industrialEra);
	}
	case 158: // POLICY_CARD_CLASSICAL_UNITS_FORCED_MARCH_PASSIVE
	{
		return (playerEra >= industrialEra);
	}
	case 159: // POLICY_CARD_CLASSICAL_UNITS_SIEGE_ENGINEERS_PASSIVE
	{
		return (playerEra >= industrialEra);
	}
	case 168: // POLICY_CARD_MEDIEVAL_RESOURCE_GILLNETS_PASSIVE
	{
		return (playerEra >= industrialEra);
	}
	case 175: // POLICY_CARD_MEDIEVAL_UNITS_LANTEEN_SAILS_PASSIVE
	{
		return (playerEra >= industrialEra);
	}
	case 184: // POLICY_CARD_ANCIENT_WONDERS_HIPPODAMUS_OF_MILETUS_PASSIVE
	{
		return (playerEra >= medievalEra);
	}
	case 186: // POLICY_CARD_CLASSICAL_BUILDINGS_CANNON_OF_TEN_PASSIVE
	{
		int numAmphitheaters = player.countNumBuildingClasses(BuildingClassTypes(19));
		return (numAmphitheaters < 10);
	}
	case 188: // POLICY_CARD_MEDIEVAL_BUILDINGS_FEALTY_ACTIVE
	{
		int numCastles = player.countNumBuildingClasses(BuildingClassTypes(19));
		return (numCastles < 6);
	}


	default: return true;
	}
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
	case 1: PlaceRandomSpy(player); return true;
	default: return true;
	};

	return true;
}
