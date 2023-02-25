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
	{ // liberty finisher
		appendNewLine(&ss, sum, "from adopting 4 policies in the Liberty branch", +1, player.HasPolicy(POLICY_LIBERTY_CLOSER_4));
	}
	{ // 1 for 8 policies (free policies included)
		appendNewLine(&ss, sum, "from 8 or more Social Policies", +1, player.GetNumPoliciesTotal() >= 8);
	}
	{ // 1 for every other era, first unlock in classical
		// ancient era is id 0, classical is id 1...
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_1}", +1, player.GetCurrentEra() >= 1); // {} evaluates to Classical Era
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_3}", +1, player.GetCurrentEra() >= 3);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_5}", +1, player.GetCurrentEra() >= 5);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_7}", +1, player.GetCurrentEra() >= 7);
		// last era is 8 (future)
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
	{ // colonialism +1
		appendNewLine(&ss, sum, "from the Colonialism Policy", +1, player.HasPolicy(POLICY_MERCHANT_NAVY));
	}
	{ // iron curtain +2
		appendNewLine(&ss, sum, "from the Iron Curtain Tenet", +2, player.HasPolicy(POLICY_IRON_CURTAIN));
	}

	//{ // wonder
	//	appendNewLine(&ss, sum, "from THE LOUVE", +1, player.countNumBuildingClasses(")) > 0);
	//}

	// ai get to build as many as they want
	if (!isHuman())
		*sum += 999;

	return ss.str();
}

int CvPlayer::GetExtraBuildingsForClass(BuildingClassTypes eClass) const
{
	const CvPlayer& player = *this; 
	int total = 0;
	
	// Landed Elite +1 Granary Supply
	const bool hasLandedElite = player.HasPolicy(POLICY_LANDED_ELITE);	
	if (hasLandedElite && eClass == BuildingClassTypes(32))
		total += 1;

	// Oligarchy +1 Hunters Haven Supply
	const bool hasOligachy = player.HasPolicy(POLICY_OLIGARCHY);
	if (hasOligachy && eClass == BuildingClassTypes(254))
		total += 1;

	// Representation +1 Stable Supply
	const bool hasRepresentation = player.HasPolicy(POLICY_REPRESENTATION);
	if (hasRepresentation && eClass == BuildingClassTypes(2))
		total += 1;

	// Military Tradition +1 Stoneworks, Forge Supply
	const bool hasMilitaryTradidion = player.HasPolicy(POLICY_MILITARY_TRADITION);
	if (hasMilitaryTradidion && eClass == BuildingClassTypes(84))
		total += 1;
	if (hasMilitaryTradidion && eClass == BuildingClassTypes(5))
		total += 1;

	return total;
}

int CvPlayer::GetMaxPoliciesForBranch(PolicyBranchTypes eBranch) const
{
	const CvPlayer& player = *this;
	int total = 3;

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
