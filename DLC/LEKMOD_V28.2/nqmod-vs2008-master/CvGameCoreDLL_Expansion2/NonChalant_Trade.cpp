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


void appendNewLineOnly(stringstream* ss)
{
	if (ss != NULL)
		(*ss) << "[NEWLINE]";
}

void addValueLine(stringstream& s, bool isGreen, const long value, const string description)
{
	const long absValue = abs(value);
	s << "[NEWLINE]";
	if (isGreen) s << "[COLOR_POSITIVE_TEXT]+" << absValue;
	else		 s << "[COLOR_GREY]+" << absValue;

	s << " " << description << "[ENDCOLOR]";
}

// will construct a string
void appendNewLine(stringstream* ss, int* sum,
	// The number of additional cities the player could add.
	int numToAdd,
	// The description of where the numToAdd amount came from.
	string desc,
	bool isUnlocked)
{
	if (ss != NULL)
		addValueLine(*ss, isUnlocked, numToAdd, desc);
	if (isUnlocked)
		*sum += numToAdd;
}

int Water(const CvPlot& plot)
{
	if (plot.isOcean() || plot.isLake())
		return 1;
	return 0;
}
int MountainsAndHills(const CvPlot& plot)
{
	if (plot.isMountain())
		return 2;
	else if (plot.isHills())
		return 1;
	return 0;
}

int CvPlayerTrade::GetTradeConnectionValueExtra(const TradeConnection& kTradeConnection,
	const YieldTypes eYieldType,
	// If true, these yields are for the origin city, if false, they are for the destination city.
	const bool isForOriginYields, std::stringstream* tooltip) const
{
	int yieldChange = 0;
	const CvPlayer& playerOrigin = GET_PLAYER(kTradeConnection.m_eOriginOwner);
	const CvPlayer& playerDest = GET_PLAYER(kTradeConnection.m_eDestOwner);
	const bool isInternal = playerOrigin.GetID() == playerDest.GetID();
	// true if the destination is a City State
	const CvCity* cityOrigin = CvGameTrade::GetOriginCity(kTradeConnection);
	const CvCity* cityDest = CvGameTrade::GetDestCity(kTradeConnection);
	if (!cityOrigin || !cityDest) return 0;

	// how many tiles between the 2 cities
	//const int tradeDistance = kTradeConnection.m_aPlotList.size();
	//const int cityCrowsDistance = cityDest->distTo(cityDest);
	const bool hasMerchantConfederacy = playerOrigin.HasPolicy(POLICY_MERCHANT_CONFEDERACY);
	const bool hasMerchantsGuild = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CARAVANSARY);
	const bool hasMarket = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MARKET);
	const bool hasBank = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BANK);
	const bool hasStockExchange = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STOCK_EXCHANGE);
	const bool hasMint = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MINT);
	const bool hasBrewery = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BREWERY);
	const bool hasStoneWorks = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STONE_WORKS);
	const bool hasTextileMill = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_TEXTILE);
	const bool hasGrocer = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GROCER);
	const bool hasCenserMaker = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CENSER);
	const bool hasGemcutter = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GEMCUTTER);
	const bool hasOilRefinery = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_REFINERY);
	const bool hasShipyard = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_SHIPYARD);
	const bool hasHimeji = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_HIMEJI_CASTLE);
	const int numExplorationPolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_EXPLORATION);
	const int numCommercePolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_COMMERCE);
	const int numHonorPolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_HONOR);
	const bool hasTheocrary = playerOrigin.HasPolicy(POLICY_THEOCRACY);
	const int era = playerOrigin.GetCurrentEra();

	// determine type of route
	TradeRouteType type = kTradeConnection.GetRouteType();

	// internal trade routes benefit the destination, external benefit the source, this is true in both circumstances and false in the rest
	const bool isPrimaryYielder = (isInternal && !isForOriginYields) || (!isInternal && isForOriginYields);
	
	// 
	bool isTooltipStart = tooltip != NULL && isPrimaryYielder && eYieldType == YIELD_FOOD;
	if (isTooltipStart)
	{
		(*tooltip) << "{TXT_KEY_TRADEROUTE_TOOLTIP_HEADER}";
	}


	switch (type)
	{
	case TRADEROUTE_FOOD:
	{
		if (isPrimaryYielder) // only the destination city of the trade route gets these benefits
		{
			if (eYieldType == YIELD_FOOD)
			{
				appendNewLine(tooltip, &yieldChange, 3 + era, "[ICON_FOOD] Base (3 + 1 per era)", true);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the {TXT_KEY_BUILDING_GROCER}", hasGrocer);
				appendNewLineOnly(tooltip);
			}
			//if (eYieldType == YIELD_PRODUCTION)
			//{
			//	appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the {TXT_KEY_CIV5_BUILDINGS_STONE_WORKS}", hasStoneWorks);
			//	appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the {TXT_KEY_BUILDING_TEXTILE}", hasTextileMill);
			//	appendNewLine(tooltip, &yieldChange, +3, "[ICON_PRODUCTION] from the {TXT_KEY_BUILDING_REFINERY}", hasOilRefinery);
			//	appendNewLineOnly(tooltip);
			//}
			//if (eYieldType == YIELD_GOLD)
			//{
			//	appendNewLine(tooltip, &yieldChange, +2, "[ICON_GOLD] from the {TXT_KEY_BUILDING_MINT}", hasMint);
			//	appendNewLine(tooltip, &yieldChange, +2, "[ICON_GOLD] from the {TXT_KEY_BUILDING_BREWERY}", hasBrewery);
			//	appendNewLineOnly(tooltip);

			//	if (eYieldType == YIELD_GOLD)
			//		yieldChange += (numCommercePolicies * 3);
			//	if (eYieldType == YIELD_GOLD && hasHimeji)
			//		yieldChange += (numCommercePolicies + numHonorPolicies) * 2;
			//}
			if (eYieldType == YIELD_FAITH)
			{
				{
					int our = cityOrigin->plot()->countNearbyPlots(MountainsAndHills, 3);
					int their = cityDest->plot()->countNearbyPlots(MountainsAndHills, 3);
					int diff = max(0, 100 * (our - their));
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_FAITH] from our city having more Mountains (x2) and Hills", amount > 0);
				}

				appendNewLine(tooltip, &yieldChange, +2, "[ICON_FAITH] from {TXT_KEY_POLICY_THEOCRACY}", hasTheocrary);
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_GOLDEN)
			{
				{
					int our = cityOrigin->plot()->countNearbyPlots(Water, 3);
					int their = cityDest->plot()->countNearbyPlots(Water, 3);
					int diff = max(0, 100 * (our - their));
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_GOLDEN] from our city having more Water", amount > 0);
				}
			}

			//if (eYieldType == YIELD_CULTURE && hasCenserMaker)
			//	yieldChange += 1;
			//if (eYieldType == YIELD_CULTURE && hasGemcutter)
			//	yieldChange += 1;

			//if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasHimeji)
			//	yieldChange += (numCommercePolicies + numHonorPolicies);
			//if (eYieldType == YIELD_PRODUCTION && hasShipyard)
			//	yieldChange += 2;
		}
	}
	break;
	case TRADEROUTE_PRODUCTION:
	{
		if (isPrimaryYielder) // only the destination city of the trade route gets these benefits
		{
			if (eYieldType == YIELD_FOOD)
			{
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the {TXT_KEY_BUILDING_GROCER}", hasGrocer);
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_PRODUCTION)
			{
				appendNewLine(tooltip, &yieldChange, 4 + era, "[ICON_PRODUCTION] Base (4 + 1 per era)", true);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the {TXT_KEY_CIV5_BUILDINGS_STONE_WORKS}", hasStoneWorks);
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_FAITH)
			{
				appendNewLine(tooltip, &yieldChange, +2, "[ICON_PEACE] from {TXT_KEY_POLICY_THEOCRACY}", hasTheocrary);
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_CULTURE)
			{
				{
					int our = cityOrigin->getYieldRateTimes100(YIELD_CULTURE, true);
					int their = cityDest->getYieldRateTimes100(YIELD_CULTURE, true);
					int diff = max(0, (our - their)) / 4;
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_CULTURE] from our city having more Culture", amount > 0);
				}
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_SCIENTIFIC_INSIGHT)
			{
				{
					int our = cityOrigin->getPopulation();
					int their = cityDest->getPopulation();
					int diff = max(0, 100 * (their - our)) / 2;
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_SCIENTIFIC_INSIGHT] from the origin city having fewer Citizens", amount > 0);
				}
			}
			//if (eYieldType == YIELD_GOLD && hasMint)
			//	yieldChange += 2;
			//if (eYieldType == YIELD_GOLD && hasBrewery)
			//	yieldChange += 2;

			//if (eYieldType == YIELD_CULTURE && hasCenserMaker)
			//	yieldChange += 1;
			//if (eYieldType == YIELD_CULTURE && hasGemcutter)
			//	yieldChange += 1;
			//if (eYieldType == YIELD_PRODUCTION && hasOilRefinery)
			//	yieldChange += 3;
			//if (eYieldType == YIELD_GOLD && hasHimeji)
			//	yieldChange += (numCommercePolicies + numHonorPolicies) * 2;
			//if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasHimeji)
			//	yieldChange += (numCommercePolicies + numHonorPolicies);
			//if (eYieldType == YIELD_PRODUCTION && hasShipyard)
			//	yieldChange += 2;

			//if (eYieldType == YIELD_GOLD)
			//	yieldChange += (numCommercePolicies * 3);
		}
	}
	break;
	case TRADEROUTE_MINOR:
	{
		if (isPrimaryYielder) // only the origin city of the trade route gets these benefits
		{
			if (eYieldType == YIELD_GOLD)
			{
				int gold = (10 + (100 * GC.getPercentTurnsDoneT10000()) / 10000) / 2;
				appendNewLine(tooltip, &yieldChange, +gold, "[ICON_GOLD] from 50% x (10 + Percent Game Done)", true);


				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_FAITH)
			{
				appendNewLine(tooltip, &yieldChange, +2, "[ICON_PEACE] from {TXT_KEY_POLICY_THEOCRACY}", hasTheocrary);
				appendNewLineOnly(tooltip);
			}

			//if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasMerchantConfederacy)
			//	yieldChange += 2;
			//if (eYieldType == YIELD_FOOD && hasMerchantConfederacy)
			//	yieldChange += 2;
			//if (eYieldType == YIELD_PRODUCTION && hasMerchantConfederacy)
			//	yieldChange += 2;
			//if (eYieldType == YIELD_CULTURE && hasMerchantConfederacy)
			//	yieldChange += 2;
			//if (eYieldType == YIELD_FOOD)
			//	yieldChange += numExplorationPolicies;
			//if (eYieldType == YIELD_PRODUCTION)
			//	yieldChange += numExplorationPolicies;

			//{ // POLICY_FREE_THOUGHT +6SC +2FD from Trade Routes
			//	const bool hasFreeThought = playerOrigin.HasPolicy(POLICY_FREE_THOUGHT);
			//	if (eYieldType == YIELD_FOOD && hasFreeThought)
			//		yieldChange += 2;
			//	if (eYieldType == YIELD_SCIENCE && hasFreeThought)
			//		yieldChange += 6;
			//}
			if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
			{
				{ // city strength
					int our = cityOrigin->getStrengthValueT100();
					int their = cityDest->getStrengthValueT100();
					int diff = max(0, (our - their));
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_DIPLOMATIC_SUPPORT] from having more [ICON_STRENGTH] City Strength", amount > 0);
				}
			}
		}
	}
	break;
	case TRADEROUTE_MAJOR:
	{
		if (isPrimaryYielder) // only the origin city of the trade route gets these benefits
		{
			//if (eYieldType == YIELD_FOOD)
			//	yieldChange += numExplorationPolicies;

			if (eYieldType == YIELD_PRODUCTION)
			{
				appendNewLine(tooltip, &yieldChange, +numExplorationPolicies, "[ICON_PRODUCTION] from +1 per {TXT_KEY_POLICY_BRANCH_EXPLORATION} policy", numExplorationPolicies > 0);

				appendNewLineOnly(tooltip);
			}


			if (eYieldType == YIELD_GOLD)
			{
				int gold = 10 + (100 * GC.getPercentTurnsDoneT10000()) / 10000;
				appendNewLine(tooltip, &yieldChange, +gold, "[ICON_GOLD] from 10 + Percent Game Done", true);

				appendNewLineOnly(tooltip);

			}
			if (eYieldType == YIELD_SCIENCE)
			{
				{ // city strength
					int our = cityOrigin->getYieldRateTimes100(YIELD_SCIENCE, true);
					int their = cityDest->getYieldRateTimes100(YIELD_SCIENCE, true);
					int diff = max(0, (their - our)) / 2;
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_SCIENCE] from our city having less Science", amount > 0);
				}
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_FAITH)
			{
				appendNewLine(tooltip, &yieldChange, +2, "[ICON_FAITH] from {TXT_KEY_POLICY_THEOCRACY}", hasTheocrary);
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_TOURISM)
			{
				{
					int our = cityOrigin->getYieldRateTimes100(YIELD_CULTURE, true);
					int their = cityDest->getYieldRateTimes100(YIELD_CULTURE, true);
					int diff = max(0, (our - their)) / 4;
					int amount = (50 + iSquareRoot(diff)) / 100; // avoid insane via square root
					if (diff > 0) amount = max(1, amount);
					appendNewLine(tooltip, &yieldChange, +amount, "[ICON_CULTURAL_INFLUENCE] from our city having more Culture", amount > 0);
				}
			}

		}
	}
	break;
	default:
		break;
	}


	//if (isPrimaryYielder) // true if this is an internal trade route
	//{
	//	{ // POLICY_FREE_THOUGHT +6SC +2FD from ALL Trade Routes
	//		const bool hasFreeThought = playerOrigin.HasPolicy(POLICY_FREE_THOUGHT);
	//		if (eYieldType == YIELD_FOOD && hasFreeThought)
	//			yieldChange += 2;
	//		if (eYieldType == YIELD_SCIENCE && hasFreeThought)
	//			yieldChange += 6;
	//	}

	//	{ // diplomatic support from trade route buildings
	//		const int numDiploSupportBoosters = hasMerchantsGuild + hasMarket + hasBank + hasShipyard + hasStockExchange + hasMint + hasBrewery + hasStoneWorks
	//			+ hasTextileMill + hasGrocer + hasCenserMaker + hasGemcutter + (hasOilRefinery * 2);
	//		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
	//			yieldChange += numDiploSupportBoosters;
	//	}
	//}


	return yieldChange;
}
int CvPlayerTrade::GetRangeFactorT100() const
{
	return 250;
}

int CvPlayerTrade::GetNumRoutesAllowed(TradeRouteClassType type) const
{
	int numAllowed = (((GetNumTradeRoutesPossible() + 1) * 100) / 2) / 100; // since we round down, +1 forces a round up
	const CvPlayer& player = *m_pPlayer;
	
	int numCities = player.getNumCities();

	// Buildings Internal Trade Route Capacity = 2/3 of cities have building.
	{  
		int numLevel1TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_CARAVANSARY));
		if (type == TRADEROUTECLASS_INTERNAL && (numLevel1TradeBuildings * 3 >= numCities * 2))
			numAllowed += 1;
	}
	{
		int numLevel2TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_MARKET || BUILDINGCLASS_MINT || BUILDINGCLASS_BREWERY));
		if (type == TRADEROUTECLASS_INTERNAL && (numLevel2TradeBuildings * 3 >= numCities * 2))
			numAllowed += 1;
	}
	{
		int numLevel3TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_BANK));
		if (type == TRADEROUTECLASS_INTERNAL && (numLevel3TradeBuildings * 3 >= numCities * 2))
			numAllowed += 1;
	}
	{
		int numLevel4TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_STOCK_EXCHANGE));
		if (type == TRADEROUTECLASS_INTERNAL && (numLevel4TradeBuildings * 3 >= numCities * 2))
			numAllowed += 1;
	}

	// Buildings External Trade Route Capacity  = 1/3 of cities have building.
	{
		int numLevel1TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_CARAVANSARY));
		if (type == TRADEROUTECLASS_EXTERNAL && (numLevel1TradeBuildings * 3 >= numCities))
			numAllowed += 1;
	}
	{
		int numLevel2TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_MARKET || BUILDINGCLASS_MINT || BUILDINGCLASS_BREWERY));
		if (type == TRADEROUTECLASS_EXTERNAL && (numLevel2TradeBuildings * 3 >= numCities))
			numAllowed += 1;
	}
	{
		int numLevel3TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_BANK));
		if (type == TRADEROUTECLASS_EXTERNAL && (numLevel3TradeBuildings * 3 >= numCities))
			numAllowed += 1;
	}
	{
		int numLevel4TradeBuildings = player.countNumBuildingClasses(BuildingClassTypes(BUILDINGCLASS_STOCK_EXCHANGE));
		if (type == TRADEROUTECLASS_EXTERNAL && (numLevel4TradeBuildings * 3 >= numCities))
			numAllowed += 1;
	}

	// Wonders Trade Route Capacity
	{
		const bool hasColossus = player.HasWonder(BUILDINGCLASS_COLOSSUS);		
		if (type == TRADEROUTECLASS_EXTERNAL && hasColossus)
			numAllowed += 1;
	}
	{
		const bool hasMausoleum = player.HasWonder(BUILDINGCLASS_MAUSOLEUM_HALICARNASSUS);
		if (type == TRADEROUTECLASS_EXTERNAL && hasMausoleum)
			numAllowed += 1;
	}
	{
		const bool hasEastIndia = player.HasWonder(BUILDINGCLASS_NATIONAL_TREASURY);
		if (type == TRADEROUTECLASS_EXTERNAL && hasEastIndia)
			numAllowed += 1;
	}
	// Rome UB gets +1 additional
	{
		const bool hasEastIndia = player.HasWonder(BUILDINGCLASS_NATIONAL_TREASURY);
		const bool isRome = player.IsCiv(CIVILIZATION_ROME);
		if (type == TRADEROUTECLASS_EXTERNAL && hasEastIndia)
			numAllowed += 1;	}
	{
		const bool hasPetra = player.HasWonder(BUILDINGCLASS_PETRA);
		if (type == TRADEROUTECLASS_INTERNAL && hasPetra)
			numAllowed += 1;
	}
	{
		const bool hasAlthing = player.HasWonder(BUILDINGCLASS_ALTHING);
		if (type == TRADEROUTECLASS_INTERNAL && hasAlthing)
			numAllowed += 1;
	}
	{
		const bool hasIronworks = player.HasWonder(BUILDINGCLASS_IRONWORKS);
		if (type == TRADEROUTECLASS_INTERNAL && hasIronworks)
			numAllowed += 1;
	}


	return numAllowed;
}
//	--------------------------------------------------------------------------------
int CvPlayerTrade::GetTradeRouteRange(DomainTypes eDomain, const CvCity* pOriginCity) const
{
	int iRange = 0;

	int iBaseRange = 0;
	switch (eDomain)
	{
	case DOMAIN_SEA:
		iBaseRange = 16;
		break;
	case DOMAIN_LAND:
		iBaseRange = 16;
		break;
	default:
		CvAssertMsg(false, "Undefined domain for trade route range");
		return -1;
		break;
	}

	int iTraitRange = 0;
	switch (eDomain)
	{
	case DOMAIN_SEA:
		// not implemented
		iTraitRange = 0;
		break;
	case DOMAIN_LAND:
		iTraitRange = m_pPlayer->GetPlayerTraits()->GetLandTradeRouteRangeBonus();
		break;
	}

	CvPlayerTechs* pMyPlayerTechs = m_pPlayer->GetPlayerTechs();
	CvTeamTechs* pMyTeamTechs = GET_TEAM(GET_PLAYER(m_pPlayer->GetID()).getTeam()).GetTeamTechs();
	CvTechEntry* pTechInfo = NULL;

	int iExtendedRange = 0;
	for (int iTechLoop = 0; iTechLoop < pMyPlayerTechs->GetTechs()->GetNumTechs(); iTechLoop++)
	{
		TechTypes eTech = (TechTypes)iTechLoop;
		if (!pMyTeamTechs->HasTech(eTech))
		{
			continue;
		}

		pTechInfo = pMyPlayerTechs->GetTechs()->GetEntry(eTech);
		CvAssertMsg(pTechInfo, "null tech entry");
		if (pTechInfo)
		{
			iExtendedRange += pTechInfo->GetTradeRouteDomainExtraRange(eDomain);
		}
	}

	int iRangeModifier = 0;
	for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GET_PLAYER(pOriginCity->getOwner()).getCivilizationInfo().getCivilizationBuildings(iI);
		if (eBuilding != NO_BUILDING)
		{
			CvBuildingEntry* pBuildingEntry = GC.GetGameBuildings()->GetEntry(eBuilding);
			if (!pBuildingEntry)
			{
				continue;
			}

			if (pBuildingEntry && pOriginCity->GetCityBuildings()->GetNumBuilding((BuildingTypes)pBuildingEntry->GetID()))
			{
				if (pBuildingEntry->GetTradeRouteSeaDistanceModifier() > 0 && eDomain == DOMAIN_SEA)
				{
					iRangeModifier += pBuildingEntry->GetTradeRouteSeaDistanceModifier();
				}
				else if (pBuildingEntry->GetTradeRouteLandDistanceModifier() > 0 && eDomain == DOMAIN_LAND)
				{
					iRangeModifier += pBuildingEntry->GetTradeRouteLandDistanceModifier();
				}
			}
		}
	}

	iRange = iBaseRange;
	iRange += iTraitRange;
	iRange += iExtendedRange;
	iRange = (iRange * (100 + iRangeModifier)) / 100;
	return iRange;
}

