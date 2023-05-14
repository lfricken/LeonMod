
#include <algorithm>
#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"

#include "cvStopWatch.h"
#include "CvUnitMission.h"

#include "CvDLLUtilDefines.h"
#include "CvAchievementUnlocker.h"

// interface uses
#include "ICvDLLUserInterface.h"
#include "CvEnumSerialization.h"
#include "FStlContainerSerialization.h"
#include "FAutoVariableBase.h"
#include "CvStringUtils.h"
#include "CvBarbarians.h"
#include "CvGoodyHuts.h"

#include <sstream>

#include "FTempHeap.h"
#include "CvDiplomacyRequests.h"

#include "CvDllPlot.h"
#include "FFileSystem.h"

#include "CvInfosSerializationHelper.h"
#include "CvCityManager.h"

const int chanceGameEndsAfterFutureTechIsDiscovered = 100;
const int chanceGameEndsBeforeThat = 0;




void addValue(stringstream& s, bool isGreen, const long value, const string description)
{
	const long absValue = abs(value);
	if (isGreen) s << "[COLOR_POSITIVE_TEXT]+" << absValue;
	else		 s << "[COLOR_GREY]+" << absValue;

	s << " " << description << "[ENDCOLOR]";
	s << "[NEWLINE]";
}
void appendTrophyLine(stringstream* ss, int* sumAmount, string desc, int numToAdd, bool isUnlocked)
{
	if (ss != NULL)
		addValue(*ss, isUnlocked, numToAdd, desc);

	if (isUnlocked)
		*sumAmount += numToAdd;
}
int CvGlobals::getTROPHY_PER_SCIENCE() const
{
	return 2;
}
int CvGlobals::getTROPHY_PER_TOURISM() const
{
	return 2;
}
int CvGlobals::getTROPHY_PER_DIPLOMATIC() const
{
	return 2;
}
int CvGlobals::getTROPHY_PER_UNITED_NATIONS() const
{
	return 2;
}
int CvGlobals::getTROPHY_PER_CAPITAL() const
{
	return 1;
}
int HasUnitedNationsTrophy(TeamTypes eTeam)
{
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes ePlayerLoop = (PlayerTypes)iPlayerLoop;
		if (GET_PLAYER(ePlayerLoop).getTeam() == eTeam)
		{
			if (GC.getGame().GetGameLeagues()->GetDiplomaticVictor() == ePlayerLoop)
			{
				return 1;
			}
		}
	}
	return 0;
}
// Has filled diplo track
int HasDiplomaticTrophy(TeamTypes eTeam)
{
	int totalTeamPoints = 0;
	int totalTeamPointsNeeded = 0;

	totalTeamPoints += GET_TEAM(eTeam).GetTotalDiplomaticInfluence();
	totalTeamPointsNeeded += GET_TEAM(eTeam).GetTotalDiplomaticInfluenceNeeded();

	// check if they won
	if (totalTeamPoints >= totalTeamPointsNeeded)
		return GC.getTROPHY_PER_DIPLOMATIC();
	return 0;
}
int HasTourismTrophy(TeamTypes eTeam)
{
	bool allHaveTourism = true;
	bool hasTeamMembers = false;
	// See if all players on this team have influential culture with all other players (still alive)
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		const CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
		if (kPlayer.isAlive())
		{
			if (kPlayer.getTeam() == eTeam)
			{
				hasTeamMembers = true;
				int numNeeded = GC.getGame().GetGameCulture()->GetNumCivsInfluentialForWin();
				bool hasEnoughInfluenced = kPlayer.GetCulture()->GetNumCivsInfluentialOn() >= numNeeded;
				if (!hasEnoughInfluenced)
				{
					allHaveTourism = false;
				}
			}
		}
	}
	if (hasTeamMembers && allHaveTourism)
		return GC.getTROPHY_PER_TOURISM();
	return 0;
}
bool HasScientificTrophy(TeamTypes eTeam)
{
	bool allHaveScience = true;
	bool hasTeamMembers = false;
	// every player on team must meet science goal
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		const PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
		const CvPlayer& player = GET_PLAYER(eLoopPlayer);
		if (player.isAlive())
		{
			if (player.getTeam() == eTeam)
			{
				hasTeamMembers = true;
				if (player.GetScientificInfluence() < player.GetScientificInfluenceNeeded())
				{
					allHaveScience = false;
				}
			}
		}
	}
	if (hasTeamMembers && allHaveScience)
		return true;
	return false;
}
// +1 per capital owned
int HasConquestTrophy(TeamTypes eTeam)
{
	int numCapitals = 0;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		const CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
		if (kPlayer.isAlive())
		{
			if (kPlayer.getTeam() == eTeam)
			{
				numCapitals += kPlayer.GetNumCapitals();
			}
		}
	}
	if (numCapitals > 0)
		return numCapitals * GC.getTROPHY_PER_CAPITAL();
	return 0;
}
int CvGame::GetTrophyPoints(string* tooltip) const
{
	int points = 0;
	stringstream* ss = NULL;
	if (tooltip != NULL)
	{
		ss = new stringstream();
	}


	delete ss;
	return points;
}
int CvTeam::GetTrophyPoints(string* tooltip) const
{
	int points = 0;
	stringstream* ss = NULL;
	if (tooltip != NULL)
	{
		ss = new stringstream();
	}

	*ss << "The number of {VICTORY_POINTS} you have. Earn enough (an unknown amount) before the "
			"game ends to achieve victory. Each player can win independently. Global sources of points benefit everyone.[NEWLINE]";
	*ss << "[NEWLINE]";

	// scientific insight
	appendTrophyLine(ss, &points, "from obtaining enough {SCIENTIFIC_INSIGHT}", GC.getTROPHY_PER_SCIENCE(), HasScientificTrophy(GetID()) > 0);
	// diplomatic support
	appendTrophyLine(ss, &points, "from earning enough {DIPLOMATIC_SUPPORT}", GC.getTROPHY_PER_DIPLOMATIC(), HasDiplomaticTrophy(GetID()) > 0);
	// cultural influence
	appendTrophyLine(ss, &points, "from exerting enough {CULTURAL_INFLUENCE}", GC.getTROPHY_PER_TOURISM(), HasTourismTrophy(GetID()) > 0);
	// conquest
	appendTrophyLine(ss, &points, "from 1 per [ICON_CAPITAL] Capital controlled", HasConquestTrophy(GetID()), HasConquestTrophy(GetID()) > 0);
	// un election
	appendTrophyLine(ss, &points, "from being elected [ICON_VICTORY_DIPLOMACY] World Leader", GC.getTROPHY_PER_UNITED_NATIONS(), HasUnitedNationsTrophy(GetID()) > 0);





	*tooltip += ss->str();
	delete ss;

	// include globals in this team
	points += GC.getGame().GetTrophyPoints(tooltip);
	return points;
}
bool CvTeam::HasEnoughTrophysToWin() const
{
	const int teamTrophies = GetTrophyPoints(NULL);

	// TODO dynamic points to win
	return (teamTrophies) > 10;
}



int chanceGameEndsThisTurnT100()
{
	// TODO
	// increase based on rockets launched
	// increase if international space station
	// instead should be based on some non tech stuff as well since last tech could be avoided via cheese

	if (GC.getGame().GetIsTechDiscovered(80)) // TECH_FUTURE_TECH
	{
		return chanceGameEndsAfterFutureTechIsDiscovered;
	}
	return chanceGameEndsBeforeThat;
}
bool shouldGameEnd()
{
	int chance = chanceGameEndsThisTurnT100();
	// TODO - better random seed
	int roll = GC.rand(100, "", NULL, 1);
	if (roll < chance)
	{
		return true;
	}
	return false;
}
struct TeamScore
{
	TeamScore(TeamTypes team, int trophyScore) : Team(team), TrophyScore(trophyScore) { }
	TeamTypes Team;
	int TrophyScore;
};
//bool compare(const TeamScore& lhs, const TeamScore& rhs)
//{
//	// randomly determine a tie
//	if (lhs.TrophyScore == rhs.TrophyScore)
//	{
//		unsigned long seed = 0;
//		seed += 345621594 * lhs.TrophyScore;
//		seed += 98615 * lhs.Team;
//		seed += 321891373 * rhs.TrophyScore;
//		seed += 96429789 * rhs.Team;
//		int randomTieResolution = GC.rand(1, "Winner Tie Resolution", NULL, seed);
//		return (bool)randomTieResolution;
//	}
//	else // higher values go earlier in the array
//		return lhs.TrophyScore > rhs.TrophyScore;
//}
void CvGame::doEndGame()
{
	// find first place
	//std::vector<TeamScore> scores;
	//for (int i = 0; i < MAX_TEAMS; ++i)
	//{
	//	TeamTypes eTeam = (TeamTypes)i;
	//	int trophies = GET_TEAM(eTeam).GetTrophyPoints(NULL);
	//	scores.push_back(TeamScore(eTeam, trophies));
	//}
	//sort(scores.begin(), scores.end(), compare);
	//TeamTypes trueWinnerTeam = scores[0].Team;


	showEndGameSequence();
	saveReplay();
}


void CvGame::checkIfGameShouldEnd()
{
	updateScore();
	bool hasGameAlreadyEnded = false; // TODO how do we check this?
	if (!hasGameAlreadyEnded && shouldGameEnd())
	{
		doEndGame();
	}
}

