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
		appendNewLine(&ss, sum, "from completing the Liberty Policy Tree", +1, player.HasPolicy("POLICY_LIBERTY_CLOSER_4"));
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
		if (player.HasPolicy("POLICY_NEW_ORDER"))
			maxConqueredBonus += 2;
		if (player.HasPolicy("POLICY_HONOR_CLOSER_4"))
			maxConqueredBonus += 1;

		stringstream desc;
		desc << "from 1 per Conquered City (max bonus of " << maxConqueredBonus << ")";
		appendNewLine(&ss, sum, desc.str(), min(maxConqueredBonus, numConquered), numConquered > 0);
	}
	{ // iron curtain +2
		appendNewLine(&ss, sum, "from the Iron Curtain Tenet", +2, player.HasPolicy("POLICY_IRON_CURTAIN"));
	}

	//{ // wonder
	//	appendNewLine(&ss, sum, "from THE LOUVE", +1, player.countNumBuildingClasses(BuildingClass("")) > 0);
	//}

	// ai get to build as many as they want
	if (!isHuman())
		*sum += 999;

	return ss.str();
}

int CvPlayer::GetExtraBuildingsForClass(BuildingClassTypes eClass) const
{
	int total = 0;


	return total;
}

int CvPlayer::GetMaxPoliciesForBranch(PolicyBranchTypes eBranch) const
{
	const CvPlayer& player = *this;
	int total = 3;



	return total;
}
void CvPlayer::UpdateFreePolicies()
{
	CvPlayer& player = *this;

	// automatically give player finisher once they max out a branch
	for (int i = 0; i < GC.getNumPolicyBranchInfos(); ++i)
	{
		const PolicyBranchTypes eBranch = (PolicyBranchTypes)i;
		const CvPolicyBranchEntry* pInfo = GC.getPolicyBranchInfo(eBranch);
		if (pInfo != NULL)
		{
			const int numHave = player.GetPlayerPolicies()->GetNumPoliciesOwnedInBranch(eBranch);
			const int numNeeded = player.GetMaxPoliciesForBranch(eBranch);
			const PolicyTypes eFinisher = (PolicyTypes)pInfo->GetFreeFinishingPolicy();
			const CvPolicyEntry* policyInfo = GC.getPolicyInfo(eFinisher);
			const bool doesGetPolicy = numHave >= numNeeded;
			if (policyInfo != NULL)
				UpdateHasPolicy(policyInfo->GetType(), doesGetPolicy);
		}
	}


	// EXAMPLE 1
	// If you want to possibly remove the policy if the CONDITIONAL fails
	//{
	//	const bool playerDeservesPolicy = CONDITIONAL;
	//	UpdateHasPolicy("POLICY_NAME", playerDeservesPolicy);
	//}
	//
	// EXAMPLE 2
	// If you want to reward it, and never remove it even if CONDITIONAL fails
	//if (CONDITIONAL)
	//{
	//	UpdateHasPolicy("POLICY_NAME", true);
	//}
}
