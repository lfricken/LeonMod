#include "CvGameCoreDLLPCH.h"
#include "CvPlot.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvArea.h"
#include "ICvDLLUserInterface.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvTeam.h"
#include "CvGameCoreUtils.h"
#include "CvRandom.h"
#include "CvInfos.h"
#include "CvImprovementClasses.h"
#include "CvAStar.h"
#include "CvEconomicAI.h"
#include "CvEnumSerialization.h"
#include "CvNotifications.h"
#include "CvMinorCivAI.h"
#include "CvUnitCombat.h"
#include "CvDLLUtilDefines.h"
#include "CvInfosSerializationHelper.h"
#include "CvBarbarians.h"

#include "CvDllPlot.h"
#include "CvGameCoreEnums.h"
#include "CvDllUnit.h"
#include "CvUnitMovement.h"
#include "CvTargeting.h"
#include "CvTypes.h"

// Include this after all other headers.
#include "LintFree.h"


int CvGlobals::getTOURISM_MODIFIER_HAMMERCOMPETITION(const PlayerTypes) const
{
	int base = 10;
	// always have some base, possibly modify based on player
	return base;
}
int CvGlobals::getCITIZENS_MIN_FOR_SPECIALIST(const PlayerTypes) const
{
	return 2;
}
int CvGlobals::getCITIZENS_PER_SPECIALIST(const PlayerTypes) const
{
	return 2;
}
int CvGlobals::getGREAT_WORKS_PER_DIG(const PlayerTypes) const
{
	return 1;
}
int CvGlobals::getYIELD_PER_TURN_ALLY(const YieldTypes eYieldType, const PlayerTypes, const PlayerTypes ePlayer, const bool) const
{
	int value = 0;
	// defaults
	if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		value = 10;

	// player specific changes
	if (ePlayer != NO_PLAYER)
	{
		const CvPlayer& player = GET_PLAYER(ePlayer);
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			const bool hasPatronageFinisher = player.HasPolicy(POLICY_PATRONAGE_FINISHER);
			if (hasPatronageFinisher)
				value += 5;
		}

		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			const bool hasGestapoCard = player.HasPolicy(POLICY_CARD_MODERN_BUILDINGS_GESTAPO_PASSIVE);
			const bool hasNationalIntelligenceAgency = player.HasWonder(BUILDINGCLASS_INTELLIGENCE_AGENCY);
			if (hasGestapoCard && hasNationalIntelligenceAgency)
				value += 10;
		}
	}

	return value;
}
int CvGlobals::getYIELD_PER_QUEST(const YieldTypes eYieldType, const PlayerTypes, const PlayerTypes ePlayer) const
{
	int value = 0;
	// defaults
	if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		value = 100;
	if (eYieldType == YIELD_FRIENDSHIP)
		value = GC.getMINOR_FRIENDSHIP_FROM_TRADE_MISSION(); // 40

	// player specific changes
	if (ePlayer != NO_PLAYER)
	{
		const CvPlayer& player = GET_PLAYER(ePlayer);
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			//POLICY_PATRONAGE_FINISHER grants 50% more Diplo Points from Completing Quests
			const bool hasPhilanthropy = player.HasPolicy(POLICY_PATRONAGE_FINISHER);
			if (hasPhilanthropy)
			{
				value *= 150;
				value /= 100;
			}
		}
		if (eYieldType == YIELD_DIPLOMATIC_SUPPORT)
		{
			//PENTAGON grants 50% more Diplo Points from Completing Quests
			const bool hasPentagon = player.HasWonder(BUILDINGCLASS_PENTAGON);
			if (hasPentagon)
			{
				value *= 150;
				value /= 100;
			}
		}
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT)
		{
			//POLICY_PATRONAGE_FINISHER grants +15 Insight from Completing Quests
			const bool hasPhilanthropy = player.HasPolicy(POLICY_PATRONAGE_FINISHER);
			if (hasPhilanthropy)
				value += 15;
		}
		if (eYieldType == YIELD_SCIENTIFIC_INSIGHT)
		{
			//PENTAGON grants +15 Insight from Completing Quests
			const bool hasPentagon = player.HasWonder(BUILDINGCLASS_PENTAGON);
			if (hasPentagon)
				value += 15;
		}
		if (eYieldType == YIELD_GOLD)
		{
			// POLICY_PATRONAGE grants 100G from City-State Quests
			const bool hasPatronageOpener = player.HasPolicy(POLICY_PATRONAGE);
			if (hasPatronageOpener)
				value += 100;
		}
	}

	return value;
}


// modify unit instapop



void CvAssertFunc(const stringstream& msg)
{
	stringstream fileName;
	fileName << "asserts2.log";
	FILogFile* pLog = LOGFILEMGR.GetLog(fileName.str().c_str(), 0);
	if (pLog)
	{
		pLog->DebugMsg(msg.str().c_str());
	}
}



