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
	
	const bool hasEastIndia = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_NATIONAL_TREASURY);
	
	const bool hasCaravansary = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CARAVANSARY);
	const bool hasMarket = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MARKET);
	const bool hasMint = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MINT);
	const bool hasBrewery = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BREWERY);
	const bool hasBank = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BANK);
	const bool hasSeaport = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_SEAPORT);
	const bool hasStockExchange = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STOCK_EXCHANGE);

	const bool toCaravansary = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CARAVANSARY);
	const bool toMarket = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MARKET);
	const bool toMint = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_MINT);
	const bool toBrewery = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BREWERY);
	const bool toBank = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_BANK);
	const bool toSeaport = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_SEAPORT);
	const bool toStockExchange = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STOCK_EXCHANGE);
	
	const bool hasGranary = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GRANARY);
	const bool hasFishery = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FISHERY);
	const bool hasHuntersHaven = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_HUNTERS_HAVEN);
	const bool hasGrocer = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GROCER);
	const bool hasGrainElevator = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GRAIN_ELEVATOR);
	const bool hasStockyards= cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STOCKYARDS);

	const bool toGranary = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GRANARY);
	const bool toFishery = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FISHERY);
	const bool toHuntersHaven = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_HUNTERS_HAVEN);
	const bool toGrocer = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GROCER);
	const bool toGrainElevator = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GRAIN_ELEVATOR);
	const bool toStockyards = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STOCKYARDS);
	
	const bool hasStoneWorks = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STONE_WORKS);
	const bool hasStable = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STABLE);
	const bool hasForge = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FORGE);
	const bool hasFactory = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FACTORY);
	const bool hasShipyard = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_SHIPYARD);
	const bool hasTextileMill = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_TEXTILE);
	const bool hasOilRefinery = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_REFINERY);
	const bool hasNuclearPlant = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_NUCLEAR_PLANT);

	const bool toStoneWorks = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STONE_WORKS);
	const bool toStable = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_STABLE);
	const bool toForge = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FORGE);
	const bool toFactory = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_FACTORY);
	const bool toShipyard = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_SHIPYARD);
	const bool toTextileMill = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_TEXTILE);
	const bool toOilRefinery = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_REFINERY);
	const bool toNuclearPlant = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_NUCLEAR_PLANT);
	
	const bool hasCenserMaker = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CENSER);
	const bool hasGemcutter = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GEMCUTTER);

	const bool toCenserMaker = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_CENSER);
	const bool toGemcutter = cityDest->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_GEMCUTTER);

	const bool hasCivilService = playerOrigin.HasTech(TECH_CIVIL_SERVICE);
	const bool hasRefrigeration = playerOrigin.HasTech(TECH_REFRIGERATION);
	const bool hasMetalCasting = playerOrigin.HasTech(TECH_METAL_CASTING);
	const bool hasCombustion = playerOrigin.HasTech(TECH_COMBUSTION);

	const bool hasTheocrary = playerOrigin.HasPolicy(POLICY_THEOCRACY);
	const bool hasJustice = playerOrigin.HasPolicy(POLICY_LEGALISM);
	const bool hasFreeThought = playerOrigin.HasPolicy(POLICY_FREE_THOUGHT);
	const bool hasCollectiveRule = playerOrigin.HasPolicy(POLICY_COLLECTIVE_RULE);
	const bool hasMerchantConfederacy = playerOrigin.HasPolicy(POLICY_MERCHANT_CONFEDERACY);	
	
	const bool hasHimeji = cityOrigin->GetCityBuildings()->HasBuildingClass(BUILDINGCLASS_HIMEJI_CASTLE);
	const int numExplorationPolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_EXPLORATION);
	const int numCommercePolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_COMMERCE);
	const int numHonorPolicies = playerOrigin.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_HONOR);
	
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
				appendNewLine(tooltip, &yieldChange, 3, "[ICON_FOOD] Base", true);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Granary", (hasGranary && !toGranary) || (!hasGranary && toGranary));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Hunter's Haven", (hasHuntersHaven && !toHuntersHaven) || (!hasHuntersHaven && toHuntersHaven));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Fishery", (hasFishery && !toFishery) || (!hasFishery && toFishery));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Grocer", (hasGrocer && !toGrocer) || (!hasGrocer && toGrocer));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Brewery", (hasBrewery && !toBrewery) || (!hasBrewery && toBrewery));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Grain Elevator", (hasGrainElevator && !toGrainElevator) || (!hasGrainElevator && toGrainElevator));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from the Stockyards", (hasStockyards && !toStockyards) || (!hasStockyards && toStockyards));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from Civil Service", hasCivilService);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_FOOD] from Refrigeration", hasRefrigeration);
				appendNewLineOnly(tooltip);
			}			
			
			int ourMountainNumber = cityOrigin->plot()->countNearbyPlots(MountainsAndHills, 3);
			int theirMountainNumber = cityDest->plot()->countNearbyPlots(MountainsAndHills, 3);
			int diffMountains = max(0, 100 * (ourMountainNumber - theirMountainNumber));
			int ourWaterNumber = cityOrigin->plot()->countNearbyPlots(Water, 3);
			int theirWaterNumber = cityDest->plot()->countNearbyPlots(Water, 3);
			int diffWater = max(0, 100 * (ourWaterNumber - theirWaterNumber));
			
			if (eYieldType == YIELD_FAITH)
			{
				int faithAmount = (50 + iSquareRoot(diffMountains)) / 100; // avoid insane via square root
				if (diffMountains <= diffWater)  faithAmount = 0; // one or the other boosted, not both
				if (diffMountains <= 200) faithAmount = 0; // 3 minimum differential
						{
							appendNewLine(tooltip, &yieldChange, +faithAmount, "[ICON_FAITH] from our city having more Mountains (x2) and Hills", faithAmount > 0);
						}				

				appendNewLine(tooltip, &yieldChange, +3, "[ICON_FAITH] from {TXT_KEY_POLICY_THEOCRACY}", (hasTheocrary && faithAmount > 0));
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_GOLDEN)
			{
				int goldenAmount = (50 + iSquareRoot(diffWater)) / 100; // avoid insane via square root
				if (diffWater <= diffMountains) goldenAmount = 0; // one or the other boosted, not both				
				if (diffWater <= 200) goldenAmount = 0; // 3 minimum differential
							{
								appendNewLine(tooltip, &yieldChange, +goldenAmount, "[ICON_GOLDEN] from our city having more Water", goldenAmount > 0);
							}			

				appendNewLine(tooltip, &yieldChange, +3, "[ICON_GOLDEN] from {TXT_KEY_POLICY_LEGALISM}", (hasJustice && goldenAmount > 0));
				appendNewLineOnly(tooltip);
			}			
		}
	}
	break;
	case TRADEROUTE_PRODUCTION:
	{
		if (isPrimaryYielder) // only the destination city of the trade route gets these benefits
		{
			if (eYieldType == YIELD_PRODUCTION)
			{
				appendNewLine(tooltip, &yieldChange, 3, "[ICON_PRODUCTION] Base", true);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Stoneworks", (hasStoneWorks && !toStoneWorks) || (!hasStoneWorks && toStoneWorks));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Stable", (hasStable && !toStable) || (!hasStable && toStable));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Forge", (hasForge && !toForge) || (!hasForge && toForge));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Mint", (hasMint && !toMint) || (!hasMint && toMint));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Factory", (hasFactory && !toFactory) || (!hasFactory && toFactory));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Textile Mill", (hasTextileMill && !toTextileMill) || (!hasTextileMill && toTextileMill));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from the Shipyard", (hasShipyard && !toShipyard) || (!hasShipyard && toShipyard));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from Metal Casting", hasMetalCasting);
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_PRODUCTION] from Combustion", hasCombustion);				
				appendNewLineOnly(tooltip);
			}			
			
			int ourCulture = cityOrigin->getYieldRateTimes100(YIELD_CULTURE, true);
			int theirCulture = cityDest->getYieldRateTimes100(YIELD_CULTURE, true);
			int diffCulture = max(0, (ourCulture - theirCulture)) / 4;
			int ourPopulation = cityOrigin->getPopulation();
			int theirPopulation = cityDest->getPopulation();
			int diffPopulation = max(0, 100 * (theirPopulation - ourPopulation)) / 2;
									
			if (eYieldType == YIELD_CULTURE)
			{
				int cultureAmount = (50 + iSquareRoot(diffCulture)) / 100; // avoid insane via square root
				if (diffCulture <= diffPopulation) cultureAmount = 0; // one or the other boosted, not both
				if (diffCulture <= 200) cultureAmount = 0; // 3 minimum differential
				{					
					appendNewLine(tooltip, &yieldChange, +cultureAmount, "[ICON_CULTURE] from our city having more Culture", cultureAmount > 0);
				}
				appendNewLine(tooltip, &yieldChange, +2, "[ICON_CULTURE] from Collective Rule", (hasCollectiveRule && cultureAmount > 0));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_CULTURE] from Gemcutter", (hasGemcutter && !toGemcutter) || (!hasGemcutter && toGemcutter));
				appendNewLine(tooltip, &yieldChange, +1, "[ICON_CULTURE] from Censer Maker", (hasCenserMaker && !toCenserMaker) || (!toCenserMaker && toCenserMaker));
				appendNewLineOnly(tooltip);
			}
			if (eYieldType == YIELD_SCIENTIFIC_INSIGHT)
			{
				int insightAmount = (50 + iSquareRoot(diffPopulation)) / 100; // avoid insane via square root
				if (diffPopulation <= diffCulture) insightAmount = 0;
				if (diffPopulation <= 200) insightAmount = 0;
				{						
						appendNewLine(tooltip, &yieldChange, +insightAmount, "[ICON_SCIENTIFIC_INSIGHT] from the origin city having fewer Citizens", insightAmount > 0);
				}
				appendNewLine(tooltip, &yieldChange, +2, "[ICON_SCIENTIFIC_INSIGHT] from Free Thought", (hasFreeThought && insightAmount > 0));
				appendNewLineOnly(tooltip);
			}			
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

	// Building Grant Trade Routes +4/+4 = 8 Possible Internal + External
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

	// Wonders Trade Route Capacity +2/+2 possible from World and +1/+1 possible from National
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
	{
		const bool hasEastIndia = player.HasWonder(BUILDINGCLASS_NATIONAL_TREASURY);
		if (type == TRADEROUTECLASS_EXTERNAL && hasEastIndia)
			numAllowed += 1;
	}
	// Civ Rome Unique Building gets +1 additional
	{
		const bool hasGrandCanal = player.HasWonder(BUILDINGCLASS_NATIONAL_TREASURY);
		const bool isRome = player.IsCiv(CIVILIZATION_ROME);
		if (type == TRADEROUTECLASS_EXTERNAL && hasGrandCanal && isRome)
			numAllowed += 1;
	}	
	// Traits - Ottomons +1 additional
	{
		const bool isOttomons = player.IsCiv(CIVILIZATION_UC_TURKEY);
		if (type == TRADEROUTECLASS_EXTERNAL && isOttomons)
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
		iBaseRange = 10;
		break;
	case DOMAIN_LAND:
		iBaseRange = 6;
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
		iTraitRange = m_pPlayer->GetPlayerTraits()->GetSeaTradeRouteRangeBonus();
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
	iRange += iRangeModifier;	
	return iRange;
}

