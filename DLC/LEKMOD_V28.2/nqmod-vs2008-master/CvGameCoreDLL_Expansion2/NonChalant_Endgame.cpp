
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

// across the whole game, how much can points vary by
const int maxPointDeviation = 6;
const int chanceGameEndsAfterFutureTechIsDiscovered = 1 * 100; // 1%
const int chanceGameEndsEachRocket = 1 * 100; // 1%
const int chanceGameEndsAfterIss = 1 * 100; // 1%
const int chanceGameEndsOnTurn0 = 0;




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
	return 3;
}
int CvGlobals::getTROPHY_PER_TOURISM() const
{
	return 3;
}
int CvGlobals::getTROPHY_PER_DIPLOMATIC() const
{
	return 3;
}
int CvGlobals::getTROPHY_PER_UNITED_NATIONS() const
{
	return 2;
}
int CvGlobals::getTROPHY_PER_CAPITAL() const
{
	return 1;
}
int HasUnitedNationsTrophy(const CvPlayer& player)
{
	if (GC.getGame().GetGameLeagues()->GetDiplomaticVictor() == player.GetID())
	{
		return GC.getTROPHY_PER_UNITED_NATIONS();
	}
	return 0;
}
// Has filled diplo track
int HasDiplomaticTrophy(const CvPlayer& player)
{
	if (player.GetDiplomaticInfluence() >= player.GetDiplomaticInfluenceNeeded())
		return GC.getTROPHY_PER_DIPLOMATIC();
	return 0;
}
int HasTourismTrophy(const CvPlayer& player)
{
	const int numNeeded = GC.getGame().GetGameCulture()->GetNumCivsInfluentialForWin();
	const bool hasEnoughInfluenced = player.GetCulture()->GetNumCivsInfluentialOn() >= numNeeded;

	if (hasEnoughInfluenced)
		return GC.getTROPHY_PER_TOURISM();
	return 0;
}
int HasScientificTrophy(const CvPlayer& player)
{
	if (player.GetScientificInfluence() >= player.GetScientificInfluenceNeeded())
	{
		return GC.getTROPHY_PER_SCIENCE();
	}
	return 0;
}
// +1 per capital owned
int HasConquestTrophy(const CvPlayer& player)
{
	return player.GetNumCapitals() * GC.getTROPHY_PER_CAPITAL();
}
bool IsIssComplete()
{
	return GC.getGame().GetGameLeagues()->IsProjectComplete(LEAGUE_PROJECT_INTERNATIONAL_SPACE_STATION);
}
bool IsFairComplete()
{
	return GC.getGame().GetGameLeagues()->IsProjectComplete(LEAGUE_PROJECT_WORLD_FAIR);
}
bool IsGamesComplete()
{
	return GC.getGame().GetGameLeagues()->IsProjectComplete(LEAGUE_PROJECT_WORLD_GAMES);
}
int CvGame::GetTrophyPoints(string* tooltip) const
{
	int points = 0;
	stringstream* ss = NULL;
	if (tooltip != NULL)
	{
		ss = new stringstream();
	}
	if (ss)
	{
		*ss << "[NEWLINE]Global Sources:[NEWLINE]";
	}
	// space station
	appendTrophyLine(ss, &points, "from the {TXT_KEY_LEAGUE_PROJECT_INTERNATIONAL_SPACE_STATION}", +2, IsIssComplete());
	// world fair
	appendTrophyLine(ss, &points, "from the {TXT_KEY_LEAGUE_PROJECT_WORLD_FAIR}", +1, IsFairComplete());
	// world games
	appendTrophyLine(ss, &points, "from the {TXT_KEY_LEAGUE_PROJECT_WORLD_GAMES}", +1, IsGamesComplete());



	if (ss)
	{
		*tooltip += ss->str();
		delete ss;
	}
	return points;
}
int CvPlayer::GetTrophyPoints(string* tooltip) const
{
	int points = 0;
	stringstream* ss = NULL;
	if (tooltip != NULL)
	{
		ss = new stringstream();
	}
	if (ss)
	{
		*ss << "The number of {TROPHYS} you have. Earn enough (an unknown amount) before the "
			"game ends to achieve victory. Each player can win independently. Global sources of points benefit everyone.[NEWLINE]";
		*ss << "[NEWLINE]";
	}

	// conquest
	appendTrophyLine(ss, &points, "from 1 per [ICON_CAPITAL] Capital controlled", HasConquestTrophy(*this), HasConquestTrophy(*this) > 0);
	// scientific insight
	appendTrophyLine(ss, &points, "from obtaining enough {SCIENTIFIC_INSIGHT}", GC.getTROPHY_PER_SCIENCE(), HasScientificTrophy(*this) > 0);
	// diplomatic support
	appendTrophyLine(ss, &points, "from earning enough {DIPLOMATIC_SUPPORT}", GC.getTROPHY_PER_DIPLOMATIC(), HasDiplomaticTrophy(*this) > 0);
	// cultural influence
	appendTrophyLine(ss, &points, "from exerting enough {CULTURAL_INFLUENCE}", GC.getTROPHY_PER_TOURISM(), HasTourismTrophy(*this) > 0);
	// un election
	appendTrophyLine(ss, &points, "from being elected [ICON_VICTORY_DIPLOMACY] World Leader", GC.getTROPHY_PER_UNITED_NATIONS(), HasUnitedNationsTrophy(*this) > 0);

	// space ship launch
	appendTrophyLine(ss, &points, "from launching the Interstellar Space Ship", +2, m_accomplishCount[ACCOMPLISH_LAUNCH_SPACESHIP] > 0);

	// buildings that give trophys
	for (int i = 0; i < GC.getNumBuildingInfos(); ++i)
	{
		const CvBuildingEntry* info = GC.getBuildingInfo((BuildingTypes)i);
		if (info != NULL)
		{
			const int numTrophys = info->GetTrophys();
			if (numTrophys > 0)
			{
				const int amount = countNumBuildings((BuildingTypes)i);
				appendTrophyLine(ss, &points, "from " + (string)info->GetDescription(), numTrophys, amount > 0);
			}
		}
	}


	if (ss)
	{
		*tooltip += ss->str();
		delete ss;
	}

	// include globals in this team
	points += GC.getGame().GetTrophyPoints(tooltip);
	return points;
}
int CvPlayer::getWinScreen() const
{
	enum // found experimentally in EndGameMenu.lua
	{
		VICTORYSCREEN_SPACE,
		VICTORYSCREEN_CULTURE,
		VICTORYSCREEN_DOMINATION,
		VICTORYSCREEN_DIPLO,
		VICTORYSCREEN_TIME,
	};
	//const CvTeam& team = GET_TEAM(getTeam());

	std::vector<int> possibilities;
	if (IsIssComplete() || m_accomplishCount[ACCOMPLISH_LAUNCH_SPACESHIP] > 0)
	{
		possibilities.push_back(VICTORYSCREEN_SPACE);
	}
	if (IsGamesComplete() || HasTourismTrophy(*this))
	{
		possibilities.push_back(VICTORYSCREEN_CULTURE);
	}
	if (HasConquestTrophy(*this) > 1) // owns at least one other capital
	{
		possibilities.push_back(VICTORYSCREEN_DOMINATION);
	}
	if (IsFairComplete() || HasDiplomaticTrophy(*this) || HasUnitedNationsTrophy(*this))
	{
		possibilities.push_back(VICTORYSCREEN_DIPLO);
	}

	if (possibilities.size() < 2)
	{
		possibilities.push_back(VICTORYSCREEN_TIME);
	}


	const int choice = GC.rand(possibilities.size(), "Victory Screen selection", NULL, GC.getFakeSeed(GC.getGame().turn(), GetTreasury()->GetGold(), 234));
	return possibilities[choice];
}
int pointsFromTurn(long turnPercentT10000)
{
	const long effectiveOnlineSpeedTurnT1000 = (turnPercentT10000 + 5) / 10; // 5 to round

	// exactly 20 points when at 100% game done
	const int basePoints = (effectiveOnlineSpeedTurnT1000 * effectiveOnlineSpeedTurnT1000) / 50000;
	return basePoints;
}
int getPointsToWin(const CvPlayer& player)
{
	const int difficultyPoints = player.getHandicapInfo().getAdvancedStartPointsMod();
	const int turnBased = pointsFromTurn(GC.getPercentTurnsDoneT10000());
	return turnBased + difficultyPoints + GC.getGame().m_randomTrophyPointDelta;
}
bool CvTeam::HasEnoughTrophysToWin() const
{
	int teamTrophies = 0;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		teamTrophies += (GET_PLAYER((PlayerTypes)i)).GetTrophyPoints(NULL);
	}

	// every player on team must meet science goal
	int pointsNeeded = 0;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		const PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
		const CvPlayer& player = GET_PLAYER(eLoopPlayer);
		if (player.getTeam() == GetID())
		{
			pointsNeeded += getPointsToWin(player);
		}
	}

	return (teamTrophies) >= pointsNeeded;
}
int chanceGameEndsThisTurnT100()
{
	// TODO
	// increase based on rockets launched
	// increase if international space station
	// instead should be based on some non tech stuff as well since last tech could be avoided via cheese

	int chanceT10000 = chanceGameEndsOnTurn0;

	// TECH_FUTURE_TECH
	if (GC.getGame().GetIsTechDiscovered(TECH_FUTURE_TECH))
	{
		chanceT10000 += chanceGameEndsAfterFutureTechIsDiscovered;
	}

	// rocket launches
	int numRocketLaunches = 0;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		const CvPlayer& player = GET_PLAYER((PlayerTypes)i);
		numRocketLaunches += player.m_accomplishCount[ACCOMPLISH_LAUNCH_SPACESHIP];
	}
	chanceT10000 += numRocketLaunches * chanceGameEndsEachRocket;

	// ISS
	if (IsIssComplete())
	{
		chanceT10000 += chanceGameEndsAfterIss;
	}

	return chanceT10000;
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
	// on certain turns, randomly adjust necessary points to win
	const int everyXTurns = GC.getGame().getMaxTurns() / (maxPointDeviation + 1);
	const bool shouldApplyRandomPointThisTurn = GC.getGame().turn() % everyXTurns == 0;
	if (shouldApplyRandomPointThisTurn)
	{
		const int rand = GC.getGame().getJonRandUnsafe().get(2, CvRandom::MutateSeed, "random trophys");
		const int points = rand == 0 ? +1 : -1;
		m_randomTrophyPointDelta += points;
	}


	updateScore();
	if (!m_hasEndgameShown && shouldGameEnd())
	{
		doEndGame();
	}
}

