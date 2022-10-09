
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

BuildingClassTypes BuildingClass(const string name)
{
	return GC.GetGameBuildings()->BuildingClass(name);
}


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
	}

	{ // POLICY_SKYSCRAPERS - adds +2 diplomatic points to plazas
		const bool hasSkyScrapers = player.HasPolicy("POLICY_SKYSCRAPERS");
		const bool isPlaza = eBuildingClass == BuildingClass("BUILDINGCLASS_STATUE_5");
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasSkyScrapers && isPlaza)
			yieldChange += 1;
	}

	{// POLICY_FUTURISM - gives + 4 scientific insight from courthouse
		const bool hasFuturism = player.HasPolicy("POLICY_FUTURISM");
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFuturism && isCourthouse)
			yieldChange += 2;
	}

	{// POLICY_FUTURISM - gives + 3 scientific insight from courthouse
		const bool hasFuturism = player.HasPolicy("POLICY_FUTURISM");
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFuturism && isCourthouse)
			yieldChange += 3;
	}

	{// POLICY_UNITED_FRONT - gives + 10 diplo points from courthouse
		const bool hasUnitedFront = player.HasPolicy("POLICY_UNITED_FRONT");
		const bool isCourthouse = eBuildingClass == BuildingClass("BUILDINGCLASS_COURTHOUSE");
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasUnitedFront && isCourthouse)
			yieldChange += 10;
	}

	{// POLICY_TRADE_UNIONS - renamed Mercenary Army - gives + 15% gold to merchant's guilds
		const bool hasMercenaryArmy = player.HasPolicy("POLICY_TRADE_UNIONS");
		const bool isMerchantsGuild = eBuildingClass == BuildingClass("BUILDINGCLASS_GUILD_GOLD");
		if (eYieldType == YIELD_GOLD && isPercentMod && hasMercenaryArmy && isMerchantsGuild)
			yieldChange += 15;
	}	

	{// POLICY_URBANIZATION - +3% Production and Science to Windmill, Workshop, Factory
		const bool hasUrbanization = player.HasPolicy("POLICY_URBANIZATION");
		const bool isWorkshopOrWindmillOrFactory = eBuildingClass == BuildingClass("BUILDINGCLASS_WORKSHOP") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_WINDMILL") ||
			eBuildingClass == BuildingClass("BUILDINGCLASS_FACTORY");
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasUrbanization && isWorkshopOrWindmillOrFactory)
			yieldChange += 3;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasUrbanization && isWorkshopOrWindmillOrFactory)
			yieldChange += 3;
	}

	{// POLICY_UNIVERSAL_HEALTHCARE = -1 gold, +1 happy for granaries, -2 gold, +1 happy +1 food for aquaducts, -2 gold, -2 production, +1 happy +4 food from Hospitals
		const bool hasUniversal =
			player.HasPolicy("POLICY_UNIVERSAL_HEALTHCARE_F") ||
			player.HasPolicy("POLICY_UNIVERSAL_HEALTHCARE_O") ||
			player.HasPolicy("POLICY_UNIVERSAL_HEALTHCARE_A");
		const bool isGranary = eBuildingClass == BuildingClass("BUILDINGCLASS_GRANARY");
		const bool isAquaduct = eBuildingClass == BuildingClass("BUILDINGCLASS_AQUEDUCT");
		const bool isHospital = eBuildingClass == BuildingClass("BUILDINGCLASS_HOSPITAL");
		if (!isPercentMod)
		{
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isGranary)
				yieldChange += 1;

			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isAquaduct)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal && isAquaduct)
				yieldChange += 1;

			if (eYieldType == YIELD_MAINTENANCE && hasUniversal && isHospital)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal && isHospital)
				yieldChange += 4;
			if (eYieldType == YIELD_PRODUCTION && hasUniversal && isHospital)
				yieldChange -= 2;
		}
	}

	{// POLICY_SCHOLASTICISM - gives +5% Science to the Palace for each City-State Ally
		const bool hasScholasticism = player.HasPolicy("POLICY_SCHOLASTICISM");
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasScholasticism && isPalace)
			yieldChange += (numCityStateAllies * 5);
	}

	{// POLICY_CONSULATES - gives +3C to the Palace for each City-State Ally
		const bool hasConsulates = player.HasPolicy("POLICY_CONSULATES");
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		if (eYieldType == YIELD_SCIENCE && !isPercentMod && hasConsulates && isPalace)
			yieldChange += (numCityStateAllies * 3);
	}

	{// TIBET_STUPA // adds one of several yields every few techs
		const bool isStupa = eBuildingClass == BuildingClass("BUILDINGCLASS_TIBET");
		const bool hasEducation = player.HasTech("TECH_EDUCATION");
		const bool hasAcoustics = player.HasTech("TECH_ACOUSTICS");
		const bool hasIndustrialization = player.HasTech("TECH_INDUSTRIALIZATION");
		const bool hasRadio = player.HasTech("TECH_RADIO");
		const bool hasRadar = player.HasTech("TECH_RADAR");
		const bool hasGlobalization = player.HasTech("TECH_GLOBALIZATION");

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
		const bool isBroadcastTower =  eBuildingClass == BuildingClass("BUILDINGCLASS_EIFFEL_TOWER");
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
		const bool isGranry = eBuildingClass == BuildingClass("BUILDINGCLASS_GRANARY");
		const bool hasStatueOfLiberty = player.HasWonder(BuildingClass("BUILDINGCLASS_STATUE_OF_LIBERTY"));
		if (eYieldType == YIELD_FOOD && isPercentMod && hasStatueOfLiberty && isGranry)
			yieldChange += 10;
	}

	{// BUILDINGCLASS_KREMLIN grants +10% PD to every Workshop
		const bool isWorkshop = eBuildingClass == BuildingClass("BUILDINGCLASS_WORKSHOP");
		const bool hasKremlin = player.HasWonder(BuildingClass("BUILDINGCLASS_KREMLIN"));
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasKremlin && isWorkshop)
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
		const bool isPrussia = player.IsCiv("CIVILIZATION_PRUSSIA");
		if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isPrussia && (isWalls || isCastle || isArsenal || isMilitaryBase))
			yieldChange += 1;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && isPrussia && isPrussia && (isWalls || isCastle || isArsenal || isMilitaryBase))
			yieldChange += 1;		
	}
	

	return yieldChange;
}
bool CvPlayer::ShouldHaveBuilding(const CvPlayer& rPlayer, const CvCity& rCity, const bool isYourCapital, const bool, const bool, const BuildingClassTypes eBuildingClass)
{
	{// POLICY_MERCHANT_CONFEDERACY gives BUILDING_MERCHANT_CONFEDERACY_TRADE_ROUTE to Capital
		const bool isMerchantConfederacyBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_MERCHANT_CONFEDERACY_TRADE_ROUTE");	
		const bool hasMerchantConfederacy = rPlayer.HasPolicy("POLICY_MERCHANT_CONFEDERACY");
		if (isMerchantConfederacyBuilding && isYourCapital && hasMerchantConfederacy)
		
			return true;
	}

	{// POLICY_MERCHANT_CONFEDERACY gives BUILDING_MERCHANT_CONFEDERACY_TRADE_ROUTE to Capital
		const bool isMerchantConfederacyBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_MERCHANT_CONFEDERACY_TRADE_ROUTE");
		const bool hasMerchantConfederacy = rPlayer.HasPolicy("POLICY_MERCHANT_CONFEDERACY");
		if (isMerchantConfederacyBuilding && isYourCapital && hasMerchantConfederacy)

			return true;
	}

	{// POLICY_CARAVANS gives BUILDING_SILK_ROAD_TRADE_ROUTE to Capital
		const bool isSilkRoadBuilding = eBuildingClass == BuildingClass("BUILDING_SILK_ROAD_TRADE_ROUTE");
		const bool hasSilkRoad = rPlayer.HasPolicy("POLICY_CARAVANS");
		if (isSilkRoadBuilding && isYourCapital && hasSilkRoad)

			return true;
	}

	{// POLICY_FREE_THOUGHT gives BUILDING_FREE_THOUGHT_TRADE_ROUTE to Capital
		const bool isFreeThoughtBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_FREE_THOUGHT_TRADE_ROUTE");
		const bool hasFreeThought = rPlayer.HasPolicy("POLICY_FREE_THOUGHT");
		if (isFreeThoughtBuilding && isYourCapital && hasFreeThought)

			return true;
	}

	{// POLICY_RATIONALISM_FINISHER gives BUILDING_RATIONALISM_FINISHER_FREE_POP to Capital
		const bool isFreePopBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_RATIONALISM_FINISHER_FREE_POP");
		const bool hasRationalismFinisher = rPlayer.HasPolicy("POLICY_RATIONALISM_FINISHER");
		if (isFreePopBuilding && isYourCapital && hasRationalismFinisher)

			return true;
	}

	{// POLICY_PHILANTHROPY gives stuff for conquered City-States
		const bool isConqueredCityStateBuilding = eBuildingClass == BuildingClass("BUILDINGCLASS_CONQUERED_CITY_STATE"); 
		const bool isCaputuredCityState = rCity.IsOwnedMinorCapital();
		const bool hasPhilanthropy = rPlayer.HasPolicy("POLICY_PHILANTHROPY");
		if (isConqueredCityStateBuilding && isCaputuredCityState && hasPhilanthropy)

			return true;
	}
	

	return false;
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
	

	{// POLICY_TRADITION_FINISHER gives +1G +1PD to Engineer Specialists
		const bool hasTraditionFinisher = player.HasPolicy("POLICY_TRADITION_FINISHER");
		if (eYield == YIELD_GOLD && hasTraditionFinisher && isEngineer)
			change += 1;
		if (eYield == YIELD_PRODUCTION && hasTraditionFinisher && isEngineer)
			change += 1;
	}

	{// POLICY_ETHICS gives +1G +1C to Writer Specialists
		const bool hasCulturalExchange = player.HasPolicy("POLICY_ETHICS");
		if (eYield == YIELD_GOLD && hasCulturalExchange && isWriter)
			change += 1;
		if (eYield == YIELD_CULTURE && hasCulturalExchange && isWriter)
			change += 1;
	}

	{// POLICY_ARTISTIC_GENIUS gives +1SC +1C to Artist Specialists
		const bool hasArtisticGenius = player.HasPolicy("POLICY_ARTISTIC_GENIUS");
		if (eYield == YIELD_SCIENCE && hasArtisticGenius && isArtist)
			change += 1;
		if (eYield == YIELD_CULTURE && hasArtisticGenius && isArtist)
			change += 1;
	}

	{// POLICY_FINE_ARTS gives +1T +1G to Musician Specialists
		const bool hasFineArts = player.HasPolicy("POLICY_FINE_ARTS");
		if (eYield == YIELD_TOURISM && hasFineArts && isMusician)
			change += 1;
		if (eYield == YIELD_GOLD && hasFineArts && isMusician)
			change += 1;
	}

	{// POLICY_ENTREPRENEURSHIP gives +1PD +1G 1C to Merchant Specialists
		const bool hasEntreprenuership = player.HasPolicy("POLICY_ENTREPRENEURSHIP");
		if (eYield == YIELD_PRODUCTION && hasEntreprenuership && isMerchant)
			change += 1;
		if (eYield == YIELD_GOLD && hasEntreprenuership && isMerchant)
			change += 1;
		if (eYield == YIELD_CULTURE && hasEntreprenuership && isMerchant)
			change += 1;
	}

	{// POLICY_SECULARISM gives 1C to Scientist Specialists
		const bool hasSecularism = player.HasPolicy("POLICY_SECULARISM");			
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
		const bool hasCulturalExchange = player.HasPolicy("POLICY_ETHICS");
		if (eYield == YIELD_GOLD && hasCulturalExchange && isWriting)
			change += 1;
		if (eYield == YIELD_CULTURE && hasCulturalExchange && isWriting)
			change += 1;
	}

	{// POLICY_ARTISTIC_GENIUS gives +1SC +1C to GW of Art
		const bool hasArtisticGenius = player.HasPolicy("POLICY_ARTISTIC_GENIUS");
		if (eYield == YIELD_SCIENCE && hasArtisticGenius && isArt)
			change += 1;
		if (eYield == YIELD_CULTURE && hasArtisticGenius && isArt)
			change += 1;
	}

	{// POLICY_FINE_ARTS gives +1T +1G to GW of Music
		const bool hasFineArts = player.HasPolicy("POLICY_FINE_ARTS");
		if (eYield == YIELD_TOURISM && hasFineArts && isMusic)
			change += 1;
		if (eYield == YIELD_GOLD && hasFineArts && isMusic)
			change += 1;
	}

	{// POLICY_FLOURISHING_OF_ARTS gives +1T +1C to GW of Artifacts
		const bool hasFlourishingOfTheArts = player.HasPolicy("POLICY_FLOURISHING_OF_ARTS");
		if (eYield == YIELD_TOURISM && hasFlourishingOfTheArts && isArtifact)
			change += 1;
		if (eYield == YIELD_CULTURE && hasFlourishingOfTheArts && isArtifact)
			change += 1;
	}

	return change;
}
