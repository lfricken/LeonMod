
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

		switch (eBuildingClass)
		{
		case BUILDINGCLASS_NATIONAL_COLLEGE:
		case BUILDINGCLASS_NATIONAL_SCIENCE_1:
		case BUILDINGCLASS_NATIONAL_SCIENCE_2:
		{ // BELIEF_PEACE_GARDENS - adds +1 scientific insight, 10% Science to national college wings
			const bool hasBeliefPeaceGardens = city.HasBelief(BELIEF_PEACE_GARDENZ);
			if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasBeliefPeaceGardens)
				yieldChange += 1;
			if (eYieldType == YIELD_SCIENCE && isPercentMod && hasBeliefPeaceGardens)
				yieldChange += 10;

			break;
		}
		case BUILDINGCLASS_HERMITAGE:
		{// BELIEF_RELIGIOUS_FART +5% C, +10% Tourism
			const bool hasBeliefReligiousArt = city.HasBelief(BELIEF_RELIGIOUS_FART);
			if (eYieldType == YIELD_CULTURE && isPercentMod && hasBeliefReligiousArt)
				yieldChange += 10;
			if (eYieldType == YIELD_TOURISM && isPercentMod && hasBeliefReligiousArt)
				yieldChange += 10;

			break;
		}
		case BUILDINGCLASS_PAGODA:
		case BUILDINGCLASS_MOSQUE:
		case BUILDINGCLASS_CATHEDRAL:
		case BUILDINGCLASS_TABERNACLE:
		case BUILDINGCLASS_GURDWARA:
		case BUILDINGCLASS_SYNAGOGUE:
		case BUILDINGCLASS_VIHARA:
		case BUILDINGCLASS_MANDIR:
		{// BELIEFS that buff buildings purchased through faith
			const bool hasBeliefDharma = city.HasBelief(BELIEF_DARMA);
			const bool hasBeliefHajj = city.HasBelief(BELIEF_HAJJJJ);
			const bool hasBeliefJizya = city.HasBelief(BELIEF_CRAFTWORKS);

			if (eYieldType == YIELD_GOLD && !isPercentMod && hasBeliefDharma)
				yieldChange += 2;
			if (eYieldType == YIELD_CULTURE && !isPercentMod && hasBeliefHajj)
				yieldChange += 1;
			if (eYieldType == YIELD_FAITH && !isPercentMod && hasBeliefJizya)
				yieldChange += 2;

			break;
		}
		case BUILDINGCLASS_HOTEL:
		{// BUILDINGCLASS_HOTEL +1 C, +1 Tourism and +2% C, +2% Tourism for every 5 citizens in a city.
			const int cityPopulation = city.getPopulation();
			if (eYieldType == YIELD_CULTURE && !isPercentMod)
				yieldChange += (cityPopulation / 5);
			if (eYieldType == YIELD_TOURISM && !isPercentMod)
				yieldChange += (cityPopulation / 5);
			if (eYieldType == YIELD_CULTURE && isPercentMod)
				yieldChange += (2 * (cityPopulation / 5));
			if (eYieldType == YIELD_TOURISM && isPercentMod)
				yieldChange += (2 * (cityPopulation / 5));

			break;
		}
		case BUILDINGCLASS_BROADCAST_TOWER:
		{// BUILDINGCLASS_BROADCAST_TOWER +2 C, Tourism and +2% C, Tourism for every World and National Wonder.
			const bool isBroadcastTower = eBuildingClass == BUILDINGCLASS_BROADCAST_TOWER;
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

			break;
		}
		case BUILDINGCLASS_CITY_CENTER:
		{
			{// POLICY_TRADITION_CLOSER - 1FD Capital.
				int numTraditionClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_TRADITION);
				if (eYieldType == YIELD_FOOD && !isPercentMod && isCapital)
					yieldChange += numTraditionClosers;
			}

			{// POLICY_LIBERTY_CLOSER - 1C Capital.
				int numLibertyClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_LIBERTY);
				if (eYieldType == YIELD_CULTURE && !isPercentMod && isCapital)
					yieldChange += numLibertyClosers;
			}

			{// POLICY_HONOR_CLOSER - +2%PD all Cities. 1PD Capital.
				int numHonorClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_HONOR);
				if (eYieldType == YIELD_PRODUCTION && isPercentMod)
					yieldChange += numHonorClosers * 2;
				if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isCapital)
					yieldChange += numHonorClosers;
			}

			{// POLICY_PIETY_CLOSER - +1FH Capital.
				int numPietyClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_PIETY);
				if (eYieldType == YIELD_FAITH && !isPercentMod && isCapital)
					yieldChange += numPietyClosers;
			}

			{// POLICY_AESTHETICS_CLOSER - +1T Capital.
				int numAestheticsClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_AESTHETICS);
				if (eYieldType == YIELD_TOURISM && !isPercentMod && isCapital)
					yieldChange += numAestheticsClosers;
			}

			{// POLICY_AESTHETICS_CLOSER_3 - +2 Golden Age Point World and National Wonder Capital. 
				const bool hasAestheticsCloser = player.HasPolicy(POLICY_AESTHETICS_CLOSER_3);
				int numWonders = (player.GetNumWonders() + player.getNumNationalWonders());
				if (eYieldType == YIELD_GOLDEN && !isPercentMod && hasAestheticsCloser && isCapital)
					yieldChange += (numWonders * 2);
			}

			{// POLICY_LEGALISM - +2 Golden Age Point per per Policy Capital.
				const bool hasLegalism = player.HasPolicy(POLICY_LEGALISM);
				int numPolicies = player.GetNumPoliciesTotal();
				if (eYieldType == YIELD_GOLDEN && !isPercentMod && hasLegalism && isCapital)
					yieldChange += (numPolicies * 2);
			}

			{// CIVILIZATION_ROME - +2 Golden Age Point per per Policy Capital.
				const bool isRome = player.IsCiv(CIVILIZATION_ROME);
				int numPolicies = player.GetNumPoliciesTotal();
				if (eYieldType == YIELD_GOLDEN && !isPercentMod && isRome && isCapital)
					yieldChange += (numPolicies * 2);
			}

			{// POLICY_COMMERCE_CLOSER - +5%G all Cities
				int numCommerceClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_COMMERCE);
				if (eYieldType == YIELD_GOLD && isPercentMod)
					yieldChange += numCommerceClosers * 5;
			}

			{// POLICY_RATIONALISM_CLOSER - +4%SC all Cities
				int numRationalismClosers = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(POLICY_BRANCH_RATIONALISM);
				if (eYieldType == YIELD_SCIENCE && isPercentMod)
					yieldChange += numRationalismClosers * 4;
			}

			break;
		}
		case BUILDINGCLASS_MOSQUE_OF_DJENNE:
		{// MOSQUE_OF_DJENNE - +1C +1FH from City-States Following
			const ReligionTypes majorityReligion = city.GetCityReligions()->GetReligiousMajority();
			const int numCityStatesFollowing = GC.getGame().GetGameReligions()->GetNumCitiesFollowing(majorityReligion, true);
			if (eYieldType == YIELD_CULTURE && !isPercentMod)
				yieldChange += numCityStatesFollowing * 2;
			if (eYieldType == YIELD_FAITH && !isPercentMod)
				yieldChange += numCityStatesFollowing * 2;

			break;
		}
		default:
			break;
		}
	}
	switch (eBuildingClass)
	{
	case BUILDINGCLASS_STATUE_5:
	{ // POLICY_SKYSCRAPERS - adds +2 diplomatic points to plazas
		const bool hasSkyScrapers = player.HasPolicy(POLICY_SKYSCRAPERS);
		const bool isPlaza = eBuildingClass == BUILDINGCLASS_STATUE_5;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasSkyScrapers && isPlaza)
			yieldChange += 1;

		{// BUILDING_CRISTO_REDENTOR grants +2T to every Plaza
			const bool isPlaza = eBuildingClass == BUILDINGCLASS_STATUE_5;
			const bool hasCristo = player.HasWonder(BUILDINGCLASS_CRISTO_REDENTOR);
			if (eYieldType == YIELD_TOURISM && !isPercentMod && isPlaza && hasCristo)
				yieldChange += 2;
		}
	}
	break;
	case BUILDINGCLASS_COURTHOUSE:
	{// POLICY_FUTURISM - gives + 4 scientific insight from courthouse
		const bool hasFuturism = player.HasPolicy(POLICY_FUTURISM);
		const bool isCourthouse = eBuildingClass == BUILDINGCLASS_COURTHOUSE;
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasFuturism && isCourthouse)
			yieldChange += 2;

		break;
	}
	case BUILDINGCLASS_TEXTILE:
	{// CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY - +5% PD Textile Mills
		const bool hasEliWhitneyCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_ELI_WHITNEY_PASSIVE);
		const bool isTextileMill = eBuildingClass == BUILDINGCLASS_TEXTILE;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasEliWhitneyCard && isTextileMill)
			yieldChange += 5;
	}
	// DO NOT Break
	case BUILDINGCLASS_WORKSHOP:
	case BUILDINGCLASS_WINDMILL:
	{// POLICY_CARD_MEDIEVAL_BUILDINGS_RHINE_RIVER_DELTA_PASSIVE - 
		const bool hasRhineCard = player.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_RHINE_RIVER_DELTA_PASSIVE);
		const bool isWindmill = eBuildingClass == BUILDINGCLASS_WINDMILL;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasRhineCard && isWindmill)
			yieldChange += 3;
	}
	// DO NOT break
	case BUILDINGCLASS_FORGE:

	case BUILDINGCLASS_SHIPYARD:
		//case BUILDINGCLASS_TEXTILE: handled above
	case BUILDINGCLASS_FACTORY:

	case BUILDINGCLASS_STEEL_MILL:
	case BUILDINGCLASS_REFINERY:
	case BUILDINGCLASS_HYDRO_PLANT:

	case BUILDINGCLASS_SOLAR_PLANT:
	case BUILDINGCLASS_NUCLEAR_PLANT:
	{// POLICY_URBANIZATION - +3% Production and Science to Windmill, Workshop, Factory
		const bool hasUrbanization = player.HasPolicy(POLICY_URBANIZATION);
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasUrbanization)
			yieldChange += 3;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasUrbanization)
			yieldChange += 3;

	}
	break;
	case BUILDINGCLASS_GRANARY:
	case BUILDINGCLASS_COOKING_HEARTH:
	case BUILDINGCLASS_FISHERY:
	case BUILDINGCLASS_HUNTERS_HAVEN:
	{// POLICY_UNIVERSAL_HEALTHCARE = -1 gold, +1 happy for granaries, -2 gold, +1 happy +1 food for aquaducts, -2 gold, -2 production, +1 happy +4 food from Hospitals
		if (!isPercentMod)
		{
			const bool hasUniversal =
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_F) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_O) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_A);
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal)
				yieldChange += 1;
			if (eYieldType == YIELD_FOOD && hasUniversal)
				yieldChange += 1;
		}
	}
	{// POLICY_CARD_ANCIENT_BUILDINGS_SOLARIZATION_PASSIVE - 
		const bool hasSolarizationCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_SOLARIZATION_PASSIVE);
		const bool isGranary = eBuildingClass == BUILDINGCLASS_GRANARY;
		if (eYieldType == YIELD_FOOD && !isPercentMod && hasSolarizationCard && isGranary)
			yieldChange += 1;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasSolarizationCard && isGranary)
			yieldChange -= 1;
	}
	{// POLICY_CARD_ANCIENT_BUILDINGS_SNARE_PASSIVE 
		const bool hasSnareCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_SNARE_PASSIVE);
		const bool isHuntersHaven = eBuildingClass == BUILDINGCLASS_HUNTERS_HAVEN;
		if (eYieldType == YIELD_FOOD && !isPercentMod && hasSnareCard && isHuntersHaven)
			yieldChange += 1;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasSnareCard && isHuntersHaven)
			yieldChange -= 1;
	}
	{// POLICY_CARD_ANCIENT_BUILDINGS_HARPOON_PASSIVE - 
		const bool hasHarpoonCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARPOON_PASSIVE);
		const bool isFishery = eBuildingClass == BUILDINGCLASS_FISHERY;
		if (eYieldType == YIELD_FOOD && !isPercentMod && hasHarpoonCard && isFishery)
			yieldChange += 1;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasHarpoonCard && isFishery)
			yieldChange -= 1;
	}
	break;
	case BUILDINGCLASS_AQUEDUCT:
	case BUILDINGCLASS_WATERMILL:
	case BUILDINGCLASS_GROCER:
	{// POLICY_UNIVERSAL_HEALTHCARE = -1 gold, +1 happy for granaries, -2 gold, +1 happy +1 food for aquaducts, -2 gold, -2 production, +1 happy +4 food from Hospitals
		if (!isPercentMod)
		{
			const bool hasUniversal =
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_F) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_O) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_A);
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal)
				yieldChange += 2;
		}
	}
	break;
	case BUILDINGCLASS_HOSPITAL:
	case BUILDINGCLASS_STOCKYARDS:
	case BUILDINGCLASS_GRAIN_ELEVATOR:
	{// POLICY_UNIVERSAL_HEALTHCARE = -1 gold, +1 happy for granaries, -2 gold, +1 happy +1 food for aquaducts, -2 gold, -2 production, +1 happy +4 food from Hospitals
		if (!isPercentMod)
		{
			const bool hasUniversal =
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_F) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_O) ||
				player.HasPolicy(POLICY_UNIVERSAL_HEALTHCARE_A);
			if (eYieldType == YIELD_MAINTENANCE && hasUniversal)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && hasUniversal)
				yieldChange += 3;
			if (eYieldType == YIELD_PRODUCTION && hasUniversal)
				yieldChange -= 2;
		}
		{// POLICY_CARD_INDUSTRIAL_BUILDINGS_UNION_STOCKYARDS_PASSIVE - 
			const bool hasUnionStockyarsCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_UNION_STOCKYARDS_PASSIVE);
			const bool isStockyard = eBuildingClass == BUILDINGCLASS_STOCKYARDS;
			if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasUnionStockyarsCard && isStockyard)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && !isPercentMod && hasUnionStockyarsCard && isStockyard)
				yieldChange += 2;
			if (eYieldType == YIELD_PRODUCTION && !isPercentMod && hasUnionStockyarsCard && isStockyard)
				yieldChange += 2;
		}
		{// POLICY_CARD_INDUSTRIAL_BUILDINGS_DARTS_ELEVATOR_PASSIVE - 
			const bool hasDartsElevatorCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_DARTS_ELEVATOR_PASSIVE);
			const bool isGrainElevator = eBuildingClass == BUILDINGCLASS_GRAIN_ELEVATOR;
			if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasDartsElevatorCard && isGrainElevator)
				yieldChange += 2;
			if (eYieldType == YIELD_FOOD && !isPercentMod && hasDartsElevatorCard && isGrainElevator)
				yieldChange += 2;
			if (eYieldType == YIELD_PRODUCTION && !isPercentMod && hasDartsElevatorCard && isGrainElevator)
				yieldChange += 2;
		}

		{// POLICY_CARD_ATOMIC_BUILDINGS_CLEAN_AIR_ACT_PASSIVE - 
			const bool hasCleanAirActCard = player.HasPolicy(POLICY_CARD_ATOMIC_BUILDINGS_CLEAN_AIR_ACT_PASSIVE);
			const bool isSolarPlant = eBuildingClass == BUILDINGCLASS_SOLAR_PLANT;
			if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasCleanAirActCard && isSolarPlant)
				yieldChange += 3;
		}
		break;
	}
	case BUILDINGCLASS_CITY_CENTER:
	{
		break;
	}
	case BUILDINGCLASS_PALACE:
	{// POLICY_SCHOLASTICISM - gives +5% Science to the Palace for each City-State Ally
		const bool hasScholasticism = player.HasPolicy(POLICY_SCHOLASTICISM);
		if (eYieldType == YIELD_SCIENCE && isPercentMod && hasScholasticism)
			yieldChange += (numCityStateAllies * 5);

	}
	{// CARD_RENAISSANCE_BUILDINGS_DOMINANCE gives +1C +1Diplo to palace for every 2 military units
		const bool hasDominanceCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_DOMINANCE_PASSIVE);
		int numUnits = player.getNumMilitaryUnits();

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasDominanceCard)
			yieldChange += numUnits / 3;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasDominanceCard)
			yieldChange += numUnits / 3;
	}
	{// CARD_RENAISSANCE_BUILDINGS_EXPRESSIONALISM gives +2C +2Diplo to palace for every GW
		const bool hasExpressionalismCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_EXPRESSIONALISM_PASSIVE);
		int numGreatWorks = player.GetNumSpecialistGreatWorks();

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasExpressionalismCard)
			yieldChange += numGreatWorks;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasExpressionalismCard)
			yieldChange += numGreatWorks;
	}

	{// CARD_RENAISSANCE_BUILDINGS_GRANDEUR gives +1C +1 Diplo to palace for every Wonder
		const bool hasGrandeurCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_GRANDEUR_PASSIVE);
		int numWonders = player.GetNumWonders();

		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasGrandeurCard)
			yieldChange += numWonders;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && hasGrandeurCard)
			yieldChange += numWonders;
	}

	{// CARD_RENAISSANCE_BUILDINGS_MEDICI_BANK gives 
		const bool hasMediciCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_MEDICI_BANK_PASSIVE);
		int numBanks = player.countNumBuildingClasses(BUILDINGCLASS_BANK);

		if (eYieldType == YIELD_GOLD && isPercentMod && hasMediciCard)
			yieldChange += numBanks * 3;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasMediciCard)
			yieldChange += numBanks;
	}
	{// CARD_MODERN_BUILDINGS_ANESTHESIA - +1/2 insight to Medical Labs
		const bool hasAnesthesiaCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_ANESTHESIA_PASSIVE);
		int numMedicalLabs = player.countNumBuildingClasses(BUILDINGCLASS_MEDICAL_LAB);
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasAnesthesiaCard)
			yieldChange += numMedicalLabs / 2;
	}
	{// CARD_INDUSTRIAL_BUILDINGS_OFFICER_TRAINING - +1/2 insight to military academies
		const bool hasOfficerCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_OFFICER_TRAINING_PASSIVE);
		int numMilitaryAcademies = player.countNumBuildingClasses(BUILDINGCLASS_MILITARY_ACADEMY);
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && hasOfficerCard)
			yieldChange += numMilitaryAcademies / 2;
	}
	break;
	case BUILDINGCLASS_TIBET:
	{// TIBET_STUPA // adds one of several yields every few techs
		const bool isStupa = eBuildingClass == BUILDINGCLASS_TIBET;
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
	break;
	case BUILDINGCLASS_RECYCLING_CENTER:
	{// Building_Recycling Center gets +1 Scientific Insight
		const bool isRecyclingCenter = eBuildingClass == BUILDINGCLASS_RECYCLING_CENTER;
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && isRecyclingCenter)
			yieldChange += 2;
	}
	break;
	case BUILDINGCLASS_CONQUERED_CITY_STATE:
	{// BUILDING_CONQUERED_CITY_STATE Center gets +10% FD, SC, C 
		const bool isConqueredCityStateBuilding = eBuildingClass == BUILDINGCLASS_CONQUERED_CITY_STATE;
		if (eYieldType == YIELD_FOOD && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
		if (eYieldType == YIELD_CULTURE && isPercentMod && isConqueredCityStateBuilding)
			yieldChange += 10;
	}
	break;
	case BUILDINGCLASS_BROADCAST_TOWER:
	{// BUILDING_EIFFEL_TOWER grants +2T to every broadcast tower 
		const bool hasEiffelTower = player.HasWonder(BUILDINGCLASS_EIFFEL_TOWER);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasEiffelTower)
			yieldChange += 2;
	}
	{// CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE - +2T to Broadcast Towers
		const bool hasThomsDoolittleCard = player.HasPolicy(POLICY_CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE_PASSIVE);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasThomsDoolittleCard)
			yieldChange += 2;
	}
	{// CARD_MODERN_BUILDINGS_FIRESIDE_CHATS - +2T to Broadcast Towers
		const bool hasFiresideChatsCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_FIRESIDE_CHATS_PASSIVE);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasFiresideChatsCard)
			yieldChange += 2;
	}
	break;
	case BUILDINGCLASS_STADIUM:
	{// CARD_MODERN_BUILDINGS_WORLD_CUP - +2T to Stadiums
		const bool hasWorldCupCard = player.HasPolicy(POLICY_CARD_MODERN_RESOURCES_THOMAS_DOOLITTLE_PASSIVE);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasWorldCupCard)
			yieldChange += 2;
	}
	break;
	case BUILDINGCLASS_HOTEL:
	{// BUILDING_BROADWAY grants +2T to every Hotel 
		const bool isHotel = eBuildingClass == BUILDINGCLASS_HOTEL;
		const bool hasBroadway = player.HasWonder(BUILDINGCLASS_EIFFEL_TOWER);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isHotel && hasBroadway)
			yieldChange += 2;
	}
	break;
	case BUILDINGCLASS_AIRPORT:
	{// BUILDING_SYDNEY_OPERA_HOUSE grants +3T to every Airport
		const bool isAirport = eBuildingClass == BUILDINGCLASS_AIRPORT;
		const bool hasSydney = player.HasWonder(BUILDINGCLASS_SYDNEY_OPERA_HOUSE);
		if (eYieldType == YIELD_TOURISM && !isPercentMod && isAirport && hasSydney)
			yieldChange += 3;
	}
	break;
	case BUILDINGCLASS_FORBIDDEN_PALACE:
	{// BUILDINGCLASS_FORBIDDEN_PALACE grants +1FD +1PD +1G +1Diplo from every City-State Ally
		const bool isForbiddenPalace = eBuildingClass == BUILDINGCLASS_FORBIDDEN_PALACE;
		if (eYieldType == YIELD_FOOD && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_GOLD && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT && !isPercentMod && isForbiddenPalace)
			yieldChange += numCityStateAllies;
	}
	break;
	case BUILDINGCLASS_WALLS:
	case BUILDINGCLASS_CASTLE:
	case BUILDINGCLASS_ARSENAL:
	case BUILDINGCLASS_MILITARY_BASE:
	{// BUILDINGCLASS_WALLS grants +1C +1PD to Walls, Castles, Arsenals, and Military Bases to Prussia

		const bool isPrussia = player.IsCiv(CIVILIZATION_PRUSSIA);
		if (eYieldType == YIELD_PRODUCTION && !isPercentMod && isPrussia)
			yieldChange += 1;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && isPrussia && isPrussia)
			yieldChange += 1;
	}
	break;
	case BUILDINGCLASS_SHRINE:
	{// CARD_ANCIENT_BUILDINGS_DRUIDS_PASSIVE 
		const bool hasDruidsCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_DRUIDS_PASSIVE);
		const bool isShrine = eBuildingClass == BUILDINGCLASS_SHRINE;
		if (eYieldType == YIELD_FAITH && !isPercentMod && hasDruidsCard && isShrine)
			yieldChange += 2;		
	}
	break;
	case BUILDINGCLASS_HARBOR:
	{// CARD_ANCIENT_BUILDINGS_HARBORMASTER_PASSIVE grants -2 Maintenance to Harbors
		const bool hasHarbormasterCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARBORMASTER_PASSIVE);
		const bool isHarbor = eBuildingClass == BUILDINGCLASS_HARBOR;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasHarbormasterCard && isHarbor)
			yieldChange -= 2;
	}
	break;
	case BUILDINGCLASS_OBSERVATORY:
	{// CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY If 5 Observeratories, +1 Insight
		const bool hasCopernicusCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY_PASSIVE);
		const bool isObserveratory = eBuildingClass == BUILDINGCLASS_OBSERVATORY;
		int numObserveratories = player.countNumBuildingClasses(BuildingClassTypes(10));
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT && !isPercentMod && (numObserveratories >= 5) && hasCopernicusCard && isObserveratory)
			yieldChange += 1;
	}
	break;
	case BUILDINGCLASS_STOCK_EXCHANGE:
	{// CARD_INDUSTRIAL_BUILDINGS_WALLSTREET - +25% G 5% Pd stock exchanges
		const bool hasWallStreetCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_WALLSTREET_PASSIVE);
		const bool isStockExchange = eBuildingClass == BUILDINGCLASS_STOCK_EXCHANGE;
		if (eYieldType == YIELD_GOLD && isPercentMod && hasWallStreetCard && isStockExchange)
			yieldChange += 25;
		if (eYieldType == YIELD_PRODUCTION && isPercentMod && hasWallStreetCard && isStockExchange)
			yieldChange += 5;
	}
	break;
	case BUILDINGCLASS_MUSEUM:
	{// CARD_INDUSTRIAL_BUILDINGS_IMPRESSIONALISM - +2C +2T to Mueseums
		const bool hasImpressionalismCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_IMPRESSIONALISM_PASSIVE);
		const bool isMuseum = eBuildingClass == BUILDINGCLASS_MUSEUM;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasImpressionalismCard && isMuseum)
			yieldChange += 2;
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasImpressionalismCard && isMuseum)
			yieldChange += 2;
	}
	break;
	case BUILDINGCLASS_GRAND_MONUMENT:
	{// CARD_MODERN_BUILDINGS_NEW_DEHLI - +5T to Grand Monument
		const bool hasNewDehliCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_NEW_DEHLI_PASSIVE);
		const bool isGrandMonument = eBuildingClass == BUILDINGCLASS_GRAND_MONUMENT;
		if (eYieldType == YIELD_TOURISM && !isPercentMod && hasNewDehliCard && isGrandMonument)
			yieldChange += 5;
	}
	break;
	case BUILDINGCLASS_ALHAMBRA:
	{// BUILDINGCLASS_ALHAMBRA - +2 C per City State
		const bool isAlhambra = eBuildingClass == BUILDINGCLASS_ALHAMBRA;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && isAlhambra && isAlhambra)
			yieldChange += numCityStateAllies * 2;
	}
	break;
	case BUILDINGCLASS_CIRCUS:
	{// POLICY_CARD_ANCIENT_BUILDINGS_GYPSIES_PASSIVE - 
		const bool hasGypsiesCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_GYPSIES_PASSIVE);
		const bool isCircus = eBuildingClass == BUILDINGCLASS_CIRCUS;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasGypsiesCard && isCircus)
			yieldChange -= 1;
	}
	break;
	case BUILDINGCLASS_STABLE:
	{// POLICY_CARD_ANCIENT_BUILDINGS_HARROW_PASSIVE - 
		const bool hasHarrowCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARROW_PASSIVE);
		const bool isStable = eBuildingClass == BUILDINGCLASS_STABLE;
		if (eYieldType == YIELD_FOOD && !isPercentMod && hasHarrowCard && isStable)
			yieldChange += 1;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasHarrowCard && isStable)
			yieldChange -= 1;
	}
	break;
	case BUILDINGCLASS_STONE_WORKS:
	{// POLICY_CARD_ANCIENT_BUILDINGS_MASONS_GUILD_PASSIVE - 
		const bool hasMasonsCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_MASONS_GUILD_PASSIVE);
		const bool isStoneWorks = eBuildingClass == BUILDINGCLASS_STONE_WORKS;
		if (eYieldType == YIELD_CULTURE && !isPercentMod && hasMasonsCard && isStoneWorks)
			yieldChange += 1;
		if (eYieldType == YIELD_MAINTENANCE && !isPercentMod && hasMasonsCard && isStoneWorks)
			yieldChange -= 1;
	}
	break;

	default:
		break;
	}

	return yieldChange;
}
BuildingAddType CvPlayer::ShouldHaveBuilding(const CvPlayer& rPlayer, const CvCity& rCity, const bool isYourCapital, const bool, const bool, const BuildingClassTypes eBuildingClass)
{
	// WARNING!! WARNINNG!! It is important that there ARE NOT multiple buildings with the same BuildingClass
	// Otherwise the code that calls this may not know to remove the existing type since the passed type resolves to a different building
	// this could be fixed by passing the explicit building type to this tag as well.

	{// POLICY_MERCHANT_CONFEDERACY gives BUILDING_MERCHANT_CONFEDERACY_TRADE_ROUTE to Capital
		const bool isMerchantConfederacyBuilding = eBuildingClass == BUILDINGCLASS_MERCHANT_CONFEDERACY_TRADE_ROUTE;
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
		const bool isFreeThoughtBuilding = eBuildingClass == BUILDINGCLASS_FREE_THOUGHT_TRADE_ROUTE;
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
		const bool isFreePopBuilding = eBuildingClass == BUILDINGCLASS_RATIONALISM_FINISHER_FREE_POP;
		if (isFreePopBuilding)
		{
			const bool hasRationalismFinisher3 = rPlayer.HasPolicy(POLICY_RATIONALISM_CLOSER_3);
			if (isYourCapital && hasRationalismFinisher3)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_PHILANTHROPY gives stuff for conquered City-States
		const bool isConqueredCityStateBuilding = eBuildingClass == BUILDINGCLASS_CONQUERED_CITY_STATE;
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
		const bool isJarliq = eBuildingClass == BUILDINGCLASS_COURTHOUSE;
		if (isJarliq)
		{
			const bool isRussia = rPlayer.IsCiv(CIVILIZATION_RUSSIA);
			const bool hasPhilosophy = rPlayer.HasTech(TECH_PHILOSOPHY);
			if (isRussia && hasPhilosophy)
				return ADD;
		}
	}

	{// BUILDING_CARD_SHIPS_OF_THE_DESERT goes in cities with Caravansary and POLICY_CARD_ANCIENT_BUILDINGS_SHIPS_OF_THE_DESERT_PASSIVE
		const bool isShipsBuilding = eBuildingClass == BUILDINGCLASS_CARD_SHIPS_OF_THE_DESERT;
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
		const bool isForcedLevyBuilding = eBuildingClass == BUILDINGCLASS_CARD_ANCIENT_BUILDINGS_FORCED_LEVY;
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
		const bool iswalls = eBuildingClass == BUILDINGCLASS_WALLS;
		if (iswalls)
		{
			const bool hasProtectiveCard = rPlayer.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_PROTECTIVE_PASSIVE);
			if (hasProtectiveCard)
				return ADD;
		}
	}

	{// BUILDING_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE goes in cities with Lighthouse and POLICY_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE
		const bool isBeaconsOfHopeBuilding = eBuildingClass == BUILDINGCLASS_CARD_CLASSICAL_BUILDINGS_BEACONS_OF_HOPE;
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
		const bool isStatue = eBuildingClass == BUILDINGCLASS_STATUE_1;
		if (isStatue)
		{
			const bool hasChampionCard = rPlayer.HasPolicy(POLICY_CARD_CLASSICAL_LEGENDARY_CHAMPION_PASSIVE);
			const bool hasBarracks = rCity.HasBuildingClass(BuildingClassTypes(25));
			if (hasChampionCard && hasBarracks)
				return ADD;
		}
	}

	{// CARD_CLASSICAL_BUILDINGS_FLUTE_AND_LYRE free Musicians Courtyard if Fuontain and Garden
		const bool isMusiciansCourtyard = eBuildingClass == BUILDINGCLASS_CARD_CLASSICAL_BUILDINGS_FLUTE_AND_LYRE;
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
		const bool isCambridgeUniversity = eBuildingClass == BUILDINGCLASS_CARD_MEDIEVAL_BUILDINGS_CAMBRIDGE_UNIVERSITY;
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
		const bool isFealtyBuilding = eBuildingClass == BUILDINGCLASS_CARD_MEDIEVAL_BUILDINGS_FEALTY;
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
		const bool isPennicilinBuilding = eBuildingClass == BUILDINGCLASS_CARD_INDUSTRIAL_BUILDINGS_PENICILLIN;
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
		const bool isCityCenterBuilding = eBuildingClass == BUILDINGCLASS_CITY_CENTER;
		if (isCityCenterBuilding)
			return ADD;
	}

	{// POLICY_AESTHETICS_CLOSER_5 free BUILDING_AESTHETICS_CLOSER_5 in Capital
		const bool isAesthetics5Building = eBuildingClass == BUILDINGCLASS_AESTHETICS_CLOSER_5;
		if (isAesthetics5Building)
		{
			const bool hasAesthetics5 = rPlayer.HasPolicy(POLICY_AESTHETICS_CLOSER_5);
			if (hasAesthetics5 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_LEGALISM free BUILDINGCLASS_POLICY_LEGALISM_GP_GENERATION in Capital
		const bool isLegalismBuilding = eBuildingClass == BUILDINGCLASS_POLICY_LEGALISM_GP_GENERATION;
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
		const bool isExplorationBuilding2 = eBuildingClass == BUILDINGCLASS_POLICY_EXPLORATION_CLOSER_2;
		if (isExplorationBuilding2)
		{
			const bool hasExploration2 = rPlayer.HasPolicy(POLICY_EXPLORATION_CLOSER_2);
			if (hasExploration2 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_EXPLORATION_CLOSER_5 free in Capital
		const bool isExplorationBuilding5 = eBuildingClass == BUILDINGCLASS_POLICY_EXPLORATION_CLOSER_5;
		if (isExplorationBuilding5)
		{
			const bool hasExploration5 = rPlayer.HasPolicy(POLICY_EXPLORATION_CLOSER_5);
			if (hasExploration5 && isYourCapital)
				return ADD;
			else
				return REMOVE;
		}
	}

	{// POLICY_CARD_ATOMIC_BUILDINGS_POLIO_VACCINE_ACTIVE free in Capital
		const bool isPolioVaccineBuilding = eBuildingClass == BUILDINGCLASS_CARD_ATOMIC_BUILDINGS_POLIO_VACCINE;
		if (isPolioVaccineBuilding)
		{
			const bool hasPolioVaccineCard = rPlayer.HasPolicy(POLICY_CARD_ATOMIC_BUILDINGS_POLIO_VACCINE_ACTIVE);
			if (hasPolioVaccineCard && isYourCapital)
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


	{// CIVILIZATION_ROME buffs Artists
		const bool isRome = player.IsCiv(CIVILIZATION_ROME);
		if (eYield == YIELD_CULTURE && isRome && isArtist)
			change += 1;
		if (eYield == YIELD_GOLD && isRome && isArtist)
			change += 1;
	}

	{// CIVILIZATION_MC_SCOTLAND buffs Writers
		const bool isEngland = player.IsCiv(CIVILIZATION_MC_SCOTLAND);
		if (eYield == YIELD_CULTURE && isEngland && isWriter)
			change += 1;		
	}

	{// CIVILIZATION_PRUSSIA buffs Engineers
		const bool isGermany = player.IsCiv(CIVILIZATION_PRUSSIA);
		if (eYield == YIELD_PRODUCTION && isGermany && isEngineer)
			change += 1;
	}

	{// CIVILIZATION_IROQUOIS buffs Musicians
		const bool isAmerica = player.IsCiv(CIVILIZATION_IROQUOIS);
		if (eYield == YIELD_TOURISM && isAmerica && isMusician)
			change += 1;
	}

	{// CIVILIZATION_KOREA buffs Scientists
		const bool isKorea = player.IsCiv(CIVILIZATION_KOREA);
		if (eYield == YIELD_SCIENCE && isKorea && isScientist)
			change += 1;
		if (eYield == YIELD_CULTURE && isKorea && isScientist)
			change += 1;
	}

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
		const bool hasGlobeTheater = player.HasWonder(BUILDINGCLASS_GLOBE_THEATER);
		if (eYield == YIELD_TOURISM && hasGlobeTheater && isWriter)
			change += 1;
		if (eYield == YIELD_CULTURE && hasGlobeTheater && isWriter)
			change += 1;
	}

	{// Sistine Chapel gives +1C, +1T  to Artist Specialists
		const bool hasSistineChapel = player.HasWonder(BUILDINGCLASS_SISTINE_CHAPEL);
		if (eYield == YIELD_CULTURE && hasSistineChapel && isArtist)
			change += 1;
		if (eYield == YIELD_TOURISM && hasSistineChapel && isArtist)
			change += 1;
	}

	{// Uffizi gives +1C, +1T  to Music Specialists
		const bool hasUffizi = player.HasWonder(BUILDINGCLASS_UFFIZI);
		if (eYield == YIELD_CULTURE && hasUffizi && isMusician)
			change += 1;
		if (eYield == YIELD_TOURISM && hasUffizi && isMusician)
			change += 1;
	}

	{// BUILDINGCLASS_BIG_BEN gives +1G +1PD to Merchants
		const bool hasBigBen = player.HasWonder(BUILDINGCLASS_BIG_BEN);
		if (eYield == YIELD_PRODUCTION && hasBigBen && isMerchant)
			change += 1;
		if (eYield == YIELD_GOLD && hasBigBen && isMerchant)
			change += 1;
	}

	{// BUILDINGCLASS_PANAMA gives +1G +1PD to Engineers
		const bool hasPanama = player.HasWonder(BUILDINGCLASS_PANAMA);
		if (eYield == YIELD_PRODUCTION && hasPanama && isEngineer)
			change += 1;
		if (eYield == YIELD_GOLD && hasPanama && isEngineer)
			change += 1;
	}

	{// BUILDINGCLASS_PORCELAIN_TOWER gives +1SC +1PD to Scientists
		const bool hasPorcelainTower = player.HasWonder(BUILDINGCLASS_PORCELAIN_TOWER);
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

	{// CARD_RENAISSANCE_BUILDINGS_WILLIAM_SHAKSPEARE 
		const bool hasWilliamShakespeareCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_WILLIAM_SHAKSPEARE_PASSIVE);
		const bool hasGlobeTheater = player.HasWonder(BUILDINGCLASS_GLOBE_THEATER);
		if (eYield == YIELD_TOURISM && hasWilliamShakespeareCard && hasGlobeTheater && isWriter)
			change += 1;		
	}
	{// CARD_RENAISSANCE_BUILDINGS_MICHAEL_ANGELO 
		const bool hasMichaelAngeloCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_MICHAEL_ANGELO_PASSIVE);
		const bool hasSistineChapel = player.HasWonder(BUILDINGCLASS_SISTINE_CHAPEL);
		if (eYield == YIELD_TOURISM && hasMichaelAngeloCard && hasSistineChapel && isArtist)
			change += 1;		
	}
	{// CARD_RENAISSANCE_BUILDINGS_J_S_BACH  
		const bool hasBachCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_J_S_BACH_PASSIVE);
		const bool hasTeatroAllaScala = player.HasWonder(BUILDINGCLASS_UFFIZI);
		if (eYield == YIELD_TOURISM && hasBachCard && hasTeatroAllaScala && isMusician)
			change += 1;		
	}
	{// CARD_RENAISSANCE_BUILDINGS_DOUBLE_ENTRY_ACCOUNTING gives +2G Merchant. +1PD +1Diplo if Big Ben
		const bool hasDoubleEntrCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_DOUBLE_ENTRY_ACCOUNTING_PASSIVE);
		const bool hasBigBen = player.HasWonder(BUILDINGCLASS_BIG_BEN);
		if (eYield == YIELD_GOLD && hasDoubleEntrCard && isMerchant)
			change += 2;		
		if (eYield == YIELD_DIPLOMATIC_SUPPORT && hasDoubleEntrCard && hasBigBen && isMerchant)
			change += 1;
	}

	{// CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY gives +2SC to Scientist.
		const bool hasCopernicusCard = player.HasPolicy(POLICY_CARD_RENAISSANCE_BUILDINGS_COPERNICUS_OBSERVERATORY_PASSIVE);
		if (eYield == YIELD_SCIENCE && hasCopernicusCard && isScientist)
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
		const bool hasGlobeTheater = player.HasWonder(BUILDINGCLASS_GLOBE_THEATER);
		if (eYield == YIELD_CULTURE && hasGlobeTheater && isWriting)
			change += 2;
		if (eYield == YIELD_TOURISM && hasGlobeTheater && isWriting)
			change += 2;
	}

	{// Sistine Chapel gives +2C, +2T  to GW Art
		const bool hasSistineChapel = player.HasWonder(BUILDINGCLASS_SISTINE_CHAPEL);
		if (eYield == YIELD_CULTURE && hasSistineChapel && isArt)
			change += 2;
		if (eYield == YIELD_TOURISM && hasSistineChapel && isArt)
			change += 2;
	}

	{// Uffizi gives +2C, +2T  to GW Music
		const bool hasUffizi = player.HasWonder(BUILDINGCLASS_UFFIZI);
		if (eYield == YIELD_CULTURE && hasUffizi && isMusic)
			change += 2;
		if (eYield == YIELD_TOURISM && hasUffizi && isMusic)
			change += 2;
	}

	{// Louvre gives +2C, +2T  to Artifacts
		const bool hasLourve = player.HasWonder(BUILDINGCLASS_LOUVRE);
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
		const bool hasLourve = player.HasWonder(BUILDINGCLASS_LOUVRE);		
		if (eYield == YIELD_TOURISM && hasHowardCarterCard && isArtifact)
			change += 1;		
		if (eYield == YIELD_TOURISM && hasHowardCarterCard && isArtifact && hasLourve)
			change += 1;
	}

	return change;
}
