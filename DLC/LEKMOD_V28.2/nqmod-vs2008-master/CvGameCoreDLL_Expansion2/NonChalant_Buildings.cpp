
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
	const BuildingTypes eBuilding,
	const BuildingClassTypes eBuildingClass,
	const CvBuildingEntry* pBuildingInfo,
	const YieldTypes eYieldType,
	const bool isPercentMod
) const
{
	int yieldChange = 0;

	const CvPlayer& player = *this;

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

	{// POLICY_FREE_THOUGHT - +1 Singularity from Research Labs
		const bool hasFreeThought = player.HasPolicy("POLICY_FREE_THOUGHT");
		const bool isResearchLab = eBuildingClass == BuildingClass("BUILDINGCLASS_LABORATORY");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFreeThought && isResearchLab)
			yieldChange += 1;
	}

	{// POLICY_FREE_THOUGHT - +1 Singularity from Research Labs
		const bool hasFreeThought = player.HasPolicy("POLICY_FREE_THOUGHT");
		const bool isResearchLab = eBuildingClass == BuildingClass("BUILDINGCLASS_LABORATORY");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFreeThought && isResearchLab)
			yieldChange += 1;
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

	{// POLICY_RATIONALISM_FINISHER - Rationalism Finisher gives 5 Scientific insight to the palace
		const bool hasRationalismFinisher = player.HasPolicy("POLICY_RATIONALISM_FINISHER");
		const bool isPalace = eBuildingClass == BuildingClass("BUILDINGCLASS_PALACE");
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasRationalismFinisher && isPalace)
			yieldChange += 5;
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

	return yieldChange;
}
bool CvPlayer::ShouldHaveBuilding(const CvPlayer& rPlayer, const CvCity& rCity, const bool isYourCapital, const bool isConquered, const bool isNewlyFounded, const BuildingClassTypes eBuildingClass)
{

	return false;
}
int CvPlayer::getSpecialistGpp(const CvCity* pCity, const SpecialistTypes eSpecialist, const SpecialistTypes eGppType, const bool isPercentMod) const
{
	const CvSpecialistInfo* pkSpecialist = GC.getSpecialistInfo(eSpecialist);
	float change = 0;
	if (pkSpecialist == NULL)
	{
		return change;
	}
	if (eSpecialist == eGppType)
	{
		// default yield
		change += pkSpecialist->getGreatPeopleRateChange();
	}

	const bool isUnemployed = eSpecialist == 0;
	const bool isWriter = eSpecialist == 1;
	const bool isArtist = eSpecialist == 2;
	const bool isMusician = eSpecialist == 3;
	const bool isScientist = eSpecialist == 4;
	const bool isMerchant = eSpecialist == 5;
	const bool isEngineer = eSpecialist == 6;
	const CvPlayer& player = *this;
	if (pCity != NULL)
	{
		const CvCity& city = *pCity;
		// logic that references city


	}


	// logic that does not reference the city


	return change;
}
int CvPlayer::getSpecialistYieldHardcoded(const CvCity* pCity, const SpecialistTypes eSpecialist, const YieldTypes eYield, const bool isPercent) const
{
	// <Table name="Specialists">
	float change = 0;
	const CvPlayer& player = *this;

	const bool isUnemployed = eSpecialist == 0;
	const bool isWriter = eSpecialist == 1;
	const bool isArtist = eSpecialist == 2;
	const bool isMusician = eSpecialist == 3;
	const bool isScientist = eSpecialist == 4;
	const bool isMerchant = eSpecialist == 5;
	const bool isEngineer = eSpecialist == 6;

	if (pCity != NULL)
	{
		const CvCity& city = *pCity;
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

	return GC.round(change);
}
int CvPlayer::getGreatWorkYieldTotal(const CvCity* pCity, const CvGreatWork* pWork, const YieldTypes eYield) const
{
	float change = 0;
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
		const CvCity& city = *pCity;
		// logic that references city


	}


	// logic that does not reference the city
	


	return GC.round(change);
}
