#include "CvGameCoreDLLPCH.h"
#include "CvPlot.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvArea.h"
#include "ICvDLLUserInterface.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvTeam.h"
#include "CvGameCoreUtils.h"
#include "CvRandom.h"
#include "CvInfos.h"
#include "CvImprovementClasses.h"
#include "CvAStar.h"
#include "CvEconomicAI.h"
#include "CvEnumSerialization.h"
#include "CvNotifications.h"
#include "CvMinorCivAI.h"
#include "CvUnitCombat.h"
#include "CvDLLUtilDefines.h"
#include "CvInfosSerializationHelper.h"
#include "CvBarbarians.h"

#include "CvDllPlot.h"
#include "CvGameCoreEnums.h"
#include "CvDllUnit.h"
#include "CvUnitMovement.h"
#include "CvTargeting.h"
#include "CvTypes.h"

// Include this after all other headers.
#include "LintFree.h"


int CvGlobals::getTOURISM_MODIFIER_HAMMERCOMPETITION(const PlayerTypes ePlayer) const
{
	int base = 10;
	// always have some base, possibly modify based on player
	return base;
}
int CvGlobals::getCITIZENS_MIN_FOR_SPECIALIST(const PlayerTypes ePlayer) const
{
	return 8;
}
int CvGlobals::getCITIZENS_PER_SPECIALIST(const PlayerTypes ePlayer) const
{
	return 2;
}
int CvGlobals::getYIELD_PER_TURN_ALLY(const YieldTypes eYieldType, const PlayerTypes eMinor, const PlayerTypes ePlayer, const bool isCaptured) const
{
	float value = 0;
	// defaults
	if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		value = 10;

	// player specific changes
	if (ePlayer != NO_PLAYER)
	{
		const CvPlayer& player = GET_PLAYER(ePlayer);
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			const bool hasPatronageFinisher = player.HasPolicy("POLICY_PATRONAGE_FINISHER");
			if (hasPatronageFinisher)
				value += 5;
		}		
	}

	return GC.round(value);
}
int CvGlobals::getYIELD_PER_QUEST(const YieldTypes eYieldType, const PlayerTypes eMinor, const PlayerTypes ePlayer) const
{
	float value = 0;
	// defaults
	if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		value = 100;
	if (eYieldType == YIELD_FRIENDSHIP)
		value = GC.getMINOR_FRIENDSHIP_FROM_TRADE_MISSION(); // 40

	// player specific changes
	if (ePlayer != NO_PLAYER)
	{
		const CvPlayer& player = GET_PLAYER(ePlayer);		
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			//POLICY_PHILANTHROPY grants 50% more Diplo Points from Completing Quests
			const bool hasPhilanthropy = player.HasPolicy("POLICY_PHILANTHROPY");
			if (hasPhilanthropy)
				value *= 1.5;
		}
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT)
		{

		}

		if (eYieldType == YIELD_GOLD)
		{
			// POLICY_PATRONAGE grants 100G from City-State Quests
			const bool hasPatronageOpener = player.HasPolicy("POLICY_PATRONAGE");
			if (hasPatronageOpener)
				value += 100;
		}
	}

	return GC.round(value);
}


// trade route modifier

int CvPlayerTrade::GetTradeConnectionValueExtra(const TradeConnection& kTradeConnection, const YieldTypes eYieldType, const bool bIsOwner) const
{
	float yieldChange = 0.0f;
	const CvPlayer& playerOrigin = GET_PLAYER(kTradeConnection.m_eOriginOwner);
	const CvPlayer& playerDest = GET_PLAYER(kTradeConnection.m_eDestOwner);
	const bool isInternal = playerOrigin.GetID() == playerDest.GetID();
	// true if the destination is a City State
	const bool isDestMinor = playerDest.isMinorCiv();
	const CvCity* cityOrigin = CvGameTrade::GetOriginCity(kTradeConnection);
	const CvCity* cityDest = CvGameTrade::GetDestCity(kTradeConnection);
	if (!cityOrigin || !cityDest) return 0;

	// how many tiles between the 2 cities
	const int tradeDistance = kTradeConnection.m_aPlotList.size();
	const bool hasSilkRoad = playerOrigin.HasPolicy("POLICY_CARAVANS");
	const bool hasMerchantConfederacy = playerOrigin.HasPolicy("POLICY_MERCHANT_CONFEDERACY");	
	const bool hasMerchantsGuild = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_CARAVANSARY"));
	const bool hasMarket = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_MARKET"));
	const bool hasBank = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_BANK"));
	const bool hasStockExchange = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_STOCK_EXCHANGE"));
	const bool hasMint = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_MINT"));
	const bool hasBrewery = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_BREWERY"));
	const bool hasStoneWorks = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_STONE_WORKS"));
	const bool hasTextileMill = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_TEXTILE"));
	const bool hasGrocer = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_GROCER"));
	const bool hasCenserMaker = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_CENSER"));
	const bool hasGemcutter = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_GEMCUTTER"));
	const bool hasOilRefinery = cityOrigin->GetCityBuildings()->HasBuildingClass(BuildingClass("BUILDINGCLASS_REFINERY"));


	if (isInternal) // true if this is an internal trade route
	{
		if (eYieldType == YIELD_GOLD && hasMint)
			yieldChange += 2;
		if (eYieldType == YIELD_GOLD && hasBrewery)
			yieldChange += 2;
		if (eYieldType == YIELD_PRODUCTION && hasStoneWorks)
			yieldChange += 1;
		if (eYieldType == YIELD_PRODUCTION && hasTextileMill)
			yieldChange += 1;
		if (eYieldType == YIELD_FOOD && hasGrocer)
			yieldChange += 2;
		if (eYieldType == YIELD_CULTURE && hasCenserMaker)
			yieldChange += 1;
		if (eYieldType == YIELD_CULTURE && hasGemcutter)
			yieldChange += 1;
		if (eYieldType == YIELD_PRODUCTION && hasOilRefinery)
			yieldChange += 3;
		{ // POLICY_FREE_THOUGHT +6SC +2FD from Internal Trade Routes
			const bool hasFreeThought = playerOrigin.HasPolicy("POLICY_FREE_THOUGHT");
			if (eYieldType == YIELD_FOOD && hasFreeThought)
				yieldChange += 2;
			if (eYieldType == YIELD_SCIENCE && hasFreeThought)
				yieldChange += 6;
		}
	}
	else
	{
		if (isDestMinor) // destination is City State
		{
			if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasMerchantConfederacy)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasMerchantConfederacy)
				yieldChange += 2;
			if (eYieldType == YIELD_PRODUCTION && hasMerchantConfederacy)
				yieldChange += 2;
			if (eYieldType == YIELD_CULTURE && hasMerchantConfederacy)
				yieldChange += 2;			
		}
		else // destination is another civ
		{
			if (eYieldType == YIELD_FOOD && hasSilkRoad)
				yieldChange += 3;
			if (eYieldType == YIELD_PRODUCTION && hasSilkRoad)
				yieldChange += 3;	
			if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasSilkRoad)
				yieldChange += 3;
		}

		{ // POLICY_AESTHETICS +2C and +2T from External Routes
			const bool hasAestheticsOpener = playerOrigin.HasPolicy("POLICY_AESTHETICS");
			if (eYieldType == YIELD_CULTURE && hasAestheticsOpener)
				yieldChange += 2;
			if (eYieldType == YIELD_TOURISM && hasAestheticsOpener)
				yieldChange += 2;
		}

		{ // POLICY_FREE_THOUGHT +1 Insight from External Routes
			const bool hasFreeThought = playerOrigin.HasPolicy("POLICY_FREE_THOUGHT");
			if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasFreeThought)
				yieldChange += 1;			
		}

		{ // diplomatic support from trade route buildings
			const int numDiploSupportBoosters = hasMerchantsGuild + hasMarket + hasBank + hasStockExchange + hasMint + hasBrewery + hasStoneWorks
				+ hasTextileMill + hasGrocer + hasCenserMaker + hasGemcutter + (hasOilRefinery * 2);
			if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
				yieldChange += numDiploSupportBoosters;
		}
	}


	return GC.round(yieldChange); // round to nearest integer
}


// modify unit instapop






