
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

			{// BELIEF_GODDESS_HUNT - gives 1FH to Camps with Luxuries. 
				const bool hasBeliefGoddessHunt = city.HasBelief("BELIEF_GODDESS_HUNT");
				const bool hasCamp = plot.HasImprovement("IMPROVEMENT_CAMP");
				if (eYieldType == YIELD_FAITH && hasBeliefGoddessHunt && hasLuxury && hasCamp)
					yieldChange += 1;
			}

			{// BELIEF_STONE_CIRCLES - gives 1FH, +1G to Quarries with Luxuries. 
				const bool hasBeliefStoneCircles = city.HasBelief("BELIEF_STONE_CIRCLES");
				const bool hasQuarry = plot.HasImprovement("IMPROVEMENT_QUARRY");
				const bool hasRockHewn = plot.HasImprovement("IMPROVEMENT_ROCK_HEWN");
				if (eYieldType == YIELD_FAITH && hasBeliefStoneCircles && hasLuxury && (hasQuarry || hasRockHewn))
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasBeliefStoneCircles && hasLuxury && (hasQuarry || hasRockHewn))
					yieldChange += 1;
			}

			{// BELIEF_SPIRITUALS - gives +2C +2FH to Holy Sites. +2FH to all other GP Tiles 
				const bool hasBeliefSpirituals = city.HasBelief("BELIEF_SPIRITUALS");
				const bool hasholysite = plot.HasImprovement("IMPROVEMENT_HOLY_SITE");				
				if (eYieldType == YIELD_FAITH && hasBeliefSpirituals && isGreatTile)
					yieldChange += 3;
				if (eYieldType == YIELD_CULTURE && hasBeliefSpirituals && hasholysite)
					yieldChange += 2;
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
					yieldChange += 2;
			}

			{ // POLICY_HUMANISM - gives 3SC Aluminum 1 Insight Uranium 
				const bool hasHumansim = player.HasPolicy("POLICY_HUMANISM");				
				const bool isAluminum = plot.HasResource("RESOURCE_ALUMINUM");
				const bool isUranium = plot.HasResource("RESOURCE_URANIUM");
				if (eYieldType == YIELD_SCIENCE && hasHumansim && isAluminum)
					yieldChange += 3;
				if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasHumansim && isUranium)
					yieldChange += 1;
			}

			{ // POLICY_SECULARISM - gives 3SC Aluminum 1 Insight Uranium 
				const bool hasHumansim = player.HasPolicy("POLICY_SECULARISM");
				const bool isSubmarineVolcano = plot.HasFeature("FEATURE_ATOLL_SCIENCE");			
				if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && hasHumansim && isSubmarineVolcano)
					yieldChange += 1;
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

			{// POLICY_CARAVANS gives +1PD to mines without resources				
				const bool hasCaravans = player.HasPolicy("POLICY_CARAVANS");
				const bool isMine = plot.HasImprovement("IMPROVEMENT_MINE");
				if (eYieldType == YIELD_PRODUCTION && hasCaravans && isMine && noResource)
					yieldChange += 1;				
			}

			{// IMPROVEMENT_POLDER - gives +2 Food to Marsh.
				const bool isMarsh = plot.HasFeature("FEATURE_MARSH");
				const bool isPolder = plot.HasImprovement("IMPROVEMENT_POLDER");
				if (eYieldType == YIELD_FOOD && isMarsh && isPolder)
					yieldChange += 2;
			}

			{// POLICY_NAVAL_TRADITION - gives +1 PD and +1G to Coast
				const bool hasNavalTradition = player.HasPolicy("POLICY_NAVAL_TRADITION");
				const bool isCoast = plot.HasTerrain(TERRAIN_COAST) || plot.HasTerrain(TERRAIN_OCEAN);
				const bool isPolder = plot.HasImprovement("IMPROVEMENT_POLDER");
				const bool isAyer = plot.HasImprovement("IMPROVEMENT_AYER");
				const bool isDryDock = plot.HasImprovement("IMPROVEMENT_DOCK") || plot.HasImprovement("IMPROVEMENT_CHILE_DOCK");
				if (eYieldType == YIELD_PRODUCTION && hasNavalTradition && isCoast && !hasLuxury && !isPolder && !isAyer && !isDryDock)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasNavalTradition && isCoast && !hasLuxury && !isPolder && !isAyer && !isDryDock)
					yieldChange += 1;				
			}

			{// POLICY_NAVIGATION_SCHOOL gives +1FD to farms without resources				
				const bool hasNaigationSchool = player.HasPolicy("POLICY_NAVIGATION_SCHOOL");
				const bool isFarm = plot.HasImprovement("IMPROVEMENT_FARM");
				const bool isFloodPlains = plot.HasFeature("FEATURE_FLOOD_PLAINS");
				if (eYieldType == YIELD_FOOD && hasNaigationSchool && isFarm && noResource && !isFloodPlains)
					yieldChange += 1;				
			}

			{// POLICY_TREASURE_FLEETS gives +2PD +2C to Coal and Oil				
				const bool hasTreasureFleets = player.HasPolicy("POLICY_TREASURE_FLEETS");
				const bool isCoal = plot.HasResource("RESOURCE_COAL");
				const bool isOil = plot.HasResource("RESOURCE_OIL");			
				if (eYieldType == YIELD_PRODUCTION && hasTreasureFleets && (isCoal || isOil))
					yieldChange += 2;
				if (eYieldType == YIELD_CULTURE && hasTreasureFleets && (isCoal || isOil))
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
					|| plot.HasResource("RESOURCE_COCOA") || plot.HasResource("RESOURCE_COCONUT") || plot.HasResource("RESOURCE_SUGAR") 
					|| plot.HasResource("RESOURCE_CITRUS") || plot.HasResource("RESOURCE_SILK"));
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

			{// CARD_ANCIENT_RESOURCES_ATLATL - 1FD to Improved Deer
				const bool hasAtlatl = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_ATLATL_PASSIVE");
				const bool isDeer = plot.HasResource("RESOURCE_DEER");
				const bool isCamp = plot.HasImprovement("IMPROVEMENT_CAMP");
				if (eYieldType == YIELD_FOOD && hasAtlatl && isDeer && isCamp)
					yieldChange += 1;
			}

			{// CARD_ANCIENT_RESOURCES_FLINT_KNAPPING - 1PD from Obsidian 1PD from Quarries
				const bool hasFlints = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_FLINT_KNAPPING_PASSIVE");
				const bool isObsidian = plot.HasResource("RESOURCE_OBSIDIAN");
				const bool isQuarry = plot.HasImprovement("IMPROVEMENT_QUARRY");
				if (eYieldType == YIELD_PRODUCTION && hasFlints && isObsidian && isQuarry)
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasFlints && isQuarry)
					yieldChange += 1;
			}			

			{// CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS - -1FD +4FH to sheep
				const bool hasSacrificialLambs = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_SACRIFICIAL_LAMBS_PASSIVE");
				const bool isSheep = plot.HasResource("RESOURCE_SHEEP");
				const bool isPasture = plot.HasImprovement("IMPROVEMENT_PASTURE");
				if (eYieldType == YIELD_FAITH && hasSacrificialLambs && isSheep && isPasture)
					yieldChange += 2;
			}

			{// CARD_ANCIENT_RESOURCES_SPEAR_FISHING +1FD to fish
				const bool hasSpearfishing = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_SPEAR_FISHING_PASSIVE");
				const bool isFish = plot.HasResource("RESOURCE_FISH");
				if (eYieldType == YIELD_FOOD && hasSpearfishing && isFish)
					yieldChange += 1;				
			}

			{// CARD_ANCIENT_RESOURCES_DIVINE_CREATION - +2C +4FH from Natural Wonders
				const bool hasDivineCreation = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_DIVINE_CREATION_PASSIVE");
				const bool isNaturalWonder = plot.HasAnyNaturalWonder();
				if (eYieldType == YIELD_CULTURE && hasDivineCreation && isNaturalWonder)
					yieldChange += 2;
				if (eYieldType == YIELD_FAITH && hasDivineCreation && isNaturalWonder)
					yieldChange += 4;
			}

			{// CARD_ANCIENT_POLITICAL_ORTHODOXY - +3 FD to City Center
				const bool hasOrthodoxyCard = player.HasPolicy("POLICY_CARD_ANCIENT_POLITICAL_ORTHODOXY_PASSIVE");				
				if (eYieldType == YIELD_FOOD && hasOrthodoxyCard && isCityCenter)
					yieldChange += 1;				
			}

			{// POLICY_CARD_ANCIENT_POLITICAL_PROGRESSIVE_PASSIVE - +1 PD to City Center
				const bool hasProgressiveCard = player.HasPolicy("POLICY_CARD_ANCIENT_POLITICAL_PROGRESSIVE_PASSIVE");
				if (eYieldType == YIELD_PRODUCTION && hasProgressiveCard && isCityCenter)
					yieldChange += 1;
			}

			{// CARD_ANCIENT_POLITICAL_AGGRESIVE - +1 C to City Center
				const bool hasAgressiveCard = player.HasPolicy("POLICY_CARD_ANCIENT_POLITICAL_AGGRESIVE_PASSIVE");
				if (eYieldType == YIELD_CULTURE && hasAgressiveCard && isCityCenter)
					yieldChange += 1;
			}

			{// CARD_ANCIENT_POLITICAL_EXCLUSIVE - +4 FD to Capital City Center
				const bool hasExclusiveCard = player.HasPolicy("POLICY_CARD_ANCIENT_POLITICAL_EXCLUSIVE_PASSIVE");
				if (eYieldType == YIELD_FOOD && hasExclusiveCard && isCityCenter && isCapital)
					yieldChange += 4;
			}

			{// CARD_CLASSICAL_RESOURCE_LIMESTONE - +3 PD to Stone
				const bool hasLimtestoneCard = player.HasPolicy("POLICY_CARD_CLASSICAL_RESOURCE_LIMESTONE_PASSIVE");
				const bool isStone = plot.HasResource("RESOURCE_STONE");
				if (eYieldType == YIELD_PRODUCTION && hasLimtestoneCard && isStone)
					yieldChange += 3;
			}

			{// CARD_MEDIEVAL_RESOURCE_GILLNETS - +2 PD to Fish
				const bool hasGillnetsCard = player.HasPolicy("POLICY_CARD_MEDIEVAL_RESOURCE_GILLNETS_PASSIVE");
				const bool isFish = plot.HasResource("RESOURCE_FISH");
				if (eYieldType == YIELD_PRODUCTION && hasGillnetsCard && isFish)
					yieldChange += 1;
			}

			{// CARD_MEDIEVAL_RESOURCE_PRECIOUS_METALS - +2C +2G to Metals
				const bool hasPreciousCard = player.HasPolicy("POLICY_CARD_MEDIEVAL_RESOURCE_PRECIOUS_METALS_PASSIVE");
				const bool isPreciousMetal = plot.HasResource("RESOURCE_GOLD") || 
					plot.HasResource("RESOURCE_SILVER") ||
					plot.HasResource("RESOURCE_GEMS");
				if (eYieldType == YIELD_CULTURE && hasPreciousCard && isPreciousMetal)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasPreciousCard && isPreciousMetal)
					yieldChange += 2;
			}

			{// CARD_MEDIEVAL_RESOURCE_SILK_ROAD - +2C +2G to Silk Road
				const bool hasSilkRoadCard = player.HasPolicy("POLICY_CARD_MEDIEVAL_RESOURCE_SILK_ROAD_PASSIVE");
				const bool isSilkyResource = plot.HasResource("RESOURCE_SILK") ||
					plot.HasResource("RESOURCE_JADE") ||
					plot.HasResource("RESOURCE_DYE") ||
					plot.HasResource("RESOURCE_PERFUME");
				if (eYieldType == YIELD_CULTURE && hasSilkRoadCard && isSilkyResource)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasSilkRoadCard && isSilkyResource)
					yieldChange += 2;
			}

			{// CARD_MEDIEVAL_RESOURCE_SPICE_TRADE - +2C +1FD to Spice Trade
				const bool hasSpiceTradeCard = player.HasPolicy("POLICY_CARD_MEDIEVAL_RESOURCE_SPICE_TRADE_PASSIVE");
				const bool isSpiceyResource = plot.HasResource("RESOURCE_SPICES") ||
					plot.HasResource("RESOURCE_INCENSE") ||
					plot.HasResource("RESOURCE_COCONUT") ||
					plot.HasResource("RESOURCE_BANANA");
				if (eYieldType == YIELD_CULTURE && hasSpiceTradeCard && isSpiceyResource)
					yieldChange += 2;
				if (eYieldType == YIELD_FOOD && hasSpiceTradeCard && isSpiceyResource)
					yieldChange += 1;
			}

			{// CARD_MEDIEVAL_RESOURCE_VITICULTURE - +2C +2FD to Wine. +1G Farms.
				const bool hasViticultureCard = player.HasPolicy("POLICY_CARD_MEDIEVAL_RESOURCE_VITICULTURE_PASSIVE");
				const bool isWine = plot.HasResource("RESOURCE_WINE");
				const bool isOlive = plot.HasResource("RESOURCE_OLIVE");
				const bool hasFarm = plot.HasImprovement("IMPROVEMENT_FARM");
				if (eYieldType == YIELD_GOLD && hasViticultureCard && hasFarm)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && hasViticultureCard && (isWine || isOlive))
					yieldChange += 2;
				if (eYieldType == YIELD_FOOD && hasViticultureCard && (isWine || isOlive))
					yieldChange += 2;				
			}

			{// CARD_ANCIENT_RESOURCES_INUITS - +2PD to Whales.
				const bool hasInuitsCard = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_INUITS_PASSIVE");
				const bool isWhale = plot.HasResource("RESOURCE_WHALE");				
				if (eYieldType == YIELD_PRODUCTION && hasInuitsCard && isWhale)
					yieldChange += 1;
			}

			{// CARD_ANCIENT_RESOURCES_INUITS - +2PD to Whales.
				const bool hasBedouinsCard = player.HasPolicy("POLICY_CARD_ANCIENT_RESOURCES_BEDOUINS_PASSIVE");
				const bool isOases = plot.HasFeature("FEATURE_OASIS");
				if (eYieldType == YIELD_FOOD && hasBedouinsCard && isOases)
					yieldChange += 1;
			}

			{// CARD_RENAISSANCE_RESOURCES_TRIANGULAR_TRADE - +1 FD, PD, G to a bunch of stuff
				const bool hasTriangularTradeCard = player.HasPolicy("POLICY_CARD_RENAISSANCE_RESOURCES_TRIANGULAR_TRADE_PASSIVE");
				const bool isTriangularResource = (plot.HasResource("RESOURCE_SUGAR") || plot.HasResource("RESOURCE_TOBACCO") ||
					plot.HasResource("RESOURCE_TEA") || plot.HasResource("RESOURCE_COFFEE") || plot.HasResource("RESOURCE_COCOA"));
				if (eYieldType == YIELD_FOOD && hasTriangularTradeCard && isTriangularResource) 
					yieldChange += 1;
				if (eYieldType == YIELD_PRODUCTION && hasTriangularTradeCard && isTriangularResource)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasTriangularTradeCard && isTriangularResource)
					yieldChange += 1;
			}

			{// CARD_INDUSTRIAL_RESOURCES_GREAT_AWAKENING - +5 FH, +3 C Holy Sites
				const bool hasGreatAwakeningCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_RESOURCES_GREAT_AWAKENING_PASSIVE");
				const bool isHolySite = plot.HasImprovement("IMPROVEMENT_HOLY_SITE");					
				if (eYieldType == YIELD_FAITH && hasGreatAwakeningCard && isHolySite)
					yieldChange += 5;
				if (eYieldType == YIELD_CULTURE && hasGreatAwakeningCard && isHolySite)
					yieldChange += 3;				
			}

			{// CARD_INDUSTRIAL_RESOURCES_WAR_MEMORIALS - +5 GD +3 C +3 T Citadels
				const bool hasWarMemorialsCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_RESOURCES_WAR_MEMORIALS_PASSIVE");
				const bool isCitadel = plot.HasImprovement("IMPROVEMENT_CITADEL");
				if (eYieldType == YIELD_GOLD && hasWarMemorialsCard && isCitadel)
					yieldChange += 5;
				if (eYieldType == YIELD_CULTURE && hasWarMemorialsCard && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_TOURISM && hasWarMemorialsCard && isCitadel)
					yieldChange += 3;
			}

			{// CARD_INDUSTRIAL_RESOURCES_ASSEMBLY_LINE - +5 PD, +5 GD
				const bool hasAssemblyLineCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_RESOURCES_ASSEMBLY_LINE_PASSIVE");
				const bool isManufactory = plot.HasImprovement("IMPROVEMENT_MANUFACTORY");
				if (eYieldType == YIELD_PRODUCTION && hasAssemblyLineCard && isManufactory)
					yieldChange += 5;
				if (eYieldType == YIELD_GOLD && hasAssemblyLineCard && isManufactory)
					yieldChange += 5;				
			}

			{// CARD_INDUSTRIAL_RESOURCES_JOHN_JACOB_ASTOR - +1C  +1 PD, +2 GD to Furs
				const bool hasJohnJaobAstorCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_RESOURCES_JOHN_JACOB_ASTOR_PASSIVE");
				const bool isFursEtc = plot.HasResource("RESOURCE_FURS") || plot.HasResource("RESOURCE_BISON")
					|| plot.HasResource("RESOURCE_DEER") || plot.HasResource("RESOURCE_SALT");
				if (eYieldType == YIELD_PRODUCTION && hasJohnJaobAstorCard && isFursEtc)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && hasJohnJaobAstorCard && isFursEtc)
					yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasJohnJaobAstorCard && isFursEtc)
					yieldChange += 2;				
			}

			{// CARD_INDUSTRIAL_RESOURCES_HYDRAULIC_FRACKING - +5 PD to Oil Wells
				const bool hasHydraulicFrackingCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_RESOURCES_HYDRAULIC_FRACKING_PASSIVE");
				const bool isOilWell = plot.HasImprovement("IMPROVEMENT_WELL");
				if (eYieldType == YIELD_PRODUCTION && hasHydraulicFrackingCard && isOilWell)
					yieldChange += 5;				
			}	

			{// CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY - +2 PD +2 G to Cotton
				const bool hasEliWhitneyCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY_PASSIVE");
				const bool isCotton = plot.HasResource("RESOURCE_COTTON");
				if (eYieldType == YIELD_PRODUCTION && hasEliWhitneyCard && isCotton)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasEliWhitneyCard && isCotton)
					yieldChange += 2;
			}

			{// CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE - +3 PD +3 G to Copper
				const bool hasThomasDoolittleCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE_PASSIVE");
				const bool isCopper = plot.HasResource("RESOURCE_COPPER");
				if (eYieldType == YIELD_PRODUCTION && hasThomasDoolittleCard && isCopper)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasThomasDoolittleCard && isCopper)
					yieldChange += 2;
			}

			{// CARD_MODERN_RESOURCES_MODU - +5 PD +5 G to Offshore Wells
				const bool hasModuCard = player.HasPolicy("POLICY_CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY_PASSIVE");
				const bool isOffshoreWell = plot.HasImprovement("IMPROVEMENT_OFFSHORE_PLATFORM");
				if (eYieldType == YIELD_PRODUCTION && hasModuCard && isOffshoreWell)
					yieldChange += 5;
				if (eYieldType == YIELD_GOLD && hasModuCard && isOffshoreWell)
					yieldChange += 5;
			}

			{// CARD_MODERN_RESOURCES_GREAT_WESTERN_TRAIL - +2 PD +2 G to Cattle, Horses
				const bool hasGreatWesternTrailCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_GREAT_WESTERN_TRAIL_PASSIVE");
				const bool isCow = plot.HasResource("RESOURCE_COW");
				const bool isHorse = plot.HasResource("RESOURCE_HORSE");
				if (eYieldType == YIELD_PRODUCTION && hasGreatWesternTrailCard && isCow)
					yieldChange += 2;
				if (eYieldType == YIELD_GOLD && hasGreatWesternTrailCard && isCow)
					yieldChange += 2;
				if (eYieldType == YIELD_PRODUCTION && hasGreatWesternTrailCard && isHorse)
					yieldChange += 1;
				if (eYieldType == YIELD_CULTURE && hasGreatWesternTrailCard && isHorse)
					yieldChange += 1;
			}

			{// CARD_MODERN_RESOURCES_NITRATES - +1 FD +1 PD to Farms
				const bool hasNitratesCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_NITRATES_PASSIVE");
				const bool isFarm = plot.HasImprovement("IMPROVEMENT_FARM");
				if (eYieldType == YIELD_PRODUCTION && hasNitratesCard && isFarm)
					yieldChange += 1;
				if (eYieldType == YIELD_FOOD && hasNitratesCard && isFarm)
					yieldChange += 1;
			}

			{// CARD_MODERN_RESOURCES_HENRY_FORD - +4 PD +4 G to Rubber
				const bool hasHenryFordCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_HENRY_FORD_PASSIVE");
				const bool isRubber = plot.HasResource("RESOURCE_RUBBER");				
				if (eYieldType == YIELD_PRODUCTION && hasHenryFordCard && isRubber)
					yieldChange += 4;
				if (eYieldType == YIELD_GOLD && hasHenryFordCard && isRubber)
					yieldChange += 4;				
			}

			{// CARD_MODERN_RESOURCES_COMMERCIAL_FISHING - +2 FD +1 PD +2 G to Fish, Crabs
				const bool hasCommercialFishinCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_COMMERCIAL_FISHING_PASSIVE");
				const bool isCrab = plot.HasResource("RESOURCE_CRAB");
				const bool isFish = plot.HasResource("RESOURCE_FISH");
				if (eYieldType == YIELD_FOOD && hasCommercialFishinCard && (isCrab || isFish))
					yieldChange += 2;
				if (eYieldType == YIELD_PRODUCTION && hasCommercialFishinCard && (isCrab || isFish))
						yieldChange += 1;
				if (eYieldType == YIELD_GOLD && hasCommercialFishinCard && (isCrab || isFish))
							yieldChange += 2;				
			}

			{// CARD_MODERN_RESOURCES_U_BOAT_PEN - +5 PD +5 Diplo to Dry Docks
				const bool hasUboatCard = player.HasPolicy("POLICY_CARD_MODERN_RESOURCES_U_BOAT_PEN_PASSIVE");
				const bool isDryDock = plot.HasImprovement("IMPROVEMENT_DOCK");
				const bool isChileDryDock = plot.HasImprovement("IMPROVEMENT_CHILE_DOCK");
				if (eYieldType == YIELD_PRODUCTION && hasUboatCard && (isDryDock || isChileDryDock))
					yieldChange += 1;
				if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && hasUboatCard && (isDryDock || isChileDryDock))
					yieldChange += 1;
			}

			{// POLICY_LIBERTY_CLOSER_5 - +2 C to GP Tile Improvements
				const bool hasLibertyCloser5 = player.HasPolicy("POLICY_LIBERTY_CLOSER_5");				
				if (eYieldType == YIELD_CULTURE && hasLibertyCloser5 && isGreatTile)
					yieldChange += 2;				
			}

			{// POLICY_TRADITION_CLOSER_5 - +2 FD to GP Tile Improvements
				const bool hasTraditionCloser5 = player.HasPolicy("POLICY_TRADITION_CLOSER_5");
				if (eYieldType == YIELD_FOOD && hasTraditionCloser5 && isGreatTile)
					yieldChange += 2;
			}

			{// POLICY_COLLECTIVE_RULE - +2 C to Natural Wonders
				const bool hasCollectiveRule = player.HasPolicy("POLICY_COLLECTIVE_RULE");
				const bool isNaturalWonder = plot.HasAnyNaturalWonder();
				if (eYieldType == YIELD_CULTURE && hasCollectiveRule && isNaturalWonder)
					yieldChange += 2;
			}			

			{// POLICY_MILITARY_TRADITION - +1 PD to improved Iron, Stone, Hardwood
				const bool hasMilitaryTradition = player.HasPolicy("POLICY_MILITARY_TRADITION");				
				const bool isIron = plot.HasResource("RESOURCE_IRON");
				const bool isStone = plot.HasResource("RESOURCE_STONE");
				const bool isHardood = plot.HasResource("RESOURCE_HARDWOOD");
				const bool isMine = plot.HasImprovement("IMPROVEMENT_MINE");
				const bool isQuarry = plot.HasImprovement("IMPROVEMENT_QUARRY");
				const bool isLumbermill = plot.HasImprovement("IMPROVEMENT_LUMBERMILL");
				if (eYieldType == YIELD_PRODUCTION && hasMilitaryTradition && (isIron || isStone || isHardood) && (isMine || isQuarry || isLumbermill))
					yieldChange += 1;				
			}

			{// POLICY_DISCIPLINE - +3 PD +3C to Citadels
				const bool hasDiscipline = player.HasPolicy("POLICY_DISCIPLINE");
				const bool isCitadel = plot.HasImprovement("IMPROVEMENT_CITADEL");
				if (eYieldType == YIELD_PRODUCTION && hasDiscipline && isCitadel)
					yieldChange += 3;
				if (eYieldType == YIELD_CULTURE && hasDiscipline && isCitadel)
					yieldChange += 3;
			}

			{// POLICY_MANDATE_OF_HEAVEN - +1FH to Luxuries
				const bool hasMandateOfHeaven = player.HasPolicy("POLICY_MANDATE_OF_HEAVEN");				
				if (eYieldType == YIELD_FAITH && hasMandateOfHeaven && hasLuxury)
					yieldChange += 1;
			}

			{// POLICY_LANDED_ELITE - +1FD to improved Wheat, Maize, Bananas
				const bool hasLandedElite = player.HasPolicy("POLICY_LANDED_ELITE");
				const bool isFarm = plot.HasImprovement("IMPROVEMENT_FARM");
				const bool isPlantation = plot.HasImprovement("IMPROVEMENT_PLANTATION");
				const bool isWheat = plot.HasResource("RESOURCE_WHEAT");
				const bool isMaize = plot.HasResource("RESOURCE_MAIZE");
				const bool isBanana = plot.HasResource("RESOURCE_BANANA");
				if (eYieldType == YIELD_FOOD && hasLandedElite && (isFarm || isPlantation) && (isWheat || isMaize || isBanana))
					yieldChange += 1;				
			}

			{// POLICY_OLIGARCHY - +1FD to improved Deer, Bison
				const bool hasOligachy = player.HasPolicy("POLICY_OLIGARCHY");
				const bool isCamp = plot.HasImprovement("IMPROVEMENT_CAMP");				
				const bool isDeer = plot.HasResource("RESOURCE_DEER");
				const bool isBison = plot.HasResource("RESOURCE_BISON");				
				if (eYieldType == YIELD_FOOD && hasOligachy && isCamp && (isDeer || isBison))
					yieldChange += 1;
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

