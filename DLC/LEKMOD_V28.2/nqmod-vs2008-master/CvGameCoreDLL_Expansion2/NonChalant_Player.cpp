// mod extension methods for the CvPlayer

#include "CvGameCoreDLLPCH.h"
#include "CvPlayer.h"
#include "CvGameCoreUtils.h"
#include "CvGoodyHuts.h"


void addColoredValue(stringstream& s, bool isGreen, const long value, const string description)
{
	const long absValue = abs(value);
	if (isGreen) s << "[COLOR_POSITIVE_TEXT]+" << absValue;
	else		 s << "[COLOR_GREY]+" << absValue;

	s << " " << description << "[ENDCOLOR]";
	s << "[NEWLINE]";
}

// will construct a string
void appendNewLine(stringstream* ss, int* numCitiesAllowed,
	// The description of where the numToAdd amount came from.
	string desc,
	// The number of additional cities the player could add.
	int numToAdd,
	bool isUnlocked)
{
	addColoredValue(*ss, isUnlocked, numToAdd, desc);
	if (isUnlocked)
		*numCitiesAllowed += numToAdd;
}
int ConqueredCities(const CvCity& c)
{
	if (c.isConquered())
		return 1;
	return 0;
}
string CvPlayer::GetCityCapCurrent_WithSourcesTooltip(int* sum) const
{
	stringstream ss;
	const CvPlayer& player = *this;

	{ // the base amount to start with always
		appendNewLine(&ss, sum, "Base", +3, true);
	}	
	{ // 1 for every other era, first unlock in classical
		// ancient era is id 0, classical is id 1...
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_1}", +1, player.GetCurrentEra() >= 1); // evaluates to Classical Era
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_2}", +1, player.GetCurrentEra() >= 2); // medieval
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_3}", +1, player.GetCurrentEra() >= 3); // renaissance
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_4}", +1, player.GetCurrentEra() >= 4); // industrial
		// last era is 8 (future)
	}
	//{ // 1 for 8 policies (free policies included)
	//	appendNewLine(&ss, sum, "from 8 or more Social Policies", +1, player.GetNumPoliciesTotal() >= 8);
	//}
	//{ // 1 per courthouse
	//	const int numConquered = CountNumCities(ConqueredCities);
	//	int maxConqueredBonus = 2;
	//	// iron curtain +1
	//	if (player.HasPolicy(POLICY_NEW_ORDER))
	//		maxConqueredBonus += 2;
	//	if (player.HasPolicy(POLICY_HONOR_CLOSER_4))
	//		maxConqueredBonus += 1;

	//	stringstream desc;
	//	desc << "from 1 per Conquered City (max bonus of " << maxConqueredBonus << ")";
	//	appendNewLine(&ss, sum, desc.str(), min(maxConqueredBonus, numConquered), numConquered > 0);
	//}
	//{ // liberty finisher
	//	appendNewLine(&ss, sum, "from adopting 4 policies in the Liberty branch", +1, player.HasPolicy(POLICY_LIBERTY_CLOSER_4));
	//}
	//{ // expansive card
	//	appendNewLine(&ss, sum, "from the Expansive Rite", +1, player.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_EXPANSIVE_ACTIVE));
	//}
	//{ // colonialism +1
	//	appendNewLine(&ss, sum, "from the Colonialism Policy", +1, player.HasPolicy(POLICY_MERCHANT_NAVY));
	//}	
	//{ 
	//	appendNewLine(&ss, sum, "from the Red Fort", +1, player.HasWonder(BUILDINGCLASS_RED_FORT));
	//}
	//{ // iron curtain +2
	//	appendNewLine(&ss, sum, "from the Iron Curtain Tenet", +2, player.HasPolicy(POLICY_IRON_CURTAIN));
	//}

	// ai get to build as many as they want
	if (!isHuman())
	{
		appendNewLine(&ss, sum, "Artifical Intelligence", +999, true); // industrial
	}

	return ss.str();
}

int CvPlayer::GetExtraBuildingsForClass(BuildingClassTypes eClass) const
{
	const CvPlayer& player = *this;
	int total = 0;

	// Policies that grant building supply
	const bool hasLandedElite = player.HasPolicy(POLICY_LANDED_ELITE);
	const bool hasOligachy = player.HasPolicy(POLICY_OLIGARCHY);
	const bool hasRepresentation = player.HasPolicy(POLICY_REPRESENTATION);
	const bool hasMilitaryTradidion = player.HasPolicy(POLICY_MILITARY_TRADITION);
	if (hasLandedElite && eClass == BuildingClassTypes(BUILDINGCLASS_GRANARY))
		total += 1;
	if (hasOligachy && eClass == BuildingClassTypes(BUILDINGCLASS_HUNTERS_HAVEN))
		total += 1;
	if (hasRepresentation && eClass == BuildingClassTypes(BUILDINGCLASS_STABLE))
		total += 1;
	if (hasMilitaryTradidion && eClass == BuildingClassTypes(BUILDINGCLASS_STONE_WORKS))
		total += 1;
	if (hasMilitaryTradidion && eClass == BuildingClassTypes(BUILDINGCLASS_FORGE))
		total += 1;
	

	// All Unique Buildings that have a % Limit get +1 Supply
	// All Civillizations Listed (if needed later)
	const bool isEngland = player.IsCiv(CIVILIZATION_MC_SCOTLAND);
	const bool isFrance = player.IsCiv(CIVILIZATION_FRANCE);
	const bool isAztec = player.IsCiv(CIVILIZATION_AZTEC);
	const bool isByzantium = player.IsCiv(CIVILIZATION_BYZANTIUM);
	const bool isPrussia = player.IsCiv(CIVILIZATION_PRUSSIA);
	const bool isRome = player.IsCiv(CIVILIZATION_ROME);
	const bool isMacedon = player.IsCiv(CIVILIZATION_MACEDON);
	const bool isPersia = player.IsCiv(CIVILIZATION_PERSIA);
	const bool isMiddleEast = player.IsCiv(CIVILIZATION_UC_TURKEY);
	const bool isEgypt = player.IsCiv(CIVILIZATION_EGYPT);
	const bool isAmerica = player.IsCiv(CIVILIZATION_IROQUOIS);
	const bool isKorea = player.IsCiv(CIVILIZATION_KOREA);
	const bool isRussia = player.IsCiv(CIVILIZATION_RUSSIA);
	const bool isDenmark = player.IsCiv(CIVILIZATION_DENMARK);
	const bool isZulu = player.IsCiv(CIVILIZATION_ZULU);
	const bool isInca = player.IsCiv(CIVILIZATION_INCA);
	const bool isIndia = player.IsCiv(CIVILIZATION_INDIA);
	const bool isIndonesia = player.IsCiv(CIVILIZATION_INDONESIA);
	if (isAztec && eClass == BuildingClassTypes(BUILDINGCLASS_WATERMILL))
		total += 1;
	if (isAztec && eClass == BuildingClassTypes(BUILDINGCLASS_WINDMILL))
		total += 1;
	if (isPrussia && eClass == BuildingClassTypes(BUILDINGCLASS_WINDMILL))
		total += 1;
	if (isEgypt && eClass == BuildingClassTypes(BUILDINGCLASS_FORGE))
		total += 1;
	if (isAmerica && eClass == BuildingClassTypes(BUILDINGCLASS_WORKSHOP))
		total += 1;
	if (isInca && eClass == BuildingClassTypes(BUILDINGCLASS_STABLE))
		total += 1;
	if (isInca && eClass == BuildingClassTypes(BUILDINGCLASS_FACTORY))
		total += 1;
	if (isEngland && eClass == BuildingClassTypes(BUILDINGCLASS_WRITERS_GUILD))
		total += 1;
	if (isEngland && eClass == BuildingClassTypes(BUILDINGCLASS_ARTISTS_GUILD))
		total += 1;
	if (isEngland && eClass == BuildingClassTypes(BUILDINGCLASS_MUSICIANS_GUILD))
		total += 1;


	// Rites that grant building supply
	const bool hasSnareCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_SNARE_PASSIVE);
	const bool hasSolorazationCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_SOLARIZATION_PASSIVE);
	const bool hasHarpoonCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARPOON_PASSIVE);
	const bool hasHarrowCard = player.HasPolicy(POLICY_CARD_ANCIENT_BUILDINGS_HARROW_PASSIVE);
	const bool hasBalanceScalesCard = player.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_BALANCE_SCALES_PASSIVE);
	const bool hasMillstoneCard = player.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_MILLSTONE_PASSIVE);
	const bool hasNationalTreasuryCard = player.HasPolicy(POLICY_CARD_CLASSICAL_BUILDINGS_NATIONAL_TREASURY_PASSIVE);
	const bool hasFerrousMetallurgyCard = player.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_FERROUS_METALLURGY_PASSIVE);
	const bool hasMonasticBreweryCard = player.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_MONASTIC_BREWERY_PASSIVE);	
	const bool hasUnionStockyarsCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_UNION_STOCKYARDS_PASSIVE);
	const bool hasDartsElevatorCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_DARTS_ELEVATOR_PASSIVE);
	const bool hasKeroseneCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_KEROSENE_PASSIVE);
	const bool hasHooverDamCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_HOOVER_DAM_PASSIVE);
	const bool hasBuffaloBillCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_RESOURCES_BUFFALO_BILL_CODY_PASSIVE);
	const bool hasLenigradCard = player.HasPolicy(POLICY_CARD_ATOMIC_BUILDINGS_LENINGRAD_FTI_PASSIVE);
	const bool hasAluminumCansCard = player.HasPolicy(POLICY_CARD_ATOMIC_BUILDINGS_ALUMINUM_CANS_PASSIVE);
	const bool hasCleanAirActCard = player.HasPolicy(POLICY_CARD_ATOMIC_BUILDINGS_CLEAN_AIR_ACT_PASSIVE);
	if (hasSnareCard && eClass == BuildingClassTypes(BUILDINGCLASS_HUNTERS_HAVEN))
		total += 1;
	if (hasSolorazationCard && eClass == BuildingClassTypes(BUILDINGCLASS_GRANARY))
		total += 1;
	if (hasHarpoonCard && eClass == BuildingClassTypes(BUILDINGCLASS_FISHERY))
		total += 1;
	if (hasHarrowCard && eClass == BuildingClassTypes(BUILDINGCLASS_STABLE))
		total += 1;
	if (hasBalanceScalesCard && eClass == BuildingClassTypes(BUILDINGCLASS_GROCER))
		total += 1;
	if (hasMillstoneCard && eClass == BuildingClassTypes(BUILDINGCLASS_WATERMILL))
		total += 1;
	if (hasNationalTreasuryCard && eClass == BuildingClassTypes(BUILDINGCLASS_MINT))
		total += 1;
	if (hasFerrousMetallurgyCard && eClass == BuildingClassTypes(BUILDINGCLASS_FORGE))
		total += 1;
	if (hasMonasticBreweryCard && eClass == BuildingClassTypes(BUILDINGCLASS_BREWERY))
		total += 1;	
	if (hasUnionStockyarsCard && eClass == BuildingClassTypes(BUILDINGCLASS_STOCKYARDS))
		total += 1;
	if (hasDartsElevatorCard && eClass == BuildingClassTypes(BUILDINGCLASS_GRAIN_ELEVATOR))
		total += 1;
	if (hasKeroseneCard && eClass == BuildingClassTypes(BUILDINGCLASS_REFINERY))
		total += 1;
	if (hasHooverDamCard && eClass == BuildingClassTypes(BUILDINGCLASS_HYDRO_PLANT))
		total += 1;
	if (hasBuffaloBillCard && eClass == BuildingClassTypes(BUILDINGCLASS_TEXTILE))
		total += 1;
	if (hasLenigradCard && eClass == BuildingClassTypes(BUILDINGCLASS_NUCLEAR_PLANT))
		total += 1;
	if (hasAluminumCansCard && eClass == BuildingClassTypes(BUILDINGCLASS_RECYCLING_CENTER))
		total += 1;
	if (hasCleanAirActCard && eClass == BuildingClassTypes(BUILDINGCLASS_SOLAR_PLANT))
		total += 1;

	return total;
}

int CvPlayer::GetMaxPoliciesForBranch(PolicyBranchTypes eBranch) const
{
	int total = 3;

	const CvPlayer& player = *this;
	const CvCity* pCity = player.getCapitalCity();

	if (pCity != NULL) // in a city
	{
		const CvCity& city = *pCity;
		const ReligionTypes majorityReligion = city.GetCityReligions()->GetReligiousMajority(); // the majority religion in this city
			//const int numCitiesFollowing = GC.getGame().GetGameReligions()->GetNumCitiesFollowing(majorityReligion); // number of cities with this as majority
		const bool isHolyCity = city.GetCityReligions()->IsHolyCityForReligion(majorityReligion); // true if this is the holy city of the majority religion in this city

		// Additional Policy Branch Capacity from Enhancer Beliefs
		const bool hasBelief1 = city.HasBelief(BELIEF_PROMISED_LAND);
		const bool hasBelief2 = city.HasBelief(BELIEF_CRAFTWORKS);
		const bool hasBelief3 = city.HasBelief(BELIEF_JUST_WAR);
		const bool hasBelief4 = city.HasBelief(BELIEF_UNITY_OF_PROPHETS);
		const bool hasBelief5 = city.HasBelief(BELIEF_PILGRIMAGE);
		const bool hasBelief6 = city.HasBelief(BELIEF_KARMA);
		const bool hasBelief7 = city.HasBelief(BELIEF_DARMA);
		const bool hasBelief8 = city.HasBelief(BELIEF_DEFENDER_FAITH);
		const bool hasBelief9 = city.HasBelief(BELIEF_KOTEL);
		if (hasBelief1 && isHolyCity && eBranch == PolicyBranchTypes(0))
			total += 1;
		if (hasBelief2 && isHolyCity && eBranch == PolicyBranchTypes(1))
			total += 1;
		if (hasBelief3 && isHolyCity && eBranch == PolicyBranchTypes(2))
			total += 1;
		if (hasBelief4 && isHolyCity && eBranch == PolicyBranchTypes(3))
			total += 1;
		if (hasBelief5 && isHolyCity && eBranch == PolicyBranchTypes(4))
			total += 1;
		if (hasBelief6 && isHolyCity && eBranch == PolicyBranchTypes(5))
			total += 1;
		if (hasBelief7 && isHolyCity && eBranch == PolicyBranchTypes(6))
			total += 1;
		if (hasBelief8 && isHolyCity && eBranch == PolicyBranchTypes(7))
			total += 1;
		if (hasBelief9 && isHolyCity && eBranch == PolicyBranchTypes(8))
			total += 1;
	}

	// Additional Policy Branch Capacity from World Wonders
	const bool hasWorldWonderUnlock1 = player.HasWonder(BUILDINGCLASS_CHICHEN_ITZA);
	const bool hasWorldWonderUnlock2 = player.HasWonder(BUILDINGCLASS_MACHU_PICHU);
	const bool hasWorldWonderUnlock3 = player.HasWonder(BUILDINGCLASS_GREAT_WALL);
	const bool hasWorldWonderUnlock4 = player.HasWonder(BUILDINGCLASS_HAGIA_SOPHIA);
	const bool hasWorldWonderUnlock5 = player.HasWonder(BUILDINGCLASS_ALHAMBRA);
	const bool hasWorldWonderUnlock6 = player.HasWonder(BUILDINGCLASS_BOROBUDUR);
	const bool hasWorldWonderUnlock7 = player.HasWonder(BUILDINGCLASS_HIMEJI_CASTLE);
	const bool hasWorldWonderUnlock8 = player.HasWonder(BUILDINGCLASS_RED_FORT);
	const bool hasWorldWonderUnlock9 = player.HasWonder(BUILDINGCLASS_BRANDENBURG_GATE);
	if (hasWorldWonderUnlock1 && eBranch == PolicyBranchTypes(0))
		total += 1;
	if (hasWorldWonderUnlock2 && eBranch == PolicyBranchTypes(1))
		total += 1;
	if (hasWorldWonderUnlock3 && eBranch == PolicyBranchTypes(2))
		total += 1;
	if (hasWorldWonderUnlock4 && eBranch == PolicyBranchTypes(3))
		total += 1;
	if (hasWorldWonderUnlock5 && eBranch == PolicyBranchTypes(4))
		total += 1;
	if (hasWorldWonderUnlock6 && eBranch == PolicyBranchTypes(5))
		total += 1;
	if (hasWorldWonderUnlock7 && eBranch == PolicyBranchTypes(6))
		total += 1;
	if (hasWorldWonderUnlock8 && eBranch == PolicyBranchTypes(7))
		total += 1;
	if (hasWorldWonderUnlock9 && eBranch == PolicyBranchTypes(8))
		total += 1;

	// Additional Policy Branch Capacity from National Wonders
	const bool hasNationalCollege = player.HasWonder(BUILDINGCLASS_NATIONAL_COLLEGE);
	const bool hasNationalEpic = player.HasWonder(BUILDINGCLASS_NATIONAL_EPIC);
	const bool hasHeroicEpic = player.HasWonder(BUILDINGCLASS_HEROIC_EPIC);
	const bool hasGrandTemple = player.HasWonder(BUILDINGCLASS_GRAND_TEMPLE);
	const bool hasThisMakesNoSense = player.HasWonder(BUILDINGCLASS_CIRCUS_MAXIMUS);
	const bool hasHermitage = player.HasWonder(BUILDINGCLASS_HERMITAGE);
	const bool hasIronworks = player.HasWonder(BUILDINGCLASS_IRONWORKS);
	const bool hasEastIndia = player.HasWonder(BUILDINGCLASS_NATIONAL_TREASURY);
	const bool hasOxfordUniversity = player.HasWonder(BUILDINGCLASS_OXFORD_UNIVERSITY);
	if (hasNationalCollege && eBranch == PolicyBranchTypes(0))
		total += 1;
	if (hasNationalEpic && eBranch == PolicyBranchTypes(1))
		total += 1;
	if (hasHeroicEpic && eBranch == PolicyBranchTypes(2))
		total += 1;
	if (hasGrandTemple && eBranch == PolicyBranchTypes(3))
		total += 1;
	if (hasThisMakesNoSense && eBranch == PolicyBranchTypes(4))
		total += 1;
	if (hasHermitage && eBranch == PolicyBranchTypes(5))
		total += 1;
	if (hasIronworks && eBranch == PolicyBranchTypes(6))
		total += 1;
	if (hasEastIndia && eBranch == PolicyBranchTypes(7))
		total += 1;
	if (hasOxfordUniversity && eBranch == PolicyBranchTypes(8))
		total += 1;


	// Additional Policy Capacity from Trading Cards
	const bool hasCard1 = player.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_HAMMURABIS_CODE_ACTIVE);
	const bool hasCard2 = player.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_ROMAN_REPUBLIC_ACTIVE);
	const bool hasCard3 = player.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_AUTHORITARIANISM_ACTIVE);
	const bool hasCard4 = player.HasPolicy(POLICY_CARD_CLASSICAL_POLITICAL_DIVINE_RIGHT_ACTIVE);
	const bool hasCard5 = player.HasPolicy(POLICY_CARD_CLASSICAL_POLITICAL_EMISSARIES_ACTIVE);
	const bool hasCard6 = player.HasPolicy(POLICY_CARD_CLASSICAL_POLITICAL_CLASSICISM_ACTIVE);
	const bool hasCard7 = player.HasPolicy(POLICY_CARD_RENAISSANCE_POLITICAL_URBANIZATION_ACTIVE);
	const bool hasCard8 = player.HasPolicy(POLICY_CARD_RENAISSANCE_POLITICAL_IMPERIAL_DECREE_ACTIVE);
	const bool hasCard9 = player.HasPolicy(POLICY_CARD_RENAISSANCE_POLITICAL_ENLIGHTENMENT_ACTIVE);
	if (hasCard1 && eBranch == PolicyBranchTypes(0))
		total += 1;
	if (hasCard2 && eBranch == PolicyBranchTypes(1))
		total += 1;
	if (hasCard3 && eBranch == PolicyBranchTypes(2))
		total += 1;
	if (hasCard4 && eBranch == PolicyBranchTypes(3))
		total += 1;
	if (hasCard5 && eBranch == PolicyBranchTypes(4))
		total += 1;
	if (hasCard6 && eBranch == PolicyBranchTypes(5))
		total += 1;
	if (hasCard7 && eBranch == PolicyBranchTypes(6))
		total += 1;
	if (hasCard8 && eBranch == PolicyBranchTypes(7))
		total += 1;
	if (hasCard9 && eBranch == PolicyBranchTypes(8))
		total += 1;


	return total;
}
void CvPlayer::UpdateFreePolicies()
{
	CvPlayer& player = *this;

	// automatically give player finisher once they max out a branch
	/**for (int i = 0; i < GC.getNumPolicyBranchInfos(); ++i)
	{
		const PolicyBranchTypes eBranch = (PolicyBranchTypes)i;
		const CvPolicyBranchEntry* pInfo = GC.getPolicyBranchInfo(eBranch);
		if (pInfo != NULL)
		{
			const int numHave = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(eBranch);
			const int numNeeded = player.GetMaxPoliciesForBranch(eBranch);
			const PolicyTypes eFinisher = (PolicyTypes)pInfo->GetFreeFinishingPolicy();
			const bool doesGetPolicy = numHave >= numNeeded;
				UpdateHasPolicy(eFinisher, doesGetPolicy);
		}
	}**/


	//for (int i = 0; i < GC.getNumPolicyBranchInfos(); ++i)
	//{
	//	const PolicyBranchTypes eBranch = (PolicyBranchTypes)i;
	//	const CvPolicyBranchEntry* pInfo = GC.getPolicyBranchInfo(eBranch);
	//	if (pInfo != NULL)
	//	{
	//		const int numHave = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(eBranch);
	//		const int numNeeded = player.GetMaxPoliciesForBranch(eBranch);
	//		const PolicyTypes eFinisher = (PolicyTypes)pInfo->GetFreeFinishingPolicy();
	//		const CvPolicyEntry* policyInfo = GC.getPolicyInfo(eFinisher);
	//		const bool doesGetPolicy = numHave >= numNeeded;
	//		if (policyInfo != NULL)
	//			UpdateHasPolicy(policyInfo->GetType(), doesGetPolicy);
	//	}
	//}

	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_TRADITION_CLOSER_1,true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_TRADITION_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_TRADITION_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_TRADITION_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(0));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_TRADITION_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_LIBERTY_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_LIBERTY_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_LIBERTY_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_LIBERTY_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(1));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_LIBERTY_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_HONOR_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_HONOR_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_HONOR_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_HONOR_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(2));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_HONOR_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(3));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_PIETY_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(3));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_PIETY_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(3));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_PIETY_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(3));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_PIETY_FINISHER, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(3));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_PIETY_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(4));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_PATRONAGE_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(4));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_PATRONAGE_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(4));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_PATRONAGE_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(4));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_PATRONAGE_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(4));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_PATRONAGE_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(5));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_AESTHETICS_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(5));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_AESTHETICS_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(5));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_AESTHETICS_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(5));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_AESTHETICS_FINISHER, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(5));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_AESTHETICS_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_COMMERCE_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_COMMERCE_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_COMMERCE_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_COMMERCE_FINISHER, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_TRADITION_FINISHER, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(6));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_COMMERCE_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(7));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_EXPLORATION_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(7));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_EXPLORATION_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(7));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_EXPLORATION_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(7));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_EXPLORATION_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(7));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_EXPLORATION_CLOSER_5, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(8));
		if (numPolicies >= 1)
			UpdateHasPolicy(POLICY_RATIONALISM_CLOSER_1, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(8));
		if (numPolicies >= 2)
			UpdateHasPolicy(POLICY_RATIONALISM_CLOSER_2, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(8));
		if (numPolicies >= 3)
			UpdateHasPolicy(POLICY_RATIONALISM_CLOSER_3, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(8));
		if (numPolicies >= 4)
			UpdateHasPolicy(POLICY_RATIONALISM_CLOSER_4, true);
	}
	{
		int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(PolicyBranchTypes(8));
		if (numPolicies >= 5)
			UpdateHasPolicy(POLICY_RATIONALISM_CLOSER_5, true);
	}
	// EXAMPLE 1
	// If you want to possibly remove the policy if the CONDITIONAL fails
	//{
	//	const bool playerDeservesPolicy = CONDITIONAL;
	//	UpdateHasPolicy(POLICY_NAME", playerDeservesPolicy);
	//}
	//
	// EXAMPLE 2
	// If you want to reward it, and never remove it even if CONDITIONAL fails
	//if (CONDITIONAL)
	//{
	//	UpdateHasPolicy(POLICY_NAME", true);
	//}
}




const int t100 = (100 * 100);
// random number
int rand(int maxExclusive, const CvPlot* pPlot)
{
	return (GC.getGame().getJonRandNum(maxExclusive, "generic_rand", pPlot, 1));
}
// randomly adjust this value by +/- 10%
int varyByPercent(int original, const CvPlot* pPlot)
{
	const int range = 20; // random value up to this value
	const int percentageT100 = rand(range, pPlot) - (range / 2); // percentageT100 now in range [-range, +range]
	const int resultT100 = original * (100 + percentageT100);
	return resultT100 / 100;
}
// Food a goody hut gives
int hutFood(const CvPlot* pPlot)
{
	int value = (2 * (7 * t100 + GC.onePerOnlineSpeedTurnT10000()));
	value *= GC.adjustForSpeedT100(YIELD_FOOD);
	value /= 100;
	return varyByPercent(value, pPlot) / (100 * 100);
}
// Production a goody hut gives
int hutProduction(const CvPlot* pPlot)
{
	int valueT10000 = (2 * (7 * t100 + GC.onePerOnlineSpeedTurnT10000()));
	valueT10000 *= GC.adjustForSpeedT100(YIELD_PRODUCTION);
	valueT10000 /= 100;
	return varyByPercent(valueT10000, pPlot) / (100 * 100);
}
// Gold a goody hut gives
int hutGold(const CvPlot* pPlot)
{
	T100 valueT10000 = (2 * (60 * t100 + GC.onePerOnlineSpeedTurnT10000()));
	valueT10000 *= GC.adjustForSpeedT100(YIELD_GOLD);
	valueT10000 /= 100;
	return varyByPercent(valueT10000, pPlot) / (100 * 100);
}
// Science a goody hut gives
int hutScience(const CvPlot* pPlot)
{
	//T100 valueT10000 = (2 * (20 * t100 + GC.onePerOnlineSpeedTurnT10000()));
	//valueT10000 *= GC.adjustForSpeedT100(YIELD_SCIENCE);
	//valueT10000 /= 100;
	//return varyByPercent(valueT10000, pPlot) / (100 * 100);
	return 5;
}
// Culture a goody hut gives
int hutCulture(const CvPlot* pPlot)
{
	T100 valueT10000 = (2 * (10 * t100 + GC.onePerOnlineSpeedTurnT10000()));
	valueT10000 *= 2;
	valueT10000 /= 3;
	valueT10000 *= GC.adjustForSpeedT100(YIELD_CULTURE);
	valueT10000 /= 100;
	return varyByPercent(valueT10000, pPlot) / (100 * 100);
}
// Culture a goody hut gives
int hutFaith(const CvPlot* pPlot)
{
	T100 valueT10000 = 2 * (15 * t100 + GC.onePerOnlineSpeedTurnT10000());
	valueT10000 *= GC.adjustForSpeedT100(YIELD_FAITH);
	valueT10000 /= 100;
	return varyByPercent(valueT10000, pPlot) / (100 * 100);
}
// Map radius a goody hut gives
int hutMapRadius(const CvPlot* pPlot)
{
	int baseRadius = 9; // starts with this much radius
	int turnsPerRadius = 12; // on average, will grant 1 more radius every 20 turns on online speed
	int randValue = GC.getGame().getJonRandNum(turnsPerRadius + 1, "Goody Hut Map", pPlot, 35); // [1-20]
	int bonusRadius = ((GC.onePerOnlineSpeedTurnT10000() / t100) + randValue) / turnsPerRadius;
	int value = baseRadius + bonusRadius;
	return value;
}
// random tile offset
int hutMapOffset()
{
	return 2;
}
// odds any individual tile will be revealed
T100 hutMapHexProbabilityT100()
{
	return 80;
}

//	--------------------------------------------------------------------------------
void CvPlayer::receiveGoody(CvPlot* pPlot, GoodyTypes eGoody, CvUnit* pUnit)
{
	//////////
	// Load from database
	//////////
	CvAssertMsg(canReceiveGoody(pPlot, eGoody, pUnit), "Instance is expected to be able to recieve goody");
	Database::SingleResult kResult;
	CvGoodyInfo kGoodyInfo;
	const bool bResult = DB.SelectAt(kResult, "GoodyHuts", eGoody);
	DEBUG_VARIABLE(bResult);
	CvAssertMsg(bResult, "Cannot find goody info.");
	kGoodyInfo.CacheResult(kResult);
	CvGoodyHuts::DoPlayerReceivedGoody(GetID(), eGoody);

	stringstream ss;
	int iRewardValue = 0; // amount of reward
	int iNumYieldBonuses = 0; // count plot yield display lines

	//////////
	// Gold
	//////////
	if (kGoodyInfo.isGold())
	{
		iRewardValue = hutGold(pPlot);
		GetTreasury()->ChangeGold(iRewardValue);

		ReportYieldFromKill(YIELD_GOLD, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
		iNumYieldBonuses += 1;
	}
	//////////
	// Food
	//////////
	if (kGoodyInfo.isFood())
	{
		CvCity* pChosenCity = this->getCapitalCity();
		if (pChosenCity != NULL)
		{
			iRewardValue = hutFood(pPlot);
			pChosenCity->changeFood(iRewardValue);

			ReportYieldFromKill(YIELD_FOOD, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
			iNumYieldBonuses += 1;
		}
	}
	//////////
	// Production
	//////////
	if (kGoodyInfo.isProduction())
	{
		CvCity* pChosenCity = this->getCapitalCity();
		if (pChosenCity != NULL)
		{
			iRewardValue = hutProduction(pPlot);
			pChosenCity->changeProduction(iRewardValue);

			ReportYieldFromKill(YIELD_PRODUCTION, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
			iNumYieldBonuses += 1;
		}
	}
	//////////
	// Beakers
	//////////
	if (kGoodyInfo.isBeakers())
	{
		ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
		if (pkScriptSystem)
		{
			CvLuaArgsHandle args;
			args->Push(GetID());
			args->Push(NO_TECH);

			bool bScriptResult;
			LuaSupport::CallHook(pkScriptSystem, "GoodyHutTechResearched", args.get(), bScriptResult);
		}

		// give this many beakers to the player
		iRewardValue = hutScience(pPlot);
		CvPlayer* pPlayer = this;
		CvTeam* pTeam = &GET_TEAM(pPlayer->getTeam());
		TechTypes eCurrentTech = pPlayer->GetPlayerTechs()->GetCurrentResearch();
		if (eCurrentTech == NO_TECH)
			this->changeOverflowResearch(iRewardValue);
		else
			pTeam->GetTeamTechs()->ChangeResearchProgress(eCurrentTech, iRewardValue, pPlayer->GetID());

		// show on tile
		ReportYieldFromKill(YIELD_SCIENCE, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
		iNumYieldBonuses += 1;
	}
	//////////
	// Culture
	//////////
	if (kGoodyInfo.isCulture())
	{
		iRewardValue = hutCulture(pPlot);
		changeJONSCulture(iRewardValue);

		// show on tile
		ReportYieldFromKill(YIELD_CULTURE, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
		iNumYieldBonuses += 1;
	}
	//////////
	// Faith
	//////////
	if (kGoodyInfo.isFaith())
	{
		iRewardValue = hutFaith(pPlot);
		ChangeFaith(iRewardValue);

		// show on tile
		ReportYieldFromKill(YIELD_FAITH, iRewardValue, pPlot->getX(), pPlot->getY(), iNumYieldBonuses);
		iNumYieldBonuses += 1;
	}
	//////////
	// Map
	//////////
	if (kGoodyInfo.isMap())
	{
		const int iRange = hutMapRadius(pPlot);
		reveal(getTeam(), pPlot, iRange, hutMapHexProbabilityT100());
	}
	//////////
	// Card
	//////////
	if (kGoodyInfo.isCard() > 0)
	{
		int numCards = kGoodyInfo.isCard();
		for (int i = 0; i < numCards; ++i)
		{
			TradingCardTypes goodyHutCard = CardsGetRandomValid();
			CardsAdd(goodyHutCard); // handles the message
		}
	}
	//////////
	// Units
	//////////
	if (kGoodyInfo.isUnit())
	{
		UnitTypes eUnit = (UnitTypes)GC.getInfoTypeForString("UNIT_ARCHER");
		CvCity* pChosenCity = this->getCapitalCity(); // try to put at capital
		CvPlot* pTargetPlot = pPlot;
		if (pChosenCity != NULL)
		{
			pTargetPlot = pChosenCity->plot();
		}

		CvUnit* pNewUnit = initUnit(eUnit, pTargetPlot->getX(), pTargetPlot->getY());
		pNewUnit->setXY(pTargetPlot->getX(), pTargetPlot->getY(), false, true, pTargetPlot->isVisibleToWatchingHuman(), true);
		pNewUnit->SetPosition(pTargetPlot); // Need this to put the unit in the right spot graphically
		pNewUnit->finishMoves();
	}

	if (iRewardValue != 0)
	{
		ss << iRewardValue;
	}

	CvString strBuffer = GetLocalizedText(kGoodyInfo.GetDescriptionKey(), ss.str().c_str());
	// messages
	if (!strBuffer.empty())
	{
		if (GC.getGame().getActivePlayer() == GetID())
			GC.messagePlot(0, pPlot->GetPlotIndex(), GetID(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer);
	}

	/// just remove the default popup
	// If it's the active player then show the popup
	//if(GetID() == GC.getGame().getActivePlayer())
	//{
	//	GC.getMap().updateDeferredFog();

	//	// Don't show in MP, or if the player has turned it off
	//	bool bDontShowRewardPopup = GC.GetEngineUserInterface()->IsOptionNoRewardPopups();
	//	if(!GC.getGame().isNetworkMultiPlayer() && !bDontShowRewardPopup)	// KWG: Candidate for !GC.getGame().isOption(GAMEOPTION_SIMULTANEOUS_TURNS)
	//	{
	//		CvPopupInfo kPopupInfo(BUTTONPOPUP_GOODY_HUT_REWARD, eGoody, iRewardValue);
	//		GC.GetEngineUserInterface()->AddPopup(kPopupInfo);
	//		// We are adding a popup that the player must make a choice in, make sure they are not in the end-turn phase.
	//		CancelActivePlayerEndTurn();
	//	}
	//}
}
