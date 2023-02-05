

#include "CvGameCoreDLLPCH.h"
#include "CvGameCoreUtils.h"
#include "CvInternalGameCoreUtils.h"
#include "CvGame.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeam.h"
#include "CvGlobals.h"
#include "CvMapGenerator.h"
#include "CvReplayMessage.h"
#include "CvInfos.h"
#include "CvReplayInfo.h"
#include "CvGameTextMgr.h"
#include "CvSiteEvaluationClasses.h"
#include "CvImprovementClasses.h"
#include "CvStartPositioner.h"
#include "CvTacticalAnalysisMap.h"
#include "CvGrandStrategyAI.h"
#include "CvMinorCivAI.h"
#include "CvDiplomacyAI.h"
#include "CvNotifications.h"
#include "CvAdvisorCounsel.h"
#include "CvAdvisorRecommender.h"
#include "CvWorldBuilderMapLoader.h"
#include "CvTypes.h"

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
int HasUnitedNationsTrophy(TeamTypes eTeam)
{
	return 0;
}
int HasDiplomaticTrophy(TeamTypes eTeam)
{
	return 0;
}
int HasTourismTrophy(TeamTypes eTeam)
{
	return 0;
}
int HasScientificTrophy(TeamTypes eTeam)
{
	return 0;
}
bool HasConquestTrophy(TeamTypes eTeam)
{
	return 0;
}
int CvTeam::GetTrophyPoints(string* tooltip) const
{
	int points = 0;
	stringstream* pSs = NULL;
	if (tooltip != NULL)
	{
		pSs = new stringstream();
	}


	delete pSs;
	return points;
}



int chanceGameEndsThisTurnT100()
{
	if (GC.getGame().GetIsTechDiscovered(80)) // TECH_FUTURE_TECH
	{
		return 100;
	}
	return 0;
}
bool shouldGameEnd()
{
	int chance = chanceGameEndsThisTurnT100();
	int roll = GC.rand(99, "", NULL, 1);
	if (roll < chance)
	{
		return true;
	}
	return false;
}


void CvGame::checkIfGameShouldEnd()
{
	bool bEndScore = false;

	// Send a game event to allow a Lua script to set the victory state
	ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
	if (pkScriptSystem)
	{
		CvLuaArgsHandle args;
		bool bResult;
		LuaSupport::CallHook(pkScriptSystem, "GameCoreTestVictory", args.get(), bResult);
	}

	// allow victory after victory
	//if(getVictory() != NO_VICTORY)
	//{
	//	return;
	//}
	//if(getGameState() == GAMESTATE_EXTENDED)
	//{
	//	return;
	//}

	updateScore();
	bool hasGameAlreadyEnded = false; // TODO
	if (!hasGameAlreadyEnded && shouldGameEnd())
	{
		//setWinner((TeamTypes)1, (VictoryTypes)2);
		//setGameState(GAMESTATE_EXTENDED);
		showEndGameSequence();
	}
}
bool CvGame::didTeamAcheiveVictory(const VictoryTypes eVictory, const TeamTypes eTeam, bool* pbEndScore) const
{
	CvAssert(eVictory >= 0 && eVictory < GC.getNumVictoryInfos());
	CvAssert(eTeam >= 0 && eTeam < MAX_CIV_TEAMS);
	CvAssert(GET_TEAM(eTeam).isAlive());

	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
	if (pkVictoryInfo == NULL)
	{
		return false;
	}

	// Has the player already achieved this victory?
	if (GET_TEAM(eTeam).isVictoryAchieved(eVictory))
	{
		return false;
	}

	bool bValid = isVictoryValid(eVictory);
	if (pbEndScore)
	{
		*pbEndScore = false;
	}

	// Can't end the game unless a certain number of turns has already passed (ignore this on Debug Micro Map because it's only for testing)
	if (getElapsedGameTurns() <= /*10*/ GC.getMIN_GAME_TURNS_ELAPSED_TO_TEST_VICTORY() && (GC.getMap().getWorldSize() != WORLDSIZE_DEBUG))
	{
		return false;
	}

	// End Score
	if (bValid)
	{
		if (pkVictoryInfo->isEndScore())
		{
			if (pbEndScore)
			{
				*pbEndScore = true;
			}

			if (getMaxTurns() == 0)
			{
				bValid = false;
			}
			else if (getElapsedGameTurns() < getMaxTurns())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam)
						{
							if (getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Target Score
	if (bValid)
	{
		if (pkVictoryInfo->isTargetScore())
		{
			if (getTargetScore() == 0)
			{
				bValid = false;
			}
			else if (getTeamScore(eTeam) < getTargetScore())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam)
						{
							if (getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Conquest
	if (bValid)
	{
		if (pkVictoryInfo->isConquest())
		{
			if (GET_TEAM(eTeam).getNumCities() == 0)
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam)
						{
							if (GET_TEAM((TeamTypes)iK).getNumCities() > 0)
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Diplomacy Victory
	if (bValid)
	{
		if (pkVictoryInfo->isDiploVote())
		{
			bValid = false;
			// if any player won, award this victory
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				PlayerTypes ePlayerLoop = (PlayerTypes)iPlayerLoop;
				if (GET_PLAYER(ePlayerLoop).getTeam() == eTeam)
				{
					if (m_pGameLeagues->GetDiplomaticVictor() == ePlayerLoop)
					{
						bValid = true;
					}
				}
			}

			{ // check to see if they've accumulated enough diplomatic influence
				int totalTeamPoints = 0;
				int totalTeamPointsNeeded = 0;

				totalTeamPoints += GET_TEAM(eTeam).GetTotalDiplomaticInfluence();
				totalTeamPointsNeeded += GET_TEAM(eTeam).GetTotalDiplomaticInfluenceNeeded();

				// check if they won
				if (totalTeamPoints >= totalTeamPointsNeeded)
					bValid = true; // mark to win
			}
		}
	}

	// Culture victory
	if (bValid)
	{
		if (pkVictoryInfo->isInfluential())
		{
			// See if all players on this team have influential culture with all other players (still alive)
			bValid = false;
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
				if (kPlayer.isAlive())
				{
					if (kPlayer.getTeam() == eTeam)
					{
						if (kPlayer.GetCulture()->GetNumCivsInfluentialOn() >= m_pGameCulture->GetNumCivsInfluentialForWin())
						{
							// Not enough civs for a win
							bValid = true;
							break;
						}
					}
				}
			}
		}
	}

	// Religion in all Cities
	if (bValid)
	{
		if (pkVictoryInfo->IsReligionInAllCities())
		{
			bool bReligionInAllCities = true;

			CvCity* pLoopCity;
			int iLoop;

			PlayerTypes eLoopPlayer;

			// See if all players on this team have their State Religion in their Cities
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				eLoopPlayer = (PlayerTypes)iPlayerLoop;

				if (GET_PLAYER(eLoopPlayer).isAlive())
				{
					if (GET_PLAYER(eLoopPlayer).getTeam() == eTeam)
					{
						for (pLoopCity = GET_PLAYER(eLoopPlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eLoopPlayer).nextCity(&iLoop))
						{
							// Any Cities WITHOUT State Religion?
							if (pLoopCity->GetPlayersReligion() != eLoopPlayer)
							{
								bReligionInAllCities = false;
								break;
							}
						}

						if (!bReligionInAllCities)
						{
							break;
						}
					}
				}
			}

			if (!bReligionInAllCities)
			{
				bValid = false;
			}
		}
	}

	// FindAllNaturalWonders
	if (bValid)
	{
		if (pkVictoryInfo->IsFindAllNaturalWonders())
		{
			int iWorldNumNaturalWonders = GC.getMap().GetNumNaturalWonders();

			if (iWorldNumNaturalWonders == 0 || GET_TEAM(eTeam).GetNumNaturalWondersDiscovered() < iWorldNumNaturalWonders)
			{
				bValid = false;
			}
		}
	}

	// Population Percent
	if (bValid)
	{
		if (getAdjustedPopulationPercent(eVictory) > 0)
		{
			if (100 * GET_TEAM(eTeam).getTotalPopulation() < getTotalPopulation() * getAdjustedPopulationPercent(eVictory))
			{
				bValid = false;
			}
		}
	}

	// Land Percent
	if (bValid)
	{
		if (getAdjustedLandPercent(eVictory) > 0)
		{
#ifdef AUI_WARNING_FIXES
			if (100 * GET_TEAM(eTeam).getTotalLand() < (int)GC.getMap().getLandPlots() * getAdjustedLandPercent(eVictory))
#else
			if (100 * GET_TEAM(eTeam).getTotalLand() < GC.getMap().getLandPlots() * getAdjustedLandPercent(eVictory))
#endif
			{
				bValid = false;
		}
	}
}

	// Buildings
	if (bValid)
	{
#ifdef AUI_WARNING_FIXES
		for (uint iK = 0; iK < GC.getNumBuildingClassInfos(); iK++)
#else
		for (int iK = 0; iK < GC.getNumBuildingClassInfos(); iK++)
#endif
		{
			BuildingClassTypes eBuildingClass = static_cast<BuildingClassTypes>(iK);
			CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo(eBuildingClass);
			if (pkBuildingClassInfo)
			{
				if (pkBuildingClassInfo->getVictoryThreshold(eVictory) > GET_TEAM(eTeam).getBuildingClassCount(eBuildingClass))
				{
					bValid = false;
					break;
				}
			}

	}
	}

	// Projects
	if (bValid)
	{
		if (pkVictoryInfo->GetID() != 1) // disallow space race rocket ship star ship
			for (int iK = 0; iK < GC.getNumProjectInfos(); iK++)
			{
				const ProjectTypes eProject = static_cast<ProjectTypes>(iK);
				CvProjectEntry* pkProjectInfo = GC.getProjectInfo(eProject);
				if (pkProjectInfo)
				{
					if (pkProjectInfo->GetVictoryMinThreshold(eVictory) > GET_TEAM(eTeam).getProjectCount(eProject))
					{
						bValid = false;
						break;
					}
				}
			}
	}

	// science
	if (bValid)
	{
		if (pkVictoryInfo->GetID() == 1) // space race rocket ship star ship
		{
			// every player on team must meet science goal
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				const PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
				const CvPlayerAI& player = GET_PLAYER(eLoopPlayer);
				if (player.isAlive())
				{
					if (player.getTeam() == eTeam)
					{
						if (player.GetScientificInfluence() < player.GetScientificInfluenceNeeded())
						{
							bValid = false; // not enough science
							break;
						}
					}
				}
			}
		}
	}

	return bValid;
}
