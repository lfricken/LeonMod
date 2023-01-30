
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



// extra yields for plots
int CvPlot::getExtraYield
(
	// type of yield we are considering
	const YieldTypes eYieldType,
	// type of improvement
	const ImprovementTypes eImprovement,
	// type of route (road, railroad, none)
	const RouteTypes,
	// owning player
	const PlayerTypes tileOwner
)
{
	const CvPlot& plot = *this;
	// city that is/could work this tile
	const CvCity* pWorkingCity = plot.getWorkingCity();
	// true if a is/could work this tile
	//const bool hasAWorkingCity = pWorkingCity != NULL;
	// true if this tile is not pillaged
	//const bool isNotPillaged = !plot.IsRoutePillaged() && !plot.IsImprovementPillaged();
	// true if we have a road or railroad
	//const bool hasAnyRoute = eRouteType != NO_ROUTE && isNotPillaged;
	// true if this tile has any improvements
	const bool hasAnyImprovement = eImprovement != NO_IMPROVEMENT;
	// true if this is the actual city tile (not just a surrounding tile)
	const bool isCityCenter = plot.getPlotCity() != NULL;
	// true if this tile has any atoll on it
	const bool hasAnyAtoll = plot.HasAnyAtoll();
	const CvImprovementEntry* pImprovement = NULL;
	string improvementName = ""; // <ImprovementType>
	if (hasAnyImprovement)
	{
		pImprovement = GC.getImprovementInfo((ImprovementTypes)m_eImprovementType);
		if (pImprovement != NULL)
		{
			improvementName = pImprovement->GetType();
		}
	}

	int yieldChange = 0;


	const bool isTundra = plot.HasTerrain(TERRAIN_TUNDRA);
	//const bool isSnow = plot.HasTerrain(TERRAIN_SNOW);
	const bool isDesert = plot.HasTerrain(TERRAIN_DESERT);
	const bool hasBonus = plot.HasResourceClass("RESOURCECLASS_BONUS"); // has a bonus resource
	const bool hasLuxury = plot.HasResourceClass("RESOURCECLASS_LUXURY"); // has a luxury resource
	const bool hasStrategic = plot.HasResourceClass("RESOURCECLASS_RUSH") || plot.HasResourceClass("RESOURCECLASS_MODERN"); // has any strategic resource
	const bool noResource = !hasBonus && !hasLuxury && !hasStrategic; // no resource of any kind (might have artifacts though)


	// depends on player
	if (tileOwner != NO_PLAYER)
	{
		const CvPlayer& player = GET_PLAYER(tileOwner);
		const int numCityStateAllies = player.GetNumMinorAllies();
		// depends on city
		if (pWorkingCity != NULL)
		{
			const CvCity& city = *pWorkingCity;
			const bool isCapital = city.isCapital();
			const ReligionTypes majorityReligion = city.GetCityReligions()->GetReligiousMajority(); // the majority religion in this city
			//const int numCitiesFollowing = GC.getGame().GetGameReligions()->GetNumCitiesFollowing(majorityReligion); // number of cities with this as majority
			const int numCityStatesFollowing = GC.getGame().GetGameReligions()->GetNumCitiesFollowing(majorityReligion, true); // number of city states with this as majority
			const bool isHolyCity = city.GetCityReligions()->IsHolyCityForReligion(majorityReligion); // true if this is the holy city of the majority religion in this city
			const int numFollowersLocal = city.GetCityReligions()->GetNumFollowers(majorityReligion); // number of people following the majority religion in this city
			const CvGameReligions& worldReligions = *GC.getGame().GetGameReligions();
			const int numFollowersGlobal = worldReligions.GetNumFollowers(majorityReligion); // number of people following the majority religion in this city globaly
			//const int cityPopulation = city.getPopulation(); // number of people in this city
			const int numTradeCityStates = player.GetTrade()->GetNumberOfCityStateTradeRoutes(); // number of trade routes we have with city states
			const int numTradeMajorCivs = player.GetTrade()->GetNumForeignTradeRoutes(player.GetID()) - numTradeCityStates; // number of trade routes we have with other civ players (not city states)
			//player.IsCiv("CIVILIZATION_AMERICA");
			//player.HasTech("TECH_POTTERY");


			const bool isGreatTile = plot.HasImprovement("IMPROVEMENT_ACADEMY") ||
				plot.HasImprovement("IMPROVEMENT_CUSTOMS_HOUSE") ||
				plot.HasImprovement("IMPROVEMENT_MANUFACTORY") ||
				plot.HasImprovement("IMPROVEMENT_HOLY_SITE") ||
				plot.HasImprovement("IMPROVEMENT_DOCK") ||
				plot.HasImprovement("IMPROVEMENT_CHILE_DOCK") ||
				plot.HasImprovement("IMPROVEMENT_SACRED_GROVE");




			if (isCityCenter)
			{ // BEGIN isCityCenter

				{// POLICY_CONSULATES - gives +3C to the Palace for each City-State Ally
					const bool hasConsulates = player.HasPolicy("POLICY_CONSULATES");
					if (eYieldType == YIELD_CULTURE && hasConsulates && isCapital) // && isCityCenter
						yieldChange += (numCityStateAllies * 3);
				}

				{// BELIEF_Religious Community - gives 1 diplo point per 6 followers (max 20)
					const bool hasBeliefReligiousCommunity = city.HasBelief("BELIEF_RELIGIOUS_COMMUNITY");
					if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasBeliefReligiousCommunity && isHolyCity) // && isCityCenter
						yieldChange += min(20, numFollowersGlobal / 3);
				}

				{// BELIEF_Church Property - Holy City - +1 (food, production, gold, faith, culture, and science) and an additional of each yield per 40 followers (max +5)
					const bool hasBeliefChurchProperty = city.HasBelief("BELIEF_CHURCH_PROPERTY");
					const bool isTileAffected = hasBeliefChurchProperty && isHolyCity;
					const bool isYieldAffected = (
						eYieldType == YIELD_FOOD || eYieldType == YIELD_PRODUCTION || eYieldType == YIELD_CULTURE ||
						eYieldType == YIELD_FAITH || eYieldType == YIELD_SCIENCE || eYieldType == YIELD_GOLD);
					if (isYieldAffected && isTileAffected) // && isCityCenter
						yieldChange += min(6, 1 + numFollowersGlobal / 50);
				}

				{// BELIEF_Dawah - Holy City - 2 Culture, 1 Culture per 8 followers (max 20)
					const bool hasBeliefDawah = city.HasBelief("BELIEF_DAWAHH");
					if (eYieldType == YIELD_CULTURE && hasBeliefDawah && isHolyCity) // && isCityCenter
						yieldChange += 2 + min(20, numFollowersGlobal / 8);
				}

				{// BELIEF_initiation rites - renamed Religios Scholars - Holy City - 2 Science, 1 science per 3 followers (max 100)
					const bool hasBeliefReligiousScholars = city.HasBelief("BELIEF_INITIATION_RITES");
					if (eYieldType == YIELD_SCIENCE && hasBeliefReligiousScholars && isHolyCity) // && isCityCenter
						yieldChange += 2 + min(100, numFollowersGlobal / 3);
				}

				{// BELIEF_MISSIONARY_ZEAL - Holy City - 2 tourism and 1 tourism per 12 followers (max 20)
					const bool hasBeliefMissionaryZeal = city.HasBelief("BELIEF_MISSIONARY_ZEAL");
					if (eYieldType == YIELD_TOURISM && hasBeliefMissionaryZeal && isHolyCity) // && isCityCenter
						yieldChange += 2 + min(20, numFollowersGlobal / 12);
				}

				{// BELIEF_MITHRAEA - Holy City - 3 food and 1 food per 6 followers (max 30)
					const bool hasBeliefMithraea = city.HasBelief("BELIEF_MITHRAEA");
					if (eYieldType == YIELD_FOOD && hasBeliefMithraea && isHolyCity) // && isCityCenter
						yieldChange += 3 + min(30, numFollowersGlobal / 6);
				}

				{// BELIEF_HEATHEN_CONVERSION - renamed religious troubarods - Trade Routes - 3 faith, 3 diplo points, +2 trade routes
					const bool hasBeliefReligiousTroubadors = city.HasBelief("BELIEF_HEATHEN_CONVERSION");
					if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasBeliefReligiousTroubadors && isHolyCity) // && isCityCenter
						yieldChange += ((numTradeMajorCivs + numTradeCityStates) * 3);
				}

				{// BELIEF_RELIGIOUS_UNITY - +4 diplo for foreign city following (max 32) 
					const bool hasBeliefReligiousUnity = city.HasBelief("BELIEF_RELIGIOUS_UNITY");
					if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasBeliefReligiousUnity && isHolyCity) // && isCityCenter
						yieldChange += (numCityStatesFollowing * 4);
				}

				{// BELIEF_SALATT - Holy City - 2 Production, 1 Production per 8 followers (max 20)
					const bool hasBeliefSalatt = city.HasBelief("BELIEF_SALATT");
					if (eYieldType == YIELD_PRODUCTION && hasBeliefSalatt && isHolyCity) // && isCityCenter
						yieldChange += 2 + min(20, numFollowersGlobal / 8);
				}

				{// BELIEF_TITHE - Holy City - 4 Gold, 1 Gold per 4 followers (max 40)
					const bool hasBeliefTithe = city.HasBelief("BELIEF_TITHE");
					if (eYieldType == YIELD_GOLD && hasBeliefTithe && isHolyCity) // && isCityCenter
						yieldChange += 4 + min(40, numFollowersGlobal / 4);
				}

				{// BELIEF_ZAKATT - Holy City - 1 Scientific Influence, 1 per 16 followers (max 10)
					const bool hasBeliefZakatt = city.HasBelief("BELIEF_ZAKATT");
					if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasBeliefZakatt && isHolyCity) // && isCityCenter
						yieldChange += 1 + min(10, numFollowersGlobal / 16);
				}

				{// BELIEF_DEFENDER_FAITH - gives 2FH for every 3 Followers in Holy City. 
					const bool hasBeliefDefenderOfTheFaith = city.HasBelief("BELIEF_DEFENDER_FAITH");
					if (eYieldType == YIELD_FAITH && hasBeliefDefenderOfTheFaith && isHolyCity) // && isCityCenter
						yieldChange += (numFollowersLocal / 3) * 2;
				}

				{// BELIEF_KARMA - gives 1C for every 2 Followers in Holy City. 
					const bool hasBeliefKarma = city.HasBelief("BELIEF_KARMA");
					if (eYieldType == YIELD_CULTURE && hasBeliefKarma && isHolyCity) // && isCityCenter
						yieldChange += (numFollowersLocal / 2);
				}

				{// BELIEF_PROMISED_LAND - gives 1G for every follower in Holy City. 
					const bool hasBeliefPromisedLand = city.HasBelief("BELIEF_PROMISED_LAND");
					if (eYieldType == YIELD_GOLD && hasBeliefPromisedLand && isHolyCity) // && isCityCenter
						yieldChange += (numFollowersLocal);
				}

				{// POLICY_IRON_CURTAIN - gives +2 tourism per city
					const bool hasIronCurtain = player.HasPolicy("POLICY_IRON_CURTAIN");
					if (eYieldType == YIELD_TOURISM && hasIronCurtain) // && isCityCenter
						yieldChange += 2;
				}

				{// POLICY_SPACEFLIGHT_PIONEERS - gives +1 scientific insight per city
					const bool hasSpaceFlightPioneers = player.HasPolicy("POLICY_SPACEFLIGHT_PIONEERS");
					if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasSpaceFlightPioneers) // && isCityCenter
						yieldChange += 1;
				}

				{// POLICY_ORGANIZED_RELIGION gives +1C for every 3 Followers in Holy city if you have adopted a religion				
					const bool hasOrganizedReligion = player.HasPolicy("POLICY_ORGANIZED_RELIGION");
					if (eYieldType == YIELD_CULTURE && hasOrganizedReligion && isHolyCity) // && isCityCenter
						yieldChange += (numFollowersLocal / 3);
				}

				{// BUILDINGCLASS_BOROBUDUR - 1 FH per city
					const bool hasBorobudur = player.HasWonder(BuildingClass("BUILDINGCLASS_BOROBUDUR"));
					if (eYieldType == YIELD_FAITH && hasBorobudur) // && isCityCenter
						yieldChange += 1;
				}

				{// BUILDINGCLASS_MACHU_PICHU - 2G per city
					const bool hasMachuPichu = player.HasWonder(BuildingClass("BUILDINGCLASS_MACHU_PICHU"));
					if (eYieldType == YIELD_GOLD && hasMachuPichu) // && isCityCenter
						yieldChange += 2;
				}

				{// BUILDINGCLASS_PYRAMID - 1PD per city
					const bool hasPyramids = player.HasWonder(BuildingClass("BUILDINGCLASS_PYRAMID"));
					if (eYieldType == YIELD_PRODUCTION && hasPyramids) // && isCityCenter
						yieldChange += 1;
				}

				{// BUILDINGCLASS_TEMPLE_ARTEMIS - 1FD per city
					const bool hasTempleOfArtemis = player.HasWonder(BuildingClass("BUILDINGCLASS_TEMPLE_ARTEMIS"));
					if (eYieldType == YIELD_FOOD && hasTempleOfArtemis) // && isCityCenter
						yieldChange += 1;
				}
			} // END isCityCenter

			{// BELIEF_Messiah - renamed Sacrificial Altars - Holy Sites - 1 faith, culture, tourism, and 1 additioanl for 40 followers (max 3)
				const bool hasBeliefSacrificialAltars = city.HasBelief("BELIEF_MESSIAH");
				const bool isHolySite = plot.HasImprovement("IMPROVEMENT_HOLY_SITE");
				if (eYieldType == YIELD_CULTURE && hasBeliefSacrificialAltars && isHolySite)
					yieldChange += 1 + min(3, numFollowersGlobal / 40);
				if (eYieldType == YIELD_FAITH && hasBeliefSacrificialAltars && isHolySite)
					yieldChange += 1 + min(3, numFollowersGlobal / 40);
				if (eYieldType == YIELD_TOURISM && hasBeliefSacrificialAltars && isHolySite)
					yieldChange += 1 + min(3, numFollowersGlobal / 40);
			}

			{// POLICY_NEW_DEAL - GP Tile +2 Atoll Yields, +2 Tourism from Natural Wonders
				const bool hasNewDeal = player.HasPolicy("POLICY_NEW_DEAL");
				const bool isAtoll = plot.HasFeature("FEATURE_ATOLL");
				const bool isAtollCulture = plot.HasFeature("FEATURE_ATOLL_CULTURE");
				const bool isAtollProduction = plot.HasFeature("FEATURE_ATOLL_PRODUCTION");
				const bool isAtollGold = plot.HasFeature("FEATURE_ATOLL_GOLD");
				const bool isAtollScience = plot.HasFeature("FEATURE_ATOLL_SCIENCE");
				const bool isNaturalWonder = plot.HasAnyNaturalWonder();
				if (eYieldType == YIELD_TOURISM && hasNewDeal && isNaturalWonder)
					yieldChange += 2;
				if (eYieldType == YIELD_FAITH && hasNewDeal && isAtoll)
					yieldChange += 2;
				if (eYieldType == YIELD_CULTURE && hasNewDeal && isAtollCulture)
					yieldChange += 2;
				if (eYieldType == YIELD_PRODUCTION && hasNewDeal && isAtollProduction)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasNewDeal && isAtollGold)
					yieldChange += 2;
				if (eYieldType == YIELD_SCIENCE && hasNewDeal && isAtollScience)
					yieldChange += 2;
			}

			{// BELIEF_DANCE_AURORA - Dance of the Aurora - on tundra tiles only - gives +1 production to bonus tiles, +1 culture to luxury tiles, +1 Gold to Strategic, and +1 faith to every other tundra tile
				const bool hasBeliefDanceOfTheAurora = city.HasBelief("BELIEF_DANCE_AURORA");
				if (eYieldType == YIELD_FAITH && hasBeliefDanceOfTheAurora && isTundra && noResource)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasBeliefDanceOfTheAurora && isTundra && hasBonus)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && hasBeliefDanceOfTheAurora && isTundra && hasLuxury)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasBeliefDanceOfTheAurora && isTundra && hasStrategic)
					yieldChange += 1;
			}

			{// BELIEF_DESERT_FOLKLORE - Desert Folklore - on Desert tiles only - gives +1 production to bonus tiles, +1 culture to luxury tiles, +1 Gold to Strategic, and +1 faith to every other Desert tile
				const bool hasBeliefDesertFolklore = city.HasBelief("BELIEF_DESERT_FOLKLORE");
				if (eYieldType == YIELD_FAITH && hasBeliefDesertFolklore && isDesert && noResource)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasBeliefDesertFolklore && isDesert && hasBonus)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && hasBeliefDesertFolklore && isDesert && hasLuxury)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasBeliefDesertFolklore && isDesert && hasStrategic)
					yieldChange += 1;
			}

			{// BELIEF_SACRED_WATERS - gives one tourism from lake and atoll tiles. Could change to lake and oasis in future if Atolls seems too good. Features don't work right now though. 
				const bool hasBeliefSacredWaters = city.HasBelief("BELIEF_SACRED_WATERS");
				const bool isLake = plot.isLake();
				const bool isOasis = plot.HasFeature("FEATURE_OASIS");
				if (eYieldType == YIELD_TOURISM && hasBeliefSacredWaters && (isLake || hasAnyAtoll || isOasis))
					yieldChange += 1;
			}

			{// Policy_Cutural Exchange - gives 1 tourism to great person tile improvements. 
				const bool hasPolicyCulturalExchange = player.HasPolicy("POLICY_ETHICS");
				if (eYieldType == YIELD_TOURISM && hasPolicyCulturalExchange && isGreatTile)
					yieldChange += 1;
			}

			{// POLICY_MEDIA_CULTURE - gives 3 tourism to great person tile improvements. 
				const bool hasMediaCulture = player.HasPolicy("POLICY_MEDIA_CULTURE");
				if (eYieldType == YIELD_TOURISM && hasMediaCulture && isGreatTile)
					yieldChange += 3;
			}

			{ // POLICY_SPACE_PROCUREMENTS - gives 3 Singularity Points per Acadamy. 
				const bool hasSpaceProcurement = player.HasPolicy("POLICY_SPACE_PROCUREMENTS");
				const bool isAcadamy = plot.HasImprovement("IMPROVEMENT_ACADEMY");
				if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasSpaceProcurement && isAcadamy)
					yieldChange += 3;
			}

			{// POLICY_NEW_ORDER - gives +3 tourism, culture, diplo points, gold to citadels
				const bool hasNewOrder = player.HasPolicy("POLICY_NEW_ORDER");
				const bool isCitadel = plot.HasImprovement("IMPROVEMENT_CITADEL");
				if (eYieldType == YIELD_TOURISM && hasNewOrder && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_CULTURE && hasNewOrder && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasNewOrder && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_GOLD && hasNewOrder && isCitadel)
					yieldChange += 3;
			}			

			{// POLICY_HONOR_FINISHER - gives +3 PD, SC, C to Citadels
				const bool hasHonorFinisher = player.HasPolicy("POLICY_HONOR_FINISHER");
				const bool isCitadel = plot.HasImprovement("IMPROVEMENT_CITADEL");
				if (eYieldType == YIELD_PRODUCTION && hasHonorFinisher && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_CULTURE && hasHonorFinisher && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_SCIENCE && hasHonorFinisher && isCitadel)
					yieldChange += 3;
			}

			{// POLICY_EXPLORATION_FINISHER gives +1C from Coastal Luxuries, +1PD from Atolls, 2PD, 2G from DryDocks				
				const bool hasExplorationFinisher = player.HasPolicy("POLICY_EXPLORATION_FINISHER");
				//const bool isFish = plot.HasResource("RESOURCE_FISH"); 
				const bool isCoastalLuxury = (plot.HasResource("RESOURCE_CRAB") || plot.HasResource("RESOURCE_WHALE") || plot.HasResource("RESOURCE_PEARLS")
					|| plot.HasResource("RESOURCE_CORAL"));
				const bool isDryDock = (plot.HasImprovement("IMPROVEMENT_DOCK") || plot.HasImprovement("IMPROVEMENT_CHILE_DOCK"));
				if (eYieldType == YIELD_CULTURE && hasExplorationFinisher && isCoastalLuxury)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasExplorationFinisher && hasAnyAtoll)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasExplorationFinisher && isDryDock)
					yieldChange += 2;
				if (eYieldType == YIELD_PRODUCTION && hasExplorationFinisher && isDryDock)
					yieldChange += 2;
			}

			{// POLICY_COMMERCE_FINISHER gives +1PD to mines withour resources, and +1FD to Farms without Resources or Flood Plains				
				const bool hasCommerceFinisher = player.HasPolicy("POLICY_COMMERCE_FINISHER");
				const bool isFarm = plot.HasImprovement("IMPROVEMENT_FARM");
				const bool isMine = plot.HasImprovement("IMPROVEMENT_MINE");
				const bool isFloodPlains = plot.HasFeature("FEATURE_FLOOD_PLAINS");
				if (eYieldType == YIELD_FOOD && hasCommerceFinisher && isFarm && noResource && !isFloodPlains)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasCommerceFinisher && isMine && noResource)
					yieldChange += 1;				
			}

			{// IMPROVEMENT_POLDER - gives +2 Food to Marsh.
				const bool isMarsh = plot.HasFeature("FEATURE_MARSH");
				const bool isPolder = plot.HasImprovement("IMPROVEMENT_POLDER");
				if (eYieldType == YIELD_FOOD && isMarsh && isPolder)
					yieldChange += 2;
			}

			{// BUILDINGCLASS_PORCELAIN_TOWER - 3SC per Lucury
				const bool hasPorcelinTower = player.HasWonder(BuildingClass("BUILDINGCLASS_PORCELAIN_TOWER"));
				if (eYieldType == YIELD_SCIENCE && hasPorcelinTower && hasLuxury)
					yieldChange += 3;
			}

			{// CIVILIZATION_MC_SCOTLAND - 1 FD 1 PD from Cattle, Sheep, After Trapping
				const bool hasCattle = plot.HasResource("RESOURCE_COW");
				const bool hasSheep = plot.HasResource("RESOURCE_SHEEP");
				const bool isEngland = player.IsCiv("CIVILIZATION_MC_SCOTLAND");
				const bool hasTrapping = player.HasTech("TECH_TRAPPING");
				if (eYieldType == YIELD_FOOD && isEngland && hasTrapping && (hasCattle || hasSheep))
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && isEngland && hasTrapping && (hasCattle || hasSheep))
						yieldChange += 1;
			}	

			{// CIVILIZATION_RUSSIA - 1 PD from Strategic Resources (only available after you can see Horse or Iron, etc.)
				const bool isRussia = player.IsCiv("CIVILIZATION_RUSSIA");				
				if (eYieldType == YIELD_PRODUCTION && isRussia && hasStrategic)
					yieldChange += 1;
			}

			{// CIVILIZATION_FRANCE - 1 G from Strategic Resources (only available after you can see Horse or Iron, etc.)
				const bool isFrance = player.IsCiv("CIVILIZATION_FRANCE");
				if (eYieldType == YIELD_GOLD && isFrance && hasStrategic)
					yieldChange += 1;
			}

			{// CIVILIZATION_INDONESIA - +1 to Atolls +2G from Coastal Luxes After Sailing
				const bool isAtoll = plot.HasFeature("FEATURE_ATOLL");
				const bool isAtollCulture = plot.HasFeature("FEATURE_ATOLL_CULTURE");
				const bool isAtollProduction = plot.HasFeature("FEATURE_ATOLL_PRODUCTION");
				const bool isAtollGold = plot.HasFeature("FEATURE_ATOLL_GOLD");
				const bool isAtollScience = plot.HasFeature("FEATURE_ATOLL_SCIENCE");
				const bool isCoastalLuxury = (plot.HasResource("RESOURCE_CRAB") || plot.HasResource("RESOURCE_WHALE") || plot.HasResource("RESOURCE_PEARLS")
					|| plot.HasResource("RESOURCE_CORAL"));
				const bool isIndonesia = player.IsCiv("CIVILIZATION_INDONESIA");
				const bool hasSailing = player.HasTech("TECH_SAILING");
				if (eYieldType == YIELD_GOLD && isIndonesia && hasSailing && isCoastalLuxury)
					yieldChange += 2;
				if (eYieldType == YIELD_FAITH && isIndonesia && hasSailing && isAtoll)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && isIndonesia && hasSailing && isAtollCulture)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && isIndonesia && hasSailing && isAtollProduction)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && isIndonesia && hasSailing && isAtollGold)
					yieldChange += 1;
				if (eYieldType == YIELD_SCIENCE && isIndonesia && hasSailing && isAtollScience)
					yieldChange += 1;				
			}

			{// CIVILIZATION_INDIA - 2FD 1C PD from Cattle After Animal Husbandry
				const bool hasCattle = plot.HasResource("RESOURCE_COW");
				const bool isIndia = player.IsCiv("CIVILIZATION_INDIA");
				const bool hasAnimalHusbandry = player.HasTech("TECH_ANIMAL_HUSBANDRY");
				if (eYieldType == YIELD_CULTURE && isIndia && hasAnimalHusbandry && hasCattle)
					yieldChange += 1;
				if (eYieldType == YIELD_FAITH && isIndia && hasAnimalHusbandry && hasCattle)
					yieldChange += 2;				
			}

			{// CIVILIZATION_DENMARK - 1PD from fish. After Sailing
				const bool hasFish = plot.HasResource("RESOURCE_FISH");
				const bool isDenmark = player.IsCiv("CIVILIZATION_DENMARK");
				const bool hasSailing = player.HasTech("TECH_SAILING");
				if (eYieldType == YIELD_PRODUCTION && isDenmark && hasSailing && hasFish)
					yieldChange += 1;				
			}

			{// CIVILIZATION_AZTEC - 1C from Jungle Lux 1PD from Bannanas After Calendar
				const bool hasBanana = plot.HasResource("RESOURCE_BANANA");
				const bool isAztec = player.IsCiv("CIVILIZATION_AZTEC");
				const bool hasCalendar = player.HasTech("TECH_CALENDAR");
				const bool isJungleLuxury = (plot.HasResource("RESOURCE_DYE") || plot.HasResource("RESOURCE_GEMS") || plot.HasResource("RESOURCE_SPICES") 
					|| plot.HasResource("RESOURCE_TRUFFLES") || plot.HasResource("RESOURCE_COCOA") || plot.HasResource("RESOURCE_COCONUT"));
				if (eYieldType == YIELD_PRODUCTION && isAztec && hasCalendar && hasBanana)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && isAztec && hasCalendar && isJungleLuxury)
					yieldChange += 1;
			}

			{// CIVILIZATION_EGYPT - 2PD from Stone After Masonry
				const bool hasStone = plot.HasResource("RESOURCE_STONE");
				const bool isEgypt = player.IsCiv("CIVILIZATION_EGYPT");
				const bool hasMasonry = player.HasTech("TECH_MASONRY");
				if (eYieldType == YIELD_PRODUCTION && isEgypt && hasMasonry && hasStone)
					yieldChange += 2;
			}

			{// CIVILIZATION_UC_TURKEY - 1PD 1G from Wheat and Oasis After Pottery
				const bool hasWheat = plot.HasResource("RESOURCE_WHEAT");
				const bool isMiddleEast = player.IsCiv("CIVILIZATION_UC_TURKEY");
				const bool hasPottery = player.HasTech("TECH_POTTERY");				
				const bool isOasis = plot.HasFeature("FEATURE_OASIS");
				if (eYieldType == YIELD_PRODUCTION && isMiddleEast && hasPottery && (hasWheat || isOasis))
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && isMiddleEast && hasPottery && (hasWheat || isOasis))
					yieldChange += 1;
			}

			{// CIVILIZATION_BYZANTIUM - 1PD 1C from Horse After The Wheel
				const bool hasHorse = plot.HasResource("RESOURCE_HORSE");
				const bool hasMaize = plot.HasResource("RESOURCE_MAIZE");
				const bool isByzantium = player.IsCiv("CIVILIZATION_BYZANTIUM");
				const bool hasTheWheel = player.HasTech("TECH_THE_WHEEL");	
				const bool hasPottery = player.HasTech("TECH_POTTERY");
				if (eYieldType == YIELD_PRODUCTION && isByzantium && hasTheWheel && hasHorse)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && isByzantium && hasTheWheel && hasHorse)
					yieldChange += 1;
				if (eYieldType == YIELD_FOOD && isByzantium && hasPottery && hasMaize)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && isByzantium && hasPottery && hasMaize)
					yieldChange += 1;
			}

			{// CIVILIZATION_IROQUOIS - 2G 1 C from Lakes After Trappingislake
				const bool isLake = plot.isLake();				
				const bool isAmerica = player.IsCiv("CIVILIZATION_IROQUOIS");
				const bool hasTrapping = player.HasTech("TECH_TRAPPING");				
				if (eYieldType == YIELD_CULTURE && hasTrapping && isAmerica && isLake)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasTrapping && isAmerica && isLake)
					yieldChange += 1;				
			}

			{// CIVILIZATION_PRUSSIA - 2G from Luxuries
				const bool isPrussia = player.IsCiv("CIVILIZATION_PRUSSIA");				
				if (eYieldType == YIELD_GOLD && isPrussia && hasLuxury)
					yieldChange += 2;
			}

			{// CARD_ANCIENT_RESOURCES_INUITS - 1C to Tundra
				const bool hasInuits = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_INUITS_PASSIVE");
				if (eYieldType == YIELD_CULTURE && hasInuits && isTundra && (hasLuxury || hasStrategic || hasBonus))
					yieldChange += 1;				
			}
			
			{// CARD_ANCIENT_RESOURCES_BEDOUINS - 1C to Desert
				const bool hasBedouins = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_BEDOUINS_PASSIVE");
				if (eYieldType == YIELD_CULTURE && hasBedouins && isDesert && (hasLuxury || hasStrategic || hasBonus))
					yieldChange += 1;
			}

			{// CARD_ANCIENT_RESOURCES_ATLATL - 2F to Deer
				const bool hasAtlatl = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_ATLATL_PASSIVE");
				const bool isDeer = plot.HasResource("RESOURCE_DEER");
				if (eYieldType == YIELD_FOOD && hasAtlatl && isDeer)
					yieldChange += 2;
			}

			{// CARD_ANCIENT_RESOURCES_FLINT_KNAPPING - 2PD from Obsidian 1G from Quarries
				const bool hasFlints = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_FLINT_KNAPPING_PASSIVE");
				const bool isObsidian = plot.HasResource("RESOURCE_OBSIDIAN");
				const bool isQuarry = plot.HasImprovement("IMPROVEMENT_QUARRY");
				if (eYieldType == YIELD_PRODUCTION && hasFlints && isObsidian)
					yieldChange += 2;
				if (eYieldType == YIELD_PRODUCTION && hasFlints && isQuarry)
					yieldChange += 1;
			}			

			{// CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS - -1FD +4FH to sheep
				const bool hasSacrificialLambs = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS_PASSIVE");
				const bool isSheep = plot.HasResource("RESOURCE_SHEEP");				
				if (eYieldType == YIELD_FOOD && hasSacrificialLambs && isSheep)
					yieldChange -= 1;
				if (eYieldType == YIELD_FAITH && hasSacrificialLambs && isSheep)
					yieldChange += 4;
			}

			{// CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS - -1FD +4FH to sheep
				const bool hasSpearfishing = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_SPEAR_FISHING_PASSIVE");
				const bool isFish = plot.HasResource("RESOURCE_FISH");
				if (eYieldType == YIELD_FOOD && hasSpearfishing && isFish)
					yieldChange += 2;				
			}

			{// CARD_ANCIENT_RESOURCES_DIVINE_CREATION - +4C +4FH from Natural Wonders
				const bool hasDivineCreation = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_DIVINE_CREATION_PASSIVE");
				const bool isNaturalWonder = plot.HasAnyNaturalWonder();
				if (eYieldType == YIELD_CULTURE && hasDivineCreation && isNaturalWonder)
					yieldChange += 4;
				if (eYieldType == YIELD_FAITH && hasDivineCreation && isNaturalWonder)
					yieldChange += 4;
			}

		}
	}

	// does not depend on player` 

	{ // don't stack lake and atoll yields
		if (plot.isLake() && hasAnyAtoll)
		{
			// remove whatever the lake would have given
			const CvYieldInfo& kYield = *GC.getYieldInfo(eYieldType);
			yieldChange -= kYield.getLakeChange();
		}
	}

	return yieldChange;
}

