// mod extension methods for the CvPlayer

#include "CvGameCoreDLLPCH.h"
#include "CvPlayer.h"
#include "CvGameCoreUtils.h"

using namespace std;

// will construct a string
void appendNewLine(stringstream* ss, int* numCitiesAllowed, 
	// The description of where the numToAdd amount came from.
	string desc, 
	// The number of additional cities the player could add.
	int numToAdd)
{
	addColoredValue(*ss, numToAdd, desc, false);
	*numCitiesAllowed += numToAdd;
}

string CvPlayer::GetCityCap_WithTooltip(int* sum) const
{
	stringstream ss;
	const CvPlayer& player = *this;

	{ // the base amount to start with always
		appendNewLine(&ss, sum, "Base", 3);
	}

	// 1 for every other era, first unlock in classical
	{
		// ancient era is id 0, classical is id 1...
		const int Classical = (player.GetCurrentEra() >= 1); // evaluates to 0(false) or 1(true)
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_1}", Classical); // {} evaluates to Classical Era

		const int Renaissance = (player.GetCurrentEra() >= 3);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_3}", Renaissance);

		const int Modern = (player.GetCurrentEra() >= 5);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_5}", Modern);

		const int Future = (player.GetCurrentEra() >= 7);
		appendNewLine(&ss, sum, "from the {TXT_KEY_ERA_7}", Modern);
		// last era is 8 (future)
	}

	// localize
	CvString cvStr = ss.str().c_str();
	return GetLocalizedText(cvStr);
}

