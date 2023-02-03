
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
	const CvPolicyEntry* pInfo = GC.getPolicyInfo((PolicyTypes)type);
	if (pInfo != NULL)
	{
		if (pInfo->CardIsActive())
		{
			return pInfo->GetType();
		}
		else
		{
			return "";
		}
	}
	return "";
}
string TradingCard::GetPassivePolicy(TradingCardTypes type)
{
	const CvPolicyEntry* pInfo = GC.getPolicyInfo((PolicyTypes)type);
	if (pInfo != NULL)
	{
		if (pInfo->CardIsActive())
		{
			return "";
		}
		else
		{
			return pInfo->GetType();
		}
	}
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

	case 142: // POLICY_CARD_ANCIENT_RESOURCES_ATLATL_PASSIVE
	{
		int numDeerPlotsOwned = player.CountOwnedPlots(DeerPlots);
		if (numDeerPlotsOwned < 6)
		{
			return false;
		}
	}
	case 128: // POLICY_CARD_ANCIENT_UNITS_FAVORABLE_WINDS_PASSIVE
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case 129: // POLICY_CARD_ANCIENT_UNITS_SACRIFICIAL_CAPTIVES_PASSIVE
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case 139: // POLICY_CARD_ANCIENT_UNITS_VALOUR_PASSIVE
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case 140: // POLICY_CARD_ANCIENT_RESOURCES_INUITS_PASSIVE
	{
		int numTundraPlotsOwned = player.CountOwnedPlots(TundraTiles);
		if (numTundraPlotsOwned < 10)
		{
			return false;
		}
	}
	case 141: // POLICY_CARD_ANCIENT_RESOURCES_BEDOUINS_PASSIVE
	{
		int numDesertPlotsOwned = player.CountOwnedPlots(DesertTiles);
		if (numDesertPlotsOwned < 10)
		{
			return false;
		}
	}
	case 145: // POLICY_CARD_ANCIENT_RESOURCES_SPEAR_FISHING_PASSIVE
	{
		if (playerEra > classicalEra)
		{
			return false;
		}
	}
	case 149: // POLICY_CARD_ANCIENT_POLITICAL_ORTHODOXY_PASSIVE
	{
		int numTraditionPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numTraditionPolicies < 3)
		{
			return false;
		}
	}
	case 150: // POLICY_CARD_ANCIENT_POLITICAL_PROGRESSIVE_PASSIVE
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numLibertyPolicies < 3)
		{
			return false;
		}
	}
	case 151: // POLICY_CARD_ANCIENT_POLITICAL_AGGRESIVE_PASSIVE
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numHonorPolicies < 3)
		{
			return false;
		}
	}
	case 152: // POLICY_CARD_ANCIENT_POLITICAL_EXCLUSIVE_PASSIVE
	{
		int numLibertyPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numLibertyPolicies == 0) && (numPolicies >= 5));
		if (!satisfied)
		{
			return false;
		}
	}
	case 155: // POLICY_CARD_ANCIENT_POLITICAL_PROTECTIVE_PASSIVE
	{
		int numHonorPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		int numPolicies = player.GetNumPolicies();
		const bool satisfied = ((numHonorPolicies == 0) && (numPolicies >= 5));
		if (!satisfied)
		{
			return false;
		}
	}
	case 156: // POLICY_CARD_CLASSICAL_RESOURCE_LIMESTONE_PASSIVE
	{
		int numStonePlotsOwned = player.CountOwnedPlots(StonePlots);
		if (numStonePlotsOwned < 3)
		{
			return false;
		}
	}
	case 157: // POLICY_CARD_CLASSICAL_UNITS_INDUSTRIOUS_PASSIVE
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case 158: // POLICY_CARD_CLASSICAL_UNITS_FORCED_MARCH_PASSIVE
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case 159: // POLICY_CARD_CLASSICAL_UNITS_SIEGE_ENGINEERS_PASSIVE
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case 168: // POLICY_CARD_MEDIEVAL_RESOURCE_GILLNETS_PASSIVE
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case 175: // POLICY_CARD_MEDIEVAL_UNITS_LANTEEN_SAILS_PASSIVE
	{
		if (playerEra >= industrialEra)
		{
			return false;
		}
	}
	case 184: // POLICY_CARD_ANCIENT_WONDERS_HIPPODAMUS_OF_MILETUS_PASSIVE
	{
		if (playerEra >= medievalEra)
		{
			return false;
		}
	}
	case 186: // POLICY_CARD_CLASSICAL_BUILDINGS_CANNON_OF_TEN_PASSIVE
	{

		int numAmphitheaters = player.countNumBuildingClasses(BuildingClassTypes(19));
		if (numAmphitheaters < 10)
		{
			return false;
		}
	}
	case 188: // POLICY_CARD_MEDIEVAL_BUILDINGS_FEALTY_ACTIVE
	{
		int numCastles = player.countNumBuildingClasses(BuildingClassTypes(19));
		if (numCastles < 6)
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
	case 1: PlaceRandomSpy(player); return true;
	default: return true;
	};

	return true;
}
