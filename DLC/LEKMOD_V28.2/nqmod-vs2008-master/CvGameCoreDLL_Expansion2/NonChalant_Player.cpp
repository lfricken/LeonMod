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

string CvPlayer::GetCityCapCurrent_WithSourcesTooltip(int* sum) const
{
	stringstream ss;
	const CvPlayer& player = *this;

	{ // the base amount to start with always
		const int baseNum = 3 * (true);
		appendNewLine(&ss, sum, "Base", baseNum, true);
	}

	// 1 for every other era, first unlock in classical
	{
		// ancient era is id 0, classical is id 1...
		const int Classical = 1;
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_1}", Classical, player.GetCurrentEra() >= 1); // {} evaluates to Classical Era

		const int Renaissance = 1;
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_3}", Renaissance, player.GetCurrentEra() >= 3);

		const int Modern = 1;
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_5}", Modern, player.GetCurrentEra() >= 5);

		const int Future = 1;
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_7}", Future, player.GetCurrentEra() >= 7);
		// last era is 8 (future)
	}

	{ // 1 per courthouse
		const int numCourthouses = player.countNumBuildingClasses(BuildingClass("BUILDINGCLASS_COURTHOUSE"));
		appendNewLine(&ss, sum, "from 1 per Courthouse", numCourthouses, numCourthouses > 0);
	}

	{ // 1 for 8 policies (free policies included)
		const int numPolicies = player.GetPlayerPolicies()->GetNumPoliciesOwned();
		appendNewLine(&ss, sum, "from 8 or more Social Policies", 1, player.GetNumPolicies() >= 8);
	}

	// ai get to build as many as they want
	if (!isHuman())
		*sum += 999;

	return ss.str();
}

