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
		appendNewLine(&ss, sum, "from completing the Liberty Policy Tree", +1, player.HasPolicy("POLICY_LIBERTY_FINISHER"));
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
	//{ // 1 per courthouse
	//	const int numCourthouses = player.countNumBuildingClasses(BuildingClass("BUILDINGCLASS_COURTHOUSE"));
	//	int maxCourthouseBonus = 2;
	//	// iron curtain +1
	//	if (player.HasPolicy("POLICY_NEW_ORDER"))
	//		maxCourthouseBonus += 2;
	//	if (player.HasPolicy("POLICY_HONOR_FINISHER"))
	//		maxCourthouseBonus += 1;

	//	stringstream desc;
	//	desc << "from 1 per Courthouse (max " << maxCourthouseBonus << ")";
	//	appendNewLine(&ss, sum, desc.str(), min(maxCourthouseBonus, numCourthouses), numCourthouses > 0);
	//}
	{ // 1 per conquered city
		int numConquered = CountNumCities(ConqueredCities);
		stringstream desc;
		desc << "from 1 per Conquered City";
		appendNewLine(&ss, sum, desc.str(), numConquered, numConquered > 0);
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

