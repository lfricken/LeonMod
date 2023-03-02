// mod extension methods for the CvPlayer

#include "CvGameCoreDLLPCH.h"
#include "CvPlayer.h"
#include "CvGameCoreUtils.h"

using namespace std;


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
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_1}", +1, player.GetCurrentEra() >= 1); // {} evaluates to Classical Era
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_3}", +1, player.GetCurrentEra() >= 3);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_5}", +1, player.GetCurrentEra() >= 5);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_7}", +1, player.GetCurrentEra() >= 7);
		// last era is 8 (future)
	}
	{ // 1 for 8 policies (free policies included)
		appendNewLine(&ss, sum, "from 8 or more Social Policies", +1, player.GetNumPoliciesTotal() >= 8);
	}
	{ // 1 per courthouse
		const int numConquered = CountNumCities(ConqueredCities);
		int maxConqueredBonus = 2;
		// iron curtain +1
		if (player.HasPolicy(POLICY_NEW_ORDER))
			maxConqueredBonus += 2;
		if (player.HasPolicy(POLICY_HONOR_CLOSER_4))
			maxConqueredBonus += 1;

		stringstream desc;
		desc << "from 1 per Conquered City (max bonus of " << maxConqueredBonus << ")";
		appendNewLine(&ss, sum, desc.str(), min(maxConqueredBonus, numConquered), numConquered > 0);
	}
	{ // liberty finisher
		appendNewLine(&ss, sum, "from adopting 4 policies in the Liberty branch", +1, player.HasPolicy(POLICY_LIBERTY_CLOSER_4));
	}
	{ // expansive card
		appendNewLine(&ss, sum, "from the Expansive Rite", +1, player.HasPolicy(POLICY_CARD_ANCIENT_POLITICAL_EXPANSIVE_ACTIVE));
	}
	{ // colonialism +1
		appendNewLine(&ss, sum, "from the Colonialism Policy", +1, player.HasPolicy(POLICY_MERCHANT_NAVY));
	}	
	{ 
		appendNewLine(&ss, sum, "from the Red Fort", +1, player.HasWonder(BUILDINGCLASS_RED_FORT));
	}
	{ // iron curtain +2
		appendNewLine(&ss, sum, "from the Iron Curtain Tenet", +2, player.HasPolicy(POLICY_IRON_CURTAIN));
	}

	// ai get to build as many as they want
	if (!isHuman())
		*sum += 999;

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

	// Mausoleum +1 Stoneworks Supply
	const bool hasMausoleum = player.HasWonder(BUILDINGCLASS_MAUSOLEUM_HALICARNASSUS);
	if (hasMausoleum && eClass == BuildingClassTypes(BUILDINGCLASS_STONE_WORKS))
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
	const bool hasRhineCard = player.HasPolicy(POLICY_CARD_MEDIEVAL_BUILDINGS_RHINE_RIVER_DELTA_PASSIVE);
	const bool hasUnionStockyarsCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_UNION_STOCKYARDS_PASSIVE);
	const bool hasDartsElevatorCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_DARTS_ELEVATOR_PASSIVE);
	const bool hasKeroseneCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_KEROSENE_PASSIVE);
	const bool hasHooverDamCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_BUILDINGS_HOOVER_DAM_PASSIVE);
	const bool hasBuffaloBillCard = player.HasPolicy(POLICY_CARD_INDUSTRIAL_RESOURCES_BUFFALO_BILL_CODY_PASSIVE);
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
	if (hasRhineCard && eClass == BuildingClassTypes(BUILDINGCLASS_WINDMILL))
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
