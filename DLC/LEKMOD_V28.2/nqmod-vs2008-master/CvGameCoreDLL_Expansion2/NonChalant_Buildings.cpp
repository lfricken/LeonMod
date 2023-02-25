
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


// Extra yields for buildings.
int CvPlayer::GetExtraYieldForBuilding
(
	const CvCity* pCity,
	const BuildingTypes,
	const BuildingClassTypes eBuildingClass,
	const CvBuildingEntry*,
	const YieldTypes eYieldType,
	const bool isPercentMod
) const
{
	int yieldChange = 0;

	const CvPlayer& player = *this;

	const int numCityStateAllies = player.GetNumMinorAllies();

	if (pCity != NULL) // in a city
	{
		const CvCity& city = *pCity;
		const bool isCapital = city.isCapital();

		{ // BELIEF_PEACE_GARDENS - adds +1 scientific insight, 10% Science to national college wings
			const bool hasBeliefPeaceGardens = city.HasBelief("BELIEF_PEACE_GARDENZ");
			const bool isNationalCollege1 = eBuildingClass == BuildingClass("BUILDINGCLASS_NATIONAL_COLLEGE");
			const bool isNationalCollege2 = eBuildingClass == BuildingClass("BUILDINGCLASS_NATIONAL_SCIENCE_1");
			const bool isNationalCollege3 = eBuildingClass == BuildingClass("BUILDINGCLASS_NATIONAL_SCIENCE_2");
			if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasBeliefPeaceGardens && (isNationalCollege1 || isNationalCollege2 || isNationalCollege3))
				yieldChange += 1;
			if (eYieldType == YIELD_SCIENCE && isPercentMod && hasBeliefPeaceGardens && (isNationalCollege1 || isNationalCollege2 || isNationalCollege3))
				yieldChange += 10;
		}

		{// BELIEF_RELIGIOUS_FART +5% C, +10% Tourism
			const bool hasBeliefReligiousArt = city.HasBelief("BELIEF_RELIGIOUS_FART");
			const bool isHermitage = eBuildingClass == BuildingClass("BUILDINGCLASS_HERMITAGE");
			if (eYieldType == YIELD_CULTURE && isPercentMod && isHermitage && hasBeliefReligiousArt)
				yieldChange += 10;
			if (eYieldType == YIELD_TOURISM && isPercentMod && isHermitage && hasBeliefReligiousArt)
				yieldChange += 10;
		}

		{// BELIEFS that buff buildings purchased through faith
			const bool hasBeliefDharma = city.HasBelief("BELIEF_DARMA");
			const bool hasBeliefHajj = city.HasBelief("BELIEF_HAJJJJ");
			const bool hasBeliefJizya = city.HasBelief("BELIEF_CRAFTWORKS");
			const bool isReligiousBuilding =
				eBuildingClass == BuildingClass("BUILDINGCLASS_PAGODA") || eBuildingClass == BuildingClass("BUILDINGCLASS_MOSQUE")
				|| eBuildingClass == BuildingClass("BUILDINGCLASS_CATHEDRAL") || eBuildingClass == BuildingClass("BUILDINGCLASS_SCRIPTORIUM")
				|| eBuildingClass == BuildingClass("BUILDINGCLASS_TABERNACLE") || eBuildingClass == BuildingClass("BUILDINGCLASS_GURDWARA")
				|| eBuildingClass == BuildingClass("BUILDINGCLASS_SYNAGOGUE") || eBuildingClass == BuildingClass("BUILDINGCLASS_MITHRAEUM")
				|| eBuildingClass == BuildingClass("BUILDINGCLASS_VIHARA") || eBuildingClass == BuildingClass("BUILDINGCLASS_MANDIR");
			if (eYieldType == YIELD_GOLD && !isPercentMod && isReligiousBuilding && hasBeliefDharma)
				yieldChange += 3;
			if (eYieldType == YIELD_CULTURE && !isPercentMod && isReligiousBuilding && hasBeliefHajj)
				yieldChange += 1;
			if (eYieldType == YIELD_FAITH && !isPercentMod && isReligiousBuilding && hasBeliefJizya)
				yieldChange += 2;
		}

		{// BUILDINGCLASS_HOTEL +1 C, +1 Tourism and +2% C, +2% Tourism for every 5 citizens in a city.
			const bool isHotel = eBuildingClass == BuildingClass("BUILDINGCLASS_HOTEL");
			const int cityPopulation = city.getPopulation();
			if (eYieldType == YIELD_CULTURE && !isPercentMod && isHotel)
				yieldChange += (cityPopulation / 5);
			if (eYieldType == YIELD_TOURISM && !isPercentMod && isHotel)
				yieldChange += (cityPopulation / 5);
			if (eYieldType == YIELD_CULTURE && isPercentMod && isHotel)
				yieldChange += (2 * (cityPopulation / 5));
			if (eYieldType == YIELD_TOURISM && isPercentMod && isHotel)
				yieldChange += (2 * (cityPopulation / 5));
		}
		{// BUILDINGCLASS_BROADCAST_TOWER +2 C, Tourism and +2% C, Tourism for every World and National Wonder.
			const bool isBroadcastTower = eBuildingClass == BuildingClass("BUILDINGCLASS_BROADCAST_TOWER");
			const int numWorldWondersInCity = city.getNumWorldWonders();
			const int numNationalWondersInCity = city.getNumNationalWonders();
			if (eYieldType == YIELD_CULTURE && !isPercentMod && isBroadcastTower)
				yieldChange += (2 * numWorldWondersInCity);
			if (eYieldType == YIELD_CULTURE && !isPercentMod && isBroadcastTower)
				yieldChange += (2 * numNationalWondersInCity);
			if (eYieldType == YIELD_TOURISM && !isPercentMod && isBroadcastTower)
				yieldChange += (2 * numWorldWondersInCity);
			if (eYieldType == YIELD_TOURISM && !isPercentMod && isBroadcastTower)
				yieldChange += (2 * numNationalWondersInCity);
			if (eYieldType == YIELD_CULTURE && isPercentMod && isBroadcastTower)
				yieldChange += (2 * numWorldWondersInCity);
			if (eYieldType == YIELD_CULTURE && isPercentMod && isBroadcastTower)
				yieldChange += (2 * numNationalWondersInCity);
			if (eYieldType == YIELD_TOURISM && isPercentMod && isBroadcastTower)
				yieldChange += (2 * numWorldWondersInCity);
			if (eYieldType == YIELD_TOURISM && isPercentMod && isBroadcastTower)
				yieldChange += (2 * numNationalWondersInCity);
		}

		{// POLICY_TRADITION_CLOSER - 1FD Capital.
			int numTraditionClosers = player.HasPolicy("POLICY_TRADITION_CLOSER_1") || player.HasPolicy("POLICY_TRADITION_CLOSER_2") ||
				player.HasPolicy("POLICY_TRADITION_CLOSER_3") || player.HasPolicy("POLICY_TRADITION_CLOSER_4") ||
				player.HasPolicy("POLICY_TRADITION_CLOSER_5") || player.HasPolicy("POLICY_TRADITION_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");			
			if (eYieldType == YIELD_FOOD && !isPercentMod && isCityCenterBuilding && isCapital)
				yieldChange += numTraditionClosers;
		}

		{// POLICY_LIBERTY_CLOSER - 1C Capital.
			int numLibertyClosers = player.HasPolicy("POLICY_LIBERTY_CLOSER_1") || player.HasPolicy("POLICY_LIBERTY_CLOSER_2") ||
				player.HasPolicy("POLICY_LIBERTY_CLOSER_3") || player.HasPolicy("POLICY_LIBERTY_CLOSER_4") ||
				player.HasPolicy("POLICY_LIBERTY_CLOSER_5") || player.HasPolicy("POLICY_LIBERTY_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");			
			if (eYieldType == YIELD_CULTURE && !isPercentMod && isCityCenterBuilding && isCapital)
				yieldChange += numLibertyClosers;
		}

		{// POLICY_HONOR_CLOSER - +2%PD all Cities. 1PD Capital.
			int numHonorClosers = player.HasPolicy("POLICY_HONOR_CLOSER_1") || player.HasPolicy("POLICY_HONOR_CLOSER_2") ||
				player.HasPolicy("POLICY_HONOR_CLOSER_3") || player.HasPolicy("POLICY_HONOR_CLOSER_4") ||
				player.HasPolicy("POLICY_HONOR_CLOSER_5") || player.HasPolicy("POLICY_HONOR_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			if (eYieldType == YIELD_PRODUCTION && isPercentMod && isCityCenterBuilding)
				yieldChange += numHonorClosers * 2;
			if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isCityCenterBuilding && isCapital)
				yieldChange += numHonorClosers;
		}

		{// POLICY_PIETY_CLOSER - +1FH Capital.
			int numPietyClosers = player.HasPolicy("POLICY_PIETY_CLOSER_1") || player.HasPolicy("POLICY_PIETY_CLOSER_2") ||
				player.HasPolicy("POLICY_PIETY_CLOSER_3") || player.HasPolicy("POLICY_PIETY_FINISHER") ||
				player.HasPolicy("POLICY_PIETY_CLOSER_5") || player.HasPolicy("POLICY_PIETY_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");			
			if (eYieldType == YIELD_FAITH && !isPercentMod && isCityCenterBuilding && isCapital)
				yieldChange += numPietyClosers;
		}

		{// POLICY_AESTHETICS_CLOSER - +1T Capital.
			int numAestheticsClosers = player.HasPolicy("POLICY_AESTHETICS_CLOSER_1") || player.HasPolicy("POLICY_AESTHETICS_CLOSER_2") ||
				player.HasPolicy("POLICY_AESTHETICS_CLOSER_3") || player.HasPolicy("POLICY_AESTHETICS_CLOSER_4") ||
				player.HasPolicy("POLICY_AESTHETICS_CLOSER_5") || player.HasPolicy("POLICY_AESTHETICS_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			if (eYieldType == YIELD_TOURISM && !isPercentMod && isCityCenterBuilding && isCapital)
				yieldChange += numAestheticsClosers;
		}

		{// POLICY_AESTHETICS_CLOSER_3 - +2 Golden Age Point World and National Wonder Capital. 
			const bool hasAestheticsCloser = player.HasPolicy("POLICY_AESTHETICS_CLOSER_3");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			int numWonders = (player.GetNumWonders() + player.getNumNationalWonders());
			if (eYieldType == YIELD_GOLDEN && !isPercentMod && isCityCenterBuilding && hasAestheticsCloser && isCapital)
				yieldChange += (numWonders * 2);
		}

		{// POLICY_LEGALISM - +2 Golden Age Point per per Policy Capital.
			const bool hasLegalism = player.HasPolicy(POLICY_LEGALISM);
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			int numPolicies = player.GetNumPoliciesTotal();
			if (eYieldType == YIELD_GOLDEN && !isPercentMod && isCityCenterBuilding && hasLegalism && isCapital)
				yieldChange += (numPolicies * 2);
		}

		{// POLICY_COMMERCE_CLOSER - +5%G all Cities
			int numCommerceClosers = player.HasPolicy("POLICY_COMMERCE_CLOSER_1") || player.HasPolicy("POLICY_COMMERCE_CLOSER_1") ||
				player.HasPolicy("POLICY_COMMERCE_CLOSER_1") || player.HasPolicy("POLICY_COMMERCE_CLOSER_1") ||
				player.HasPolicy("POLICY_COMMERCE_CLOSER_1") || player.HasPolicy("POLICY_COMMERCE_CLOSER_1");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			if (eYieldType == YIELD_GOLD && isPercentMod && isCityCenterBuilding)
				yieldChange += numCommerceClosers * 5;			
		}

		{// POLICY_RATIONALISM_CLOSER - +4%SC all Cities
			int numRationalismClosers = player.HasPolicy("POLICY_RATIONALISM_CLOSER_1") || player.HasPolicy("POLICY_RATIONALISM_CLOSER_2") ||
				player.HasPolicy("POLICY_RATIONALISM_CLOSER_3") || player.HasPolicy("POLICY_RATIONALISM_FINISHER") ||
				player.HasPolicy("POLICY_RATIONALISM_CLOSER_5") || player.HasPolicy("POLICY_RATIONALISM_CLOSER_6");
			const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
			if (eYieldType == YIELD_SCIENCE && isPercentMod && isCityCenterBuilding)
				yieldChange += numRationalismClosers * 4;
		}

	}

	{ // POLICY_SKYSCRAPERS - adds +2 diplomatic points to plazas
		const bool hasSkyScrapers = player.HasPolicy(POLICY_SKYSCRAPERS);
		const bool isPlaza = eBuildingClass == BuildingClass("BUILDINGCLASS_STATUE_5");
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasSkyScrapers && isPlaza)
			yieldChange += 1;
	}

	{// POLICY_FUTURISM - gives + 4 scientific insight from courthouse
		const bool hasFuturism = player.HasPolicy(POLICY_FUTURISM);
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFuturism && isCourthouse)
			yieldChange += 2;
	}

	{// POLICY_FUTURISM - gives + 3 scientific insight from courthouse
		const bool hasFuturism = player.HasPolicy(POLICY_FUTURISM);
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFuturism && isCourthouse)
			yieldChange += 2;
	}

	{// POLICY_UNITED_FRONT - gives + 10 diplo points from courthouse
		const bool hasUnitedFront = player.HasPolicy(POLICY_UNITED_FRONT);
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasUnitedFront && isCourthouse)
			yieldChange += 10;
	}

	{// POLICY_TRADE_UNIONS - renamed Mercenary Army - gives + 15% gold to Caravansarys
		const bool hasMercenaryArmy = player.HasPolicy(POLICY_TRADE_UNIONS);
		const bool isMerchantsGuild = eBuildingClass == BuildingClass("BUILDINGCLASS_GUILD_GOLD");
		if (eYieldType == YIELD_GOLD && isPercentMod && hasMercenaryArmy && isMerchantsGuild)
			yieldChange += 15;
	}

	{// POLICY_URBANIZATION - +3% Production and Science to Windmill, Workshop, Factory
		const bool hasUrbanization = player.HasPolicy(POLICY_URBANIZATION);
		const bool isProductionBuilding = 
			eBuildingClass == BuildingClass("BUILDINGCLASS_WORKSHOP") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_WINDMILL") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_FORGE") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_SHIPYARD") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_TEXTILE") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_FACTORY") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_STEEL_MILL") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_REFINERY") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_HYDRO_PLANT") || 
			eBuildingClass == BuildingClass("BUILDINGCLASS_REFINERY") || 
			eBuildingClass == BuildingClass("BUILDINGCLASS_SOLAR_PLANT") || 
			eBuildingClass == BuildingClass("BUILDINGCLASS_NUCLEAR_PLANT");
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasUrbanization && isProductionBuilding)
			yieldChange += 3;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasUrbanization && isProductionBuilding)
			yieldChange += 3;
	}

	{// POLICY_UNIVERSAL_HEALTHCARE = -1 gold, +1 happy for granaries, -2 gold, +1 happy +1 food for aquaducts, -2 gold, -2 production, +1 happy +4 food from Hospitals
		const bool hasUniversal =
			player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_F) ||
			player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_O) ||
			player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_A);
		const bool isLevel1FoodBuilding = 
			eBuildingClass == BuildingClass("BUILDINGCLASS_GRANARY") || eBuildingClass == BuildingClass("BUILDINGCLASS_COOKING_HEARTH") || 
			eBuildingClass == BuildingClass("BUILDINGCLASS_FISHERY") || eBuildingClass == BuildingClass("BUILDINGCLASS_HUNTERS_HAVEN");
		const bool isLevel2FoodBuilding = 
			eBuildingClass == BuildingClass("BUILDINGCLASS_AQUEDUCT") || eBuildingClass == BuildingClass("BUILDINGCLASS_WATERMILL") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_GROCER");
		const bool isLevel3FoodBuilding = 
			eBuildingClass == BuildingClass("BUILDINGCLASS_HOSPITAL") || eBuildingClass == BuildingClass("BUILDINGCLASS_STOCKYARDS") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_GRAIN_SILO");
		if (!isPercentMod)
		{
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isLevel1FoodBuilding)
				yieldChange += 1;
			if (eYieldType == YIELD_FOOD && hasUniversal && isLevel1FoodBuilding)
				yieldChange += 1;
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isLevel2FoodBuilding)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal && isLevel2FoodBuilding)
				yieldChange += 2;
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isLevel3FoodBuilding)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal && isLevel3FoodBuilding)
				yieldChange += 3;
			if (eYieldType == YIELD_PRODUCTION && hasUniversal && isLevel3FoodBuilding)
				yieldChange -= 2;
		}
	}

	{// POLICY_SCHOLASTICISM - gives +5% Science to the Palace for each City-State Ally
		const bool hasScholasticism = player.HasPolicy(POLICY_SCHOLASTICISM);
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasScholasticism && isPalace)
			yieldChange += (numCityStateAllies * 5);
	}

	{// TIBET_STUPA // adds one of several yields every few techs
		const bool isStupa = eBuildingClass == BuildingClass("BUILDINGCLASS_TIBET");
		const bool hasEducation = player.HasTech(TECH_EDUCATION);
		const bool hasAcoustics = player.HasTech(TECH_ACOUSTICS);
		const bool hasIndustrialization = player.HasTech(TECH_INDUSTRIALIZATION);
		const bool hasRadio = player.HasTech(TECH_RADIO);
		const bool hasRadar = player.HasTech(TECH_RADAR);
		const bool hasGlobalization = player.HasTech(TECH_GLOBALIZATION);

		const int numTechBoosters = hasEducation + hasAcoustics + hasIndustrialization + hasRadio + hasRadar + hasGlobalization;
		const bool isYieldBoosted = eYieldType == YIELD_CULTURE || eYieldType == YIELD_SCIENCE || eYieldType == YIELD_PRODUCTION || eYieldType == YIELD_FOOD
			|| eYieldType == YIELD_GOLD || eYieldType == YIELD_FAITH;
		if (isStupa && isYieldBoosted && !isPercentMod)
			yieldChange += numTechBoosters;
	}

	{// Building_Recycling Center gets +1 Scientific Insight
		const bool isRecyclingCenter = eBuildingClass == BuildingClass("BUILDINGCLASS_RECYCLING_CENTER");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && isRecyclingCenter)
			yieldChange += 2;
	}

	{// BUILDING_CONQUERED_CITY_STATE Center gets +10% FD, SC, C 
		const bool isConqueredCityStateBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CONQUERED_CITY_STATE");
		if (eYieldType == YIELD_FOOD && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
		if (eYieldType == YIELD_CULTURE && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
	}

	{// BUILDING_EIFFEL_TOWER grants +2T to every broadcast tower 
		const bool isBroadcastTower = eBuildingClass == BuildingClass("BUILDINGCLASS_EIFFEL_TOWER");
		const bool hasEiffelTower = player.HasWonder(BuildingClass("BUILDINGCLASS_EIFFEL_TOWER"));
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isBroadcastTower && hasEiffelTower)
			yieldChange += 2;
	}

	{// BUILDING_BROADWAY grants +2T to every Hotel 
		const bool isHotel = eBuildingClass == BuildingClass("BUILDINGCLASS_EIFFEL_TOWER");
		const bool hasBroadway = player.HasWonder(BuildingClass("BUILDINGCLASS_HOTEL"));
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isHotel && hasBroadway)
			yieldChange += 2;
	}

	{// BUILDING_CRISTO_REDENTOR grants +2T to every Plaza
		const bool isPlaza = eBuildingClass == BuildingClass("BUILDINGCLASS_STATUE_5");
		const bool hasCristo = player.HasWonder(BuildingClass("BUILDINGCLASS_CRISTO_REDENTOR"));
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isPlaza && hasCristo)
			yieldChange += 2;
	}

	{// BUILDING_SYDNEY_OPERA_HOUSE grants +3T to every Airport
		const bool isAirport = eBuildingClass == BuildingClass("BUILDINGCLASS_AIRPORT");
		const bool hasSydney = player.HasWonder(BuildingClass("BUILDINGCLASS_SYDNEY_OPERA_HOUSE"));
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isAirport && hasSydney)
			yieldChange += 3;
	}

	{// BUILDINGCLASS_STATUE_OF_LIBERTY grants +10% FD to every Granary
		const bool isAqueduct = eBuildingClass == BuildingClass("BUILDINGCLASS_AQUEDUCT");
		const bool isWatermill = eBuildingClass == BuildingClass("BUILDINGCLASS_WATERMILL");
		const bool isGrocer = eBuildingClass == BuildingClass("BUILDINGCLASS_GROCER");
		const bool hasStatueOfLiberty = player.HasWonder(BuildingClass("BUILDINGCLASS_STATUE_OF_LIBERTY"));
		if (eYieldType == YIELD_FOOD && isPercentMod && hasStatueOfLiberty && (isAqueduct || isWatermill || isGrocer))
			yieldChange += 10;
	}

	{// BUILDINGCLASS_KREMLIN grants +10% PD to every Workshop
		const bool isWorkshop = eBuildingClass == BuildingClass("BUILDINGCLASS_WORKSHOP");
		const bool isWindmill = eBuildingClass == BuildingClass("BUILDINGCLASS_WINDMILL");
		const bool isForge = eBuildingClass == BuildingClass("BUILDINGCLASS_FORGE");
		const bool hasKremlin = player.HasWonder(BuildingClass("BUILDINGCLASS_KREMLIN"));
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasKremlin && (isWorkshop || isWindmill  || isForge))
			yieldChange += 10;
	}

	{// BUILDINGCLASS_FORBIDDEN_PALACE grants +1FD +1PD +1G +1Diplo from every City-State Ally
		const bool isForbiddenPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_FORBIDDEN_PALACE");
		if (eYieldType == YIELD_FOOD && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_GOLD && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
	}

	{// BUILDINGCLASS_WALLS grants +1C +1PD to Walls, Castles, Arsenals, and Military Bases to Prussia
		const bool isWalls = eBuildingClass == BuildingClass("BUILDINGCLASS_WALLS");
		const bool isCastle = eBuildingClass == BuildingClass("BUILDINGCLASS_CASTLE");
		const bool isArsenal = eBuildingClass == BuildingClass("BUILDINGCLASS_ARSENAL");
		const bool isMilitaryBase = eBuildingClass == BuildingClass("BUILDINGCLASS_MILITARY_BASE");
		const bool isPrussia = player.IsCiv(CIVILIZATION_PRUSSIA);
		if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isPrussia && (isWalls || isCastle || isArsenal || isMilitaryBase))
			yieldChange += 1;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && isPrussia && isPrussia && (isWalls || isCastle || isArsenal || isMilitaryBase))
			yieldChange += 1;
	}

	{// CARD_ANCIENT_BUILDINGS_DRUIDS_PASSIVE grants +1C +1PD to Walls, Castles, Arsenals, and Military Bases to Prussia
		const bool hasDruidsCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_DRUIDS_PASSIVE);
		const bool isShrine = eBuildingClass == BuildingClass("BUILDINGCLASS_SHRINE");		
		if (eYieldType == YIELD_FAITH && !isPercentMod && hasDruidsCard && isShrine)
			yieldChange += 2;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasDruidsCard && isShrine)
			yieldChange -= 1;		
	}

	{// CARD_ANCIENT_BUILDINGS_HARBORMASTER_PASSIVE grants -2 Maintenance to Harbors
		const bool hasHarbormasterCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARBORMASTER_PASSIVE);
		const bool isHarbor = eBuildingClass == BuildingClass("BUILDINGCLASS_HARBOR");		
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasHarbormasterCard && isHarbor)
			yieldChange -= 2;
	}

	{// CARD_CLASSICAL_BUILDINGS_GLADIATOR_GAMES_PASSIVE grants +2 Maintenance to Colloseums
		const bool hasGladitorGamesCard = player.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_GLADIATOR_GAMES_PASSIVE);
		const bool isColloseum = eBuildingClass == BuildingClass("BUILDINGCLASS_COLOSSEUM");
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasGladitorGamesCard && isColloseum)
			yieldChange += 2;
	}

	{// CARD_RENAISSANCE_BUILDINGS_DOMINANCE gives +1C +1Diplo to palace for every 2 military units
		const bool hasDominanceCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_DOMINANCE_PASSIVE);
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		int numUnits = player.getNumMilitaryUnits();

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasDominanceCard && isPalace)
			yieldChange += numUnits / 2;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasDominanceCard && isPalace)
			yieldChange += numUnits / 2;
	}

	{// CARD_RENAISSANCE_BUILDINGS_EXPRESSIONALISM gives +2C +2Diplo to palace for every GW
		const bool hasExpressionalismCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_EXPRESSIONALISM_PASSIVE);
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		int numGreatWorks = player.GetNumSpecialistGreatWorks();

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasExpressionalismCard && isPalace)
			yieldChange += numGreatWorks * 2;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasExpressionalismCard && isPalace)
			yieldChange += numGreatWorks * 2;
	}

	{// CARD_RENAISSANCE_BUILDINGS_GRANDEUR gives +1C +1 Diplo to palace for every Wonder
		const bool hasGrandeurCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_GRANDEUR_PASSIVE);
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");		
		int numWonders = player.GetNumWonders(); 

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasGrandeurCard && isPalace)
			yieldChange += numWonders;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasGrandeurCard && isPalace)
			yieldChange += numWonders; 
	}

	{// CARD_RENAISSANCE_BUILDINGS_MEDICI_BANK gives +5%GD +1C  for each Bank
		const bool hasMediciCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_MEDICI_BANK_PASSIVE);
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");		
		int numBanks = player.countNumBuildingClasses(BuildingClassTypes(40));

		if (eYieldType == YIELD_GOLD && isPercentMod && hasMediciCard && isPalace)
			yieldChange += numBanks * 5;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasMediciCard && isPalace)
			yieldChange += numBanks;
	}
	

	{// CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY If 5 Observeratories, +1 Insight
		const bool hasCopernicusCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY_PASSIVE);
		const bool isObserveratory = eBuildingClass == BuildingClass("BUILDINGCLASS_OBSERVATORY");
		int numObserveratories = player.countNumBuildingClasses(BuildingClassTypes(10));
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && (numObserveratories >= 5) && hasCopernicusCard && isObserveratory)
			yieldChange += 1;
	}

	{// CARD_INDUSTRIAL_BUILDINGS_OFFICER_TRAINING - +1/2 insight to military academies
		const bool hasOfficerCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_OFFICER_TRAINING_PASSIVE);
		// const bool isMilitaryAcademy = eBuildingClass == BuildingClass("BUILDINGCLASS_MILITARY_ACADEMY");
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		int numMilitaryAcademies = player.countNumBuildingClasses(BuildingClassTypes(27));
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasOfficerCard && isPalace)
			yieldChange += numMilitaryAcademies / 2;
	}

	{// CARD_INDUSTRIAL_BUILDINGS_WALLSTREET - +25% G 5% Pd stock exchanges
		const bool hasWallStreetCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_WALLSTREET_PASSIVE);
		const bool isStockExchange = eBuildingClass == BuildingClass("BUILDINGCLASS_STOCK_EXCHANGE");
		if (eYieldType == YIELD_GOLD && isPercentMod && hasWallStreetCard && isStockExchange)
			yieldChange += 25;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasWallStreetCard && isStockExchange)
			yieldChange += 5;
	}	

	{// CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY - +5% PD Textile Mills
		const bool hasEliWhitneyCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY_PASSIVE);
		const bool isTextileMill = eBuildingClass == BuildingClass("BUILDINGCLASS_TEXTILE");
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasEliWhitneyCard && isTextileMill)
			yieldChange += 5;
	}

	{// CARD_INDUSTRIAL_BUILDINGS_IMPRESSIONALISM - +2C +2T to Mueseums
		const bool hasImpressionalismCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_IMPRESSIONALISM_PASSIVE);
		const bool isMuseum = eBuildingClass == BuildingClass("BUILDINGCLASS_MUSEUM");
		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasImpressionalismCard && isMuseum)
			yieldChange += 2;
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasImpressionalismCard && isMuseum)
			yieldChange += 2;
	}

	{// CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE - +2T to Broadcast Towers
		const bool hasThomsDoolittleCard = player.HasPolicy(POLICY_CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE_PASSIVE);
		const bool isBroadcastTower = eBuildingClass == BuildingClass("BUILDINGCLASS_BROADCAST_TOWER");		
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasThomsDoolittleCard && isBroadcastTower)
			yieldChange += 2;
	}

	{// CARD_MODERN_BUILDINGS_WORLD_CUP - +2T to Stadiums
		const bool hasWorldCupCard = player.HasPolicy(POLICY_CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE_PASSIVE);
		const bool isStadium = eBuildingClass == BuildingClass("BUILDINGCLASS_BROADCAST_TOWER");
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasWorldCupCard && isStadium)
			yieldChange += 2;
	}	

	{// CARD_MODERN_BUILDINGS_FIRESIDE_CHATS - +2T to Broadcast Towers
		const bool hasFiresideChatsCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_FIRESIDE_CHATS_PASSIVE);
		const bool isBroadcastTower = eBuildingClass == BuildingClass("BUILDINGCLASS_BROADCAST_TOWER");
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasFiresideChatsCard && isBroadcastTower)
			yieldChange += 2;
	}

	{// CARD_MODERN_BUILDINGS_NEW_DEHLI - +5T to Grand Monument
		const bool hasNewDehliCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_NEW_DEHLI_PASSIVE);
		const bool isGrandMonument = eBuildingClass == BuildingClass("BUILDINGCLASS_GRAND_MONUMENT");
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasNewDehliCard && isGrandMonument)
			yieldChange += 5;
	}

	{// CARD_MODERN_BUILDINGS_ANESTHESIA - +1/2 insight to Medical Labs
		const bool hasAnesthesiaCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_ANESTHESIA_PASSIVE);		
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		int numMedicalLabs = player.countNumBuildingClasses(BuildingClassTypes(34));
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasAnesthesiaCard && isPalace)
			yieldChange += numMedicalLabs / 2;
	}


	return yieldChange;
}
BuildingAddType CvPlayer::ShouldHaveBuilding(const CvPlayer& rPlayer, const CvCity& rCity, const bool isYourCapital, const bool, const bool, const BuildingClassTypes eBuildingClass)
{
	// WARNING!! WARNINNG!! It is important that there ARE NOT multiple buildings with the same BuildingClass
	// Otherwise the code that calls this may not know to remove the existing type since the passed type resolves to a different building
	// this could be fixed by passing the explicit building type to this tag as well.

	{// POLICY_MERCHANT_CONFEDERACY gives BUILDING_MERCHANT_CONFEDERACY_TRADE_ROUTE to Capital
		const bool isMerchantConfederacyBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_MERCHANT_CONFEDERACY_TRADE_ROUTE");
		if (isMerchantConfederacyBuilding)
		{
			const bool hasMerchantConfederacy = rPlayer.HasPolicy(POLICY_MERCHANT_CONFEDERACY);
			if (isYourCapital && hasMerchantConfederacy)
				return ADD;
			else
				return REMOVE;
		}
	}
	{// POLICY_FREE_THOUGHT gives BUILDING_FREE_THOUGHT_TRADE_ROUTE to Capital
		const bool isFreeThoughtBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_FREE_THOUGHT_TRADE_ROUTE");
		if (isFreeThoughtBuilding)
		{
			const bool hasFreeThought = rPlayer.HasPolicy(POLICY_FREE_THOUGHT);
			if (isYourCapital && hasFreeThought)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_RATIONALISM_FINISHER gives BUILDING_RATIONALISM_FINISHER_FREE_POP to Capital
		const bool isFreePopBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_RATIONALISM_FINISHER_FREE_POP");
		if (isFreePopBuilding)
		{
			const bool hasRationalismFinisher3 = rPlayer.HasPolicy("POLICY_RATIONALISM_CLOSER_3");
			if (isYourCapital && hasRationalismFinisher3)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_PHILANTHROPY gives stuff for conquered City-States
		const bool isConqueredCityStateBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CONQUERED_CITY_STATE");
		if (isConqueredCityStateBuilding)
		{
			const bool isCapturedCityState = rCity.IsOwnedMinorCapital();
			const bool hasPhilanthropy = rPlayer.HasPolicy(POLICY_PHILANTHROPY);
			if (isCapturedCityState && hasPhilanthropy)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CIVILIZATION_RUSSIA gets free Jarliq in Capital upon discovery of Philosophy
		const bool isJarliq = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (isJarliq)
		{
			const bool isRussia = rPlayer.IsCiv(CIVILIZATION_RUSSIA);
			const bool hasPhilosophy = rPlayer.HasTech(TECH_PHILOSOPHY);
			if (isRussia && hasPhilosophy)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// BUILDING_CARD_SHIPS_OF_THE_DESERT goes in cities with Caravansary and POLICY_CARD_ANCIENT_BUILDINGS_SHIPS_OF_THE_DESERT_PASSIVE
		const bool isShipsBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_SHIPS_OF_THE_DESERT");
		if (isShipsBuilding)
		{
			const bool hasCaravansary = rCity.HasBuildingClass(BuildingClassTypes(129));
			const bool hasShipsPolicy = rPlayer.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_SHIPS_OF_THE_DESERT_PASSIVE);
			if (hasCaravansary && hasShipsPolicy)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// BUILDING_CARD_ANCIENT_BUILDINGS_FORCED_LEVY goes in cities with Caravansary and POLICY_CARD_ANCIENT_BUILDINGS_FORCED_LEVY_PASSIVE
		const bool isForcedLevyBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_ANCIENT_BUILDINGS_FORCED_LEVY");
		if (isForcedLevyBuilding)
		{
			const bool hasBarracks = rCity.HasBuildingClass(BuildingClassTypes(25));
			const bool hasForcedLevyPolicy = rPlayer.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_FORCED_LEVY_PASSIVE);
			if (hasBarracks && hasForcedLevyPolicy)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// BUILDING_CARD_ANCIENT_BUILDINGS_FORCED_LEVY goes in cities with Caravansary and POLICY_CARD_ANCIENT_BUILDINGS_FORCED_LEVY_PASSIVE
		const bool iswalls = eBuildingClass == BuildingClass("BUILDINGCLASS_WALLS");
		if (iswalls)
		{
			const bool hasProtectiveCard = rPlayer.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_PROTECTIVE_PASSIVE);			
			if (hasProtectiveCard)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// BUILDING_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE goes in cities with Lighthouse and POLICY_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE
		const bool isBeaconsOfHopeBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE");
		if (isBeaconsOfHopeBuilding)
		{
			const bool hasBeaconsOfHopeCard = rPlayer.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE_PASSIVE);
			const bool hasLighthouse = rCity.HasBuildingClass(BuildingClassTypes(13));
			if (hasBeaconsOfHopeCard && hasLighthouse)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CARD_CLASSICAL_BUILDINGS_CHAMPION_OF_THE_PEOPLE free statue if barracks
		const bool isStatue = eBuildingClass == BuildingClass("BUILDINGCLASS_STATUE_1");
		if (isStatue)
		{
			const bool hasChampionCard = rPlayer.HasPolicy(POLICY_CARD_CLASSICAL_LEGENDARY_CHAMPION_PASSIVE);
			const bool hasBarracks = rCity.HasBuildingClass(BuildingClassTypes(25));
			if (hasChampionCard && hasBarracks)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CARD_CLASSICAL_BUILDINGS_FLUTE_AND_LYRE free Musicians Courtyard if Fuontain and Garden
		const bool isMusiciansCourtyard = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_CLASSICAL_BUILDINGS_FLUTE_AND_LYRE");
		if (isMusiciansCourtyard)
		{
			const bool hasFluteCard = rPlayer.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_FLUTE_AND_LYRE_PASSIVE);
			const bool hasGarden = rCity.HasBuildingClass(BuildingClassTypes(12));
			const bool hasFountain = rCity.HasBuildingClass(BuildingClassTypes(232));
			if (hasFluteCard && hasGarden && hasFountain)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CARD_MEDIEVAL_BUILDINGS_CAMBRIDGE_UNIVERSITY free Cambridge University if Oxford University + Wings
		const bool isCambridgeUniversity = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_MEDIEVAL_BUILDINGS_CAMBRIDGE_UNIVERSITY");
		if (isCambridgeUniversity)
		{
			const bool hasCambrigeCard = rPlayer.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_CAMBRIDGE_UNIVERSITY_PASSIVE);
			const bool hasOxford1 = rCity.HasBuildingClass(BuildingClassTypes(53));
			const bool hasOxford2 = rCity.HasBuildingClass(BuildingClassTypes(229));
			const bool hasOxford3 = rCity.HasBuildingClass(BuildingClassTypes(230));
			if (hasCambrigeCard && hasOxford1 && hasOxford2 && hasOxford3)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CARD_MEDIEVAL_BUILDINGS_FEALTY free Fealty Building in Capital
		const bool isFealtyBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_MEDIEVAL_BUILDINGS_FEALTY");
		if (isFealtyBuilding)
		{
			const bool hasFealtyCard = rPlayer.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_FEALTY_ACTIVE);			
			if (hasFealtyCard && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// CARD_INDUSTRIAL_BUILDINGS_PENICILLIN free Pennicilin Building in Capital
		const bool isPennicilinBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CARD_INDUSTRIAL_BUILDINGS_PENICILLIN");
		if (isPennicilinBuilding)
		{
			const bool hasPennicilinCard = rPlayer.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_PENICILLIN_PASSIVE);
			if (hasPennicilinCard && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// BUILDING_CITY_CENTER goes in every city.
		const bool isCityCenterBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CITY_CENTER");
		if (isCityCenterBuilding)		
				return ADD;			
	}	

	{// POLICY_AESTHETICS_CLOSER_5 free BUILDING_AESTHETICS_CLOSER_5 in Capital
		const bool isAesthetics5Building = eBuildingClass == BuildingClass("BUILDINGCLASS_AESTHETICS_CLOSER_5");
		if (isAesthetics5Building)
		{
			const bool hasAesthetics5 = rPlayer.HasPolicy("POLICY_AESTHETICS_CLOSER_5");
			if (hasAesthetics5 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_LEGALISM free BUILDINGCLASS_POLICY_LEGALISM_GP_GENERATION in Capital
		const bool isLegalismBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_POLICY_LEGALISM_GP_GENERATION");
		if (isLegalismBuilding)
		{
			const bool hasLegalism = rPlayer.HasPolicy(POLICY_LEGALISM);
			if (hasLegalism && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_EXPLORATION_CLOSER_2 free in Capital
		const bool isExplorationBuilding2 = eBuildingClass == BuildingClass("BUILDINGCLASS_POLICY_EXPLORATION_CLOSER_2");
		if (isExplorationBuilding2)
		{
			const bool hasExploration2 = rPlayer.HasPolicy("POLICY_EXPLORATION_CLOSER_2");
			if (hasExploration2 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_EXPLORATION_CLOSER_5 free in Capital
		const bool isExplorationBuilding5 = eBuildingClass == BuildingClass("BUILDINGCLASS_POLICY_EXPLORATION_CLOSER_5");
		if (isExplorationBuilding5)
		{
			const bool hasExploration5 = rPlayer.HasPolicy("POLICY_EXPLORATION_CLOSER_5");
			if (hasExploration5 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	return INDIFFERENT;
}
int CvPlayer::getSpecialistGpp(const CvCity* pCity, const SpecialistTypes eSpecialist, const SpecialistTypes eGppType, const bool) const
{
	const CvSpecialistInfo* pkSpecialist = GC.getSpecialistInfo(eSpecialist);
	int change = 0;
	if (pkSpecialist == NULL)
	{
		return change;
	}
	if (eSpecialist == eGppType)
	{
		// default yield
		change += pkSpecialist->getGreatPeopleRateChange();
	}

	//const bool isUnemployed = eSpecialist == 0;
	//const bool isWriter = eSpecialist == 1;
	//const bool isArtist = eSpecialist == 2;
	//const bool isMusician = eSpecialist == 3;
	//const bool isScientist = eSpecialist == 4;
	//const bool isMerchant = eSpecialist == 5;
	//const bool isEngineer = eSpecialist == 6;
	//const CvPlayer& player = *this;
	if (pCity != NULL)
	{
		//const CvCity& city = *pCity;
		// logic that references city


	}


	// logic that does not reference the city


	return change;
}
int CvPlayer::getSpecialistYieldHardcoded(const CvCity* pCity, const SpecialistTypes eSpecialist, const YieldTypes eYield, const bool) const
{
	// <Table name="Specialists">
	int change = 0;
	const CvPlayer& player = *this;

	//const bool isUnemployed = eSpecialist == 0;
	const bool isWriter = eSpecialist == 1;
	const bool isArtist = eSpecialist == 2;
	const bool isMusician = eSpecialist == 3;
	const bool isScientist = eSpecialist == 4;
	const bool isMerchant = eSpecialist == 5;
	const bool isEngineer = eSpecialist == 6;

	if (pCity != NULL)
	{
		//const CvCity& city = *pCity;
		// logic that references city


	}


	// logic that does not reference the city



	{// POLICY_TRADE_UNIONS gives +1G +1PD to Engineer Specialists
		const bool hasTradeUnions = player.HasPolicy(POLICY_TRADE_UNIONS);
		if (eYield == YIELD_GOLD && hasTradeUnions && isEngineer)
			change += 1;
		if (eYield == YIELD_PRODUCTION && hasTradeUnions && isEngineer)
			change += 1;
	}

	{// POLICY_ETHICS gives +1G +1C to Writer Specialists
		const bool hasCulturalExchange = player.HasPolicy(POLICY_ETHICS);
		if (eYield == YIELD_GOLD && hasCulturalExchange && isWriter)
			change += 1;
		if (eYield == YIELD_CULTURE && hasCulturalExchange && isWriter)
			change += 1;
	}

	{// POLICY_ARTISTIC_GENIUS gives +1SC +1C to Artist Specialists
		const bool hasArtisticGenius = player.HasPolicy(POLICY_ARTISTIC_GENIUS);
		if (eYield == YIELD_SCIENCE && hasArtisticGenius && isArtist)
			change += 1;
		if (eYield == YIELD_CULTURE && hasArtisticGenius && isArtist)
			change += 1;
	}

	{// POLICY_FINE_ARTS gives +1T +1G to Musician Specialists
		const bool hasFineArts = player.HasPolicy(POLICY_FINE_ARTS);
		if (eYield == YIELD_TOURISM && hasFineArts && isMusician)
			change += 1;
		if (eYield == YIELD_GOLD && hasFineArts && isMusician)
			change += 1;
	}

	{// POLICY_ENTREPRENEURSHIP gives +1PD +1G 1C to Merchant Specialists
		const bool hasEntreprenuership = player.HasPolicy(POLICY_ENTREPRENEURSHIP);
		if (eYield == YIELD_GOLD && hasEntreprenuership && isMerchant)
			change += 1;
		if (eYield == YIELD_CULTURE && hasEntreprenuership && isMerchant)
			change += 1;
	}

	{// POLICY_SECULARISM gives 1C to Scientist Specialists
		const bool hasSecularism = player.HasPolicy(POLICY_SECULARISM);
		if (eYield == YIELD_CULTURE && hasSecularism && isScientist)
			change += 1;
	}

	{// Globe Theater gives 1C, +1T  to Writer Specialists
		const bool hasGlobeTheater = player.HasWonder(BuildingClass("BUILDINGCLASS_GLOBE_THEATER"));
		if (eYield == YIELD_TOURISM && hasGlobeTheater && isWriter)
			change += 1;
		if (eYield == YIELD_CULTURE && hasGlobeTheater && isWriter)
			change += 1;
	}

	{// Sistine Chapel gives +1C, +1T  to Artist Specialists
		const bool hasSistineChapel = player.HasWonder(BuildingClass("BUILDINGCLASS_SISTINE_CHAPEL"));
		if (eYield == YIELD_CULTURE && hasSistineChapel && isArtist)
			change += 1;
		if (eYield == YIELD_TOURISM && hasSistineChapel && isArtist)
			change += 1;
	}

	{// Uffizi gives +1C, +1T  to Music Specialists
		const bool hasUffizi = player.HasWonder(BuildingClass("BUILDINGCLASS_UFFIZI"));
		if (eYield == YIELD_CULTURE && hasUffizi && isMusician)
			change += 1;
		if (eYield == YIELD_TOURISM && hasUffizi && isMusician)
			change += 1;
	}

	{// BUILDINGCLASS_BIG_BEN gives +1G +1PD to Merchants
		const bool hasBigBen = player.HasWonder(BuildingClass("BUILDINGCLASS_BIG_BEN"));
		if (eYield == YIELD_PRODUCTION && hasBigBen && isMerchant)
			change += 1;
		if (eYield == YIELD_GOLD && hasBigBen && isMerchant)
			change += 1;
	}

	{// BUILDINGCLASS_PANAMA gives +1G +1PD to Engineers
		const bool hasPanama = player.HasWonder(BuildingClass("BUILDINGCLASS_PANAMA"));
		if (eYield == YIELD_PRODUCTION && hasPanama && isEngineer)
			change += 1;
		if (eYield == YIELD_GOLD && hasPanama && isEngineer)
			change += 1;
	}

	{// BUILDINGCLASS_PORCELAIN_TOWER gives +1SC +1PD to Scientists
		const bool hasPorcelainTower = player.HasWonder(BuildingClass("BUILDINGCLASS_PORCELAIN_TOWER"));
		if (eYield == YIELD_PRODUCTION && hasPorcelainTower && isScientist)
			change += 1;
		if (eYield == YIELD_SCIENCE && hasPorcelainTower && isScientist)
			change += 1;
	}

	{// CARD_CLASSICAL_BUILDINGS_CANNON_OF_TEN gives +1T to Writer, Artist and Musician Specialists
		const bool hasCannonCard = player.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_CANNON_OF_TEN_PASSIVE);
		if (eYield == YIELD_TOURISM && hasCannonCard && (isMusician || isWriter || isArtist))
			change += 1;		
	}

	{// CARD_RENAISSANCE_BUILDINGS_WILLIAM_SHAKSPEARE gives +1C, +1T to Writer if you have Globe Theater
		const bool hasWilliamShakespeareCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_WILLIAM_SHAKSPEARE_PASSIVE);
		const bool hasGlobeTheater = player.HasWonder(BuildingClass("BUILDINGCLASS_GLOBE_THEATER"));
		if (eYield == YIELD_TOURISM && hasWilliamShakespeareCard && hasGlobeTheater && isWriter)
			change += 1;
		if (eYield == YIELD_CULTURE && hasWilliamShakespeareCard && hasGlobeTheater && isWriter)
			change += 1;
	}
	{// CARD_RENAISSANCE_BUILDINGS_MICHAEL_ANGELO gives +1C, +1T to Artist if you have Sistine Chapel
		const bool hasMichaelAngeloCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_MICHAEL_ANGELO_PASSIVE);
		const bool hasSistineChapel = player.HasWonder(BuildingClass("BUILDINGCLASS_SISTINE_CHAPEL"));
		if (eYield == YIELD_TOURISM && hasMichaelAngeloCard && hasSistineChapel && isArtist)
			change += 1;
		if (eYield == YIELD_CULTURE && hasMichaelAngeloCard && hasSistineChapel && isArtist)
			change += 1;
	}
	{// CARD_RENAISSANCE_BUILDINGS_J_S_BACH gives +1C, +1T to Musician if you have Teatro Alla Scala
		const bool hasBachCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_J_S_BACH_PASSIVE);
		const bool hasTeatroAllaScala = player.HasWonder(BuildingClass("BUILDINGCLASS_UFFIZI"));
		if (eYield == YIELD_TOURISM && hasBachCard && hasTeatroAllaScala && isMusician)
			change += 1;
		if (eYield == YIELD_CULTURE && hasBachCard && hasTeatroAllaScala && isMusician)
			change += 1;
	}
	{// CARD_RENAISSANCE_BUILDINGS_DOUBLE_ENTRY_ACCOUNTING gives +2G Merchant. +1PD +1Diplo if Big Ben
		const bool hasDoubleEntrCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_DOUBLE_ENTRY_ACCOUNTING_PASSIVE);
		const bool hasBigBen = player.HasWonder(BuildingClass("BUILDINGCLASS_BIG_BEN"));
		if (eYield == YIELD_GOLD && hasDoubleEntrCard && isMerchant)
			change += 2;
		if (eYield == YIELD_PRODUCTION && hasDoubleEntrCard && hasBigBen && isMerchant)
			change += 1;
		if (eYield == YIELD_DIPLOMATIC_SUPPORT && hasDoubleEntrCard && hasBigBen && isMerchant)
			change += 1;
	}

	{// CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY gives +2SC to Scientist.
		const bool hasCopernicusCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY_PASSIVE);				
		if (eYield == YIELD_SCIENCE && hasCopernicusCard && isScientist)
			change += 2;		
	}

	return change;
}
int CvPlayer::getGreatWorkYieldTotal(const CvCity* pCity, const CvGreatWork* pWork, const YieldTypes eYield) const
{
	int change = 0;
	const CvPlayer& player = *this;

	{ // base
		if (eYield == YIELD_CULTURE)
			change += GC.getBASE_CULTURE_PER_GREAT_WORK();
		if (eYield == YIELD_TOURISM)
		{
			change += GC.getBASE_TOURISM_PER_GREAT_WORK();
			change += player.GetPlayerPolicies()->GetNumericModifier(POLICYMOD_EXTRA_TOURISM_PER_GREAT_WORK);
		}

		change += player.GetGreatWorkYieldChange(eYield);
	}
	const CvGreatWork& work = *pWork;
	const bool isArt = work.m_eClassType == 1;
	const bool isArtifact = work.m_eClassType == 2;
	const bool isWriting = work.m_eClassType == 3;
	const bool isMusic = work.m_eClassType == 4;

	if (pCity != NULL)
	{
		//const CvCity& city = *pCity;
		// logic that references city


	}


	// logic that does not reference the city

	{// Globe Theater gives +2C, +2T  to GW Writing
		const bool hasGlobeTheater = player.HasWonder(BuildingClass("BUILDINGCLASS_GLOBE_THEATER"));
		if (eYield == YIELD_CULTURE && hasGlobeTheater && isWriting)
			change += 2;
		if (eYield == YIELD_TOURISM && hasGlobeTheater && isWriting)
			change += 2;
	}

	{// Sistine Chapel gives +2C, +2T  to GW Art
		const bool hasSistineChapel = player.HasWonder(BuildingClass("BUILDINGCLASS_SISTINE_CHAPEL"));
		if (eYield == YIELD_CULTURE && hasSistineChapel && isArt)
			change += 2;
		if (eYield == YIELD_TOURISM && hasSistineChapel && isArt)
			change += 2;
	}

	{// Uffizi gives +2C, +2T  to GW Music
		const bool hasUffizi = player.HasWonder(BuildingClass("BUILDINGCLASS_UFFIZI"));
		if (eYield == YIELD_CULTURE && hasUffizi && isMusic)
			change += 2;
		if (eYield == YIELD_TOURISM && hasUffizi && isMusic)
			change += 2;
	}

	{// Louvre gives +2C, +2T  to Artifacts
		const bool hasLourve = player.HasWonder(BuildingClass("BUILDINGCLASS_LOUVRE"));
		if (eYield == YIELD_CULTURE && hasLourve && isArtifact)
			change += 2;
		if (eYield == YIELD_TOURISM && hasLourve && isArtifact)
			change += 2;
	}

	{// POLICY_ETHICS gives +1G +1C to GW of Writing
		const bool hasCulturalExchange = player.HasPolicy(POLICY_ETHICS);
		if (eYield == YIELD_GOLD && hasCulturalExchange && isWriting)
			change += 1;
		if (eYield == YIELD_CULTURE && hasCulturalExchange && isWriting)
			change += 1;
	}

	{// POLICY_ARTISTIC_GENIUS gives +1SC +1C to GW of Art
		const bool hasArtisticGenius = player.HasPolicy(POLICY_ARTISTIC_GENIUS);
		if (eYield == YIELD_SCIENCE && hasArtisticGenius && isArt)
			change += 1;
		if (eYield == YIELD_CULTURE && hasArtisticGenius && isArt)
			change += 1;
	}

	{// POLICY_FINE_ARTS gives +1T +1G to GW of Music
		const bool hasFineArts = player.HasPolicy(POLICY_FINE_ARTS);
		if (eYield == YIELD_TOURISM && hasFineArts && isMusic)
			change += 1;
		if (eYield == YIELD_GOLD && hasFineArts && isMusic)
			change += 1;
	}

	{// POLICY_FLOURISHING_OF_ARTS gives +1T +1C to GW of Artifacts
		const bool hasFlourishingOfTheArts = player.HasPolicy(POLICY_FLOURISHING_OF_ARTS);
		if (eYield == YIELD_TOURISM && hasFlourishingOfTheArts && isArtifact)
			change += 1;
		if (eYield == YIELD_CULTURE && hasFlourishingOfTheArts && isArtifact)
			change += 1;
	}

	{// CARD_INDUSTRIAL_BUILDINGS_HOWARD_CARTER gives +1C +1T to Artifacts.
		const bool hasHowardCarterCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_HOWARD_CARTER_PASSIVE);
		const bool hasLourve = player.HasWonder(BuildingClass("BUILDINGCLASS_LOUVRE"));
		if (eYield == YIELD_CULTURE && hasHowardCarterCard && isArtifact)
			change += 1;
		if (eYield == YIELD_TOURISM && hasHowardCarterCard && isArtifact)
			change += 1;
		if (eYield == YIELD_CULTURE && hasHowardCarterCard && isArtifact && hasLourve)
			change += 1;
		if (eYield == YIELD_TOURISM && hasHowardCarterCard && isArtifact && hasLourve)
			change += 1;
	}

	return change;
}
