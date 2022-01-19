/*	-------------------------------------------------------------------------------------------------------
	� 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#include "CvGameCoreDLLPCH.h"
#include "CvUnit.h"
#include "CvUnitCombat.h"
#include "CvUnitMission.h"
#include "CvGameCoreUtils.h"
#include "ICvDLLUserInterface.h"
#include "CvDiplomacyAI.h"
#include "CvTypes.h"

#include "CvDllCity.h"
#include "CvDllUnit.h"
#include "CvDllCombatInfo.h"

// include this after all other headers
#include "LintFree.h"

// Maximum damage members for the nuke, units and cities
#define MAX_NUKE_DAMAGE_MEMBERS	64

#define POST_QUICK_COMBAT_DELAY	110
#define POST_COMBAT_DELAY		1

//	---------------------------------------------------------------------------
static int GetPostCombatDelay()
{
	return CvPreGame::quickCombat()?POST_QUICK_COMBAT_DELAY:POST_COMBAT_DELAY;
}

CvUnit* getMissileInterceptor(CvUnit& kAttacker, CvPlot& plot)
{
	CvUnit* interceptor = NULL;

	// todo check via upgrade
	UnitTypes them = kAttacker.getUnitType();
	UnitTypes missile = (UnitTypes)GC.getInfoTypeForString("UNIT_GUIDED_MISSILE");
	UnitTypes nuke = (UnitTypes)GC.getInfoTypeForString("UNIT_NUCLEAR_MISSILE");
	UnitTypes icbm = (UnitTypes)GC.getInfoTypeForString("UNIT_ICBM");
	bool isMissile = (them == missile || them == nuke || them == icbm);

	if (isMissile)
	{
		interceptor = kAttacker.WasMissileIntercepted(plot);
	}
	return interceptor;
}

//	---------------------------------------------------------------------------
// Find a object in the combat member array
static CvCombatMemberEntry* FindCombatMember(CvCombatMemberEntry* pkArray, int iMembers, IDInfo kMember, CvCombatMemberEntry::MEMBER_TYPE eType)
{
	if(iMembers > 0)
	{
		while(iMembers--)
		{
			if(pkArray->IsType(eType))
			{
				if(pkArray->GetID() == kMember.iID && pkArray->GetPlayer() == kMember.eOwner)
					return pkArray;
			}
			++pkArray;
		}
	}

	return NULL;
}

//	---------------------------------------------------------------------------
// Add a member to the combat array
static CvCombatMemberEntry* AddCombatMember(CvCombatMemberEntry* pkArray, int* piMembers, int iMaxMembers, IDInfo kMember, CvCombatMemberEntry::MEMBER_TYPE eType, int iX, int iY, EraTypes eEra)
{
	if(*piMembers < iMaxMembers)
	{
		int iCount = *piMembers;
		if(!FindCombatMember(pkArray, iCount, kMember, eType))
		{
			CvCombatMemberEntry& kEntry = pkArray[iCount];
			kEntry.SetPlayer(kMember.eOwner);
			kEntry.SetID(kMember.iID, eType);
			kEntry.SetDamage(0);
			kEntry.SetFinalDamage(0);
			kEntry.SetMaxHitPoints(0);
			kEntry.SetPlot(iX, iY);
			kEntry.SetEra(eEra);
			*piMembers += 1;
			return &(pkArray[iCount]);
		}
	}
	return NULL;
}

//	---------------------------------------------------------------------------
// Add a unit member to the combat array
static CvCombatMemberEntry* AddCombatMember(CvCombatMemberEntry* pkArray, int* piMembers, int iMaxMembers, CvUnit* pkMember)
{
	if(pkMember)
		return AddCombatMember(pkArray, piMembers, iMaxMembers, pkMember->GetIDInfo(), CvCombatMemberEntry::MEMBER_UNIT, pkMember->getX(), pkMember->getY(), GET_PLAYER(pkMember->getOwner()).GetCurrentEra());

	return NULL;
}

//	---------------------------------------------------------------------------
// Add a city member to the combat array
static CvCombatMemberEntry* AddCombatMember(CvCombatMemberEntry* pkArray, int* piMembers, int iMaxMembers, CvCity* pkMember)
{
	if(pkMember)
		return AddCombatMember(pkArray, piMembers, iMaxMembers, pkMember->GetIDInfo(), CvCombatMemberEntry::MEMBER_CITY, pkMember->getX(), pkMember->getY(), GET_PLAYER(pkMember->getOwner()).GetCurrentEra());

	return NULL;
}

//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateMeleeCombatInfo(CvUnit& kAttacker, CvUnit* pkDefender, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	int iMaxHP = GC.getMAX_HIT_POINTS();

	pkCombatInfo->setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, pkDefender);
	pkCombatInfo->setPlot(&plot);

	// Attacking a City
	if(plot.isCity())
	{
		// Unit vs. City (non-ranged so the city will retaliate
		CvCity* pkCity = plot.getPlotCity();
		int iMaxCityHP = pkCity->GetMaxHitPoints();

		int iAttackerStrength = kAttacker.GetMaxAttackStrength(kAttacker.plot(), &plot, NULL);
		int iDefenderStrength = pkCity->getStrengthValue();

		int iAttackerDamageInflicted = kAttacker.getCombatDamage(iAttackerStrength, iDefenderStrength, kAttacker.getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ true);
		int iDefenderDamageInflicted = kAttacker.getCombatDamage(iDefenderStrength, iAttackerStrength, pkCity->getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ true, /*bDefenderIsCity*/ false);

		int iAttackerTotalDamageInflicted = iAttackerDamageInflicted + pkCity->getDamage();
		int iDefenderTotalDamageInflicted = iDefenderDamageInflicted + kAttacker.getDamage();

		// Will both the attacker die, and the city fall? If so, the unit wins
		if(iAttackerTotalDamageInflicted >= iMaxCityHP && iDefenderTotalDamageInflicted >= iMaxHP)
		{
			iDefenderDamageInflicted = iMaxHP - kAttacker.getDamage() - 1;
			iDefenderTotalDamageInflicted = iMaxHP - 1;
		}

		pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iDefenderTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iAttackerDamageInflicted);
		pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iAttackerTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDefenderDamageInflicted);
		
		int iExperience = /*5*/ GC.getEXPERIENCE_ATTACKING_CITY_MELEE();

		pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, iExperience);
		int iMaxExperience = (GET_PLAYER(pkCity->getOwner()).isMinorCiv()) ? 30 : MAX_INT; // NQMP GJS - cap XP from fighting CS to 30
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, iMaxExperience);
		pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == pkCity->getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() && !pkCity->isBarbarian() && !GET_PLAYER(pkCity->getOwner()).isMinorCiv();
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

		pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, 0);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, kAttacker.maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !pkCity->isBarbarian());
#endif

		pkCombatInfo->setAttackIsRanged(false);
		pkCombatInfo->setDefenderRetaliates(true);
	}
	// Attacking a Unit
	else
	{
		// Unit vs. Unit
		CvAssert(pkDefender != NULL);

#ifdef NQ_HEAVY_CHARGE_DOWNHILL
		bool isAttackingFromHigherElevation;
#endif
		int iDefenderStrength = pkDefender->GetMaxDefenseStrength(&plot, &kAttacker);
		int iAttackerStrength = 0;
		if(kAttacker.GetMaxRangedCombatStrength(NULL, /*pCity*/ NULL, true, true) > 0 && kAttacker.getDomainType() == DOMAIN_AIR)
		{
			iAttackerStrength = kAttacker.GetMaxRangedCombatStrength(NULL, /*pCity*/ NULL, true, true);
			if(pkDefender->getDomainType() != DOMAIN_AIR)
			{
				iDefenderStrength /= 2;
			}
		}
		else
		{
			iAttackerStrength = kAttacker.GetMaxAttackStrength(kAttacker.plot(), &plot, pkDefender);
		}

#ifndef AUI_UNIT_FIX_HEAVY_CHARGE_BONUS_INTEGRATED_INTO_STACKS
#ifdef NQ_HEAVY_CHARGE_DOWNHILL
		isAttackingFromHigherElevation =
			(kAttacker.plot()->isMountain() && !pkDefender->plot()->isMountain()) ||
			(kAttacker.plot()->isHills() && pkDefender->plot()->isFlatlands());
		
		if ((kAttacker.IsCanHeavyCharge() || (kAttacker.GetHeavyChargeDownhill() > 0 && isAttackingFromHigherElevation))
			&& !pkDefender->CanFallBackFromMelee(kAttacker))
		{
			iAttackerStrength = (iAttackerStrength * 150) / 100;
		}
#else
		if (kAttacker.IsCanHeavyCharge() && !pkDefender->CanFallBackFromMelee(kAttacker))
		{
			iAttackerStrength = (iAttackerStrength * 150) / 100;
		}
#endif
#endif

		int iAttackerDamageInflicted = kAttacker.getCombatDamage(iAttackerStrength, iDefenderStrength, kAttacker.getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ false);
		int iDefenderDamageInflicted = pkDefender->getCombatDamage(iDefenderStrength, iAttackerStrength, pkDefender->getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ false);

		if(kAttacker.getDomainType() == DOMAIN_AIR && pkDefender->getDomainType() != DOMAIN_AIR)
		{
			iAttackerDamageInflicted /= 2;
			iDefenderDamageInflicted /= 3;
		}

		int iAttackerTotalDamageInflicted = iAttackerDamageInflicted + pkDefender->getDamage();
		int iDefenderTotalDamageInflicted = iDefenderDamageInflicted + kAttacker.getDamage();

		// Will both units be killed by this? :o If so, take drastic corrective measures
		if(iAttackerTotalDamageInflicted >= iMaxHP && iDefenderTotalDamageInflicted >= iMaxHP)
		{
			// He who hath the least amount of damage survives with 1 HP left
			if(iAttackerTotalDamageInflicted > iDefenderTotalDamageInflicted)
			{
				iDefenderDamageInflicted = iMaxHP - kAttacker.getDamage() - 1;
				iDefenderTotalDamageInflicted = iMaxHP - 1;
				iAttackerTotalDamageInflicted = iMaxHP;
			}
			else
			{
				iAttackerDamageInflicted = iMaxHP - pkDefender->getDamage() - 1;
				iAttackerTotalDamageInflicted = iMaxHP - 1;
				iDefenderTotalDamageInflicted = iMaxHP;
			}
		}

		pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iDefenderTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iAttackerDamageInflicted);
		pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iAttackerTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDefenderDamageInflicted);

		// Fear Damage
		pkCombatInfo->setFearDamageInflicted(BATTLE_UNIT_ATTACKER, kAttacker.getCombatDamage(iAttackerStrength, iDefenderStrength, kAttacker.getDamage(), true, false, true));
		//	pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_DEFENDER, getCombatDamage(iDefenderStrength, iAttackerStrength, pDefender->getDamage(), true, false, true) );

#ifndef NQM_PRUNING
		int iAttackerEffectiveStrength = iAttackerStrength * (iMaxHP - range(kAttacker.getDamage(), 0, iMaxHP-1)) / iMaxHP;
		iAttackerEffectiveStrength = iAttackerEffectiveStrength > 0 ? iAttackerEffectiveStrength : 1;
		int iDefenderEffectiveStrength = iDefenderStrength * (iMaxHP - range(pkDefender->getDamage(), 0, iMaxHP-1)) / iMaxHP;
		iDefenderEffectiveStrength = iDefenderEffectiveStrength > 0 ? iDefenderEffectiveStrength : 1;
#endif

		//int iExperience = kAttacker.defenseXPValue();
		//iExperience = ((iExperience * iAttackerEffectiveStrength) / iDefenderEffectiveStrength); // is this right? looks like more for less [Jon: Yes, it's XP for the defender]
		//iExperience = range(iExperience, GC.getMIN_EXPERIENCE_PER_COMBAT(), GC.getMAX_EXPERIENCE_PER_COMBAT());
		int iExperience = /*4*/ GC.getEXPERIENCE_DEFENDING_UNIT_MELEE();
		pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iExperience);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, kAttacker.maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == pkDefender->getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() && !pkDefender->isBarbarian() && !GET_PLAYER(pkDefender->getOwner()).isMinorCiv();
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !kAttacker.isBarbarian());
#endif

		//iExperience = ((iExperience * iDefenderEffectiveStrength) / iAttackerEffectiveStrength);
		//iExperience = range(iExperience, GC.getMIN_EXPERIENCE_PER_COMBAT(), GC.getMAX_EXPERIENCE_PER_COMBAT());
		iExperience = /*6*/ GC.getEXPERIENCE_ATTACKING_UNIT_MELEE();
		pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, iExperience);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, pkDefender->maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !pkDefender->isBarbarian());
#endif

		pkCombatInfo->setAttackIsRanged(false);

#ifdef NQ_HEAVY_CHARGE_DOWNHILL
		isAttackingFromHigherElevation = false;
		if (kAttacker.GetHeavyChargeDownhill() > 0)
		{
			isAttackingFromHigherElevation = 
				(kAttacker.plot()->isMountain() && !pkDefender->plot()->isMountain()) ||
				(kAttacker.plot()->isHills() && pkDefender->plot()->isFlatlands());
		}
#endif
		bool bAdvance = true;
		if(plot.getNumDefenders(pkDefender->getOwner()) > 1)
		{
			bAdvance = false;
		}
		else if(iAttackerTotalDamageInflicted >= iMaxHP && kAttacker.IsCaptureDefeatedEnemy() && kAttacker.AreUnitsOfSameType(*pkDefender))
		{
			int iCaptureRoll = GC.getGame().getJonRandNum(100, "Capture Enemy Roll");

			if (iCaptureRoll < kAttacker.GetCaptureChance(pkDefender))
			{
				bAdvance = false;
				pkCombatInfo->setDefenderCaptured(true);
			}
		}
#ifdef NQ_HEAVY_CHARGE_DOWNHILL
		else if ((kAttacker.IsCanHeavyCharge() || (kAttacker.GetHeavyChargeDownhill() > 0 && isAttackingFromHigherElevation))
			&& !pkDefender->isDelayedDeath() && (iAttackerDamageInflicted > iDefenderDamageInflicted) )
#else
		else if (kAttacker.IsCanHeavyCharge() && !pkDefender->isDelayedDeath() && (iAttackerDamageInflicted > iDefenderDamageInflicted) )
#endif
		{
			bAdvance = true;
		}

		pkCombatInfo->setAttackerAdvances(bAdvance);
		pkCombatInfo->setDefenderRetaliates(true);
	}

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}

//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveMeleeCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	// After combat stuff
	CvString strBuffer;
	bool bAttackerDead = false;
	bool bDefenderDead = false;
	int iAttackerDamageDelta = 0;

	CvUnit* pkAttacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);
	CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	if(!pkTargetPlot && pkDefender)
		pkTargetPlot = pkDefender->plot();

	CvAssert_Debug(pkAttacker && pkDefender && pkTargetPlot);

	int iActivePlayerID = GC.getGame().getActivePlayer();

	bool bAttackerDidMoreDamage = false;

	if(pkAttacker != NULL && pkDefender != NULL && pkTargetPlot != NULL &&
	        pkDefender->IsCanDefend()) 		// Did the defender actually defend?
	{
		// Internal variables
		int iAttackerDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
		int iDefenderDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);
		int iAttackerFearDamageInflicted = 0;//pInfo->getFearDamageInflicted( BATTLE_UNIT_ATTACKER );

		bAttackerDidMoreDamage = iAttackerDamageInflicted > iDefenderDamageInflicted;

		//One Hit
		if(pkDefender->GetCurrHitPoints() == GC.getMAX_HIT_POINTS() && iAttackerDamageInflicted >= pkDefender->GetCurrHitPoints()  // Defender at full hit points and will the damage be more than the full hit points?
		        && pkAttacker->isHuman() && !GC.getGame().isGameMultiPlayer())
		{
			gDLL->UnlockAchievement(ACHIEVEMENT_ONEHITKILL);
		}

		pkDefender->changeDamage(iAttackerDamageInflicted, pkAttacker->getOwner());
		iAttackerDamageDelta = pkAttacker->changeDamage(iDefenderDamageInflicted, pkDefender->getOwner(), -1.f);		// Signal that we don't want the popup text.  It will be added later when the unit is at its final location

		// Update experience for both sides.
		pkDefender->changeExperience(
		    kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
		    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
		    true,
		    kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
		    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));

		pkAttacker->changeExperience(
		    kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
		    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
		    true,
		    kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
		    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));

		// Anyone eat it?
		bAttackerDead = (pkAttacker->getDamage() >= GC.getMAX_HIT_POINTS());
		bDefenderDead = (pkDefender->getDamage() >= GC.getMAX_HIT_POINTS());

		CvPlayerAI& kAttackerOwner = GET_PLAYER(pkAttacker->getOwner());
		kAttackerOwner.GetPlayerAchievements().AttackedUnitWithUnit(pkAttacker, pkDefender);

		// Attacker died
		if(bAttackerDead)
		{
			CvPlayerAI& kDefenderOwner = GET_PLAYER(pkDefender->getOwner());
			kDefenderOwner.GetPlayerAchievements().KilledUnitWithUnit(pkDefender, pkAttacker);

			auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
			gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());

			if(iActivePlayerID == pkAttacker->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", pkAttacker->getNameKey(), pkDefender->getNameKey(), iAttackerDamageInflicted, iAttackerFearDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}
			if(iActivePlayerID == pkDefender->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pkDefender->getNameKey(), iAttackerDamageInflicted, iAttackerFearDamageInflicted, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}
#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
			pkDefender->testPromotionReady();
#endif

			ApplyPostCombatTraitEffects(pkDefender, pkAttacker);

		}
		// Defender died
		else if(bDefenderDead)
		{
			kAttackerOwner.GetPlayerAchievements().KilledUnitWithUnit(pkAttacker, pkDefender);

			auto_ptr<ICvUnit1> pDefender = GC.WrapUnitPointer(pkDefender);
			gDLL->GameplayUnitDestroyedInCombat(pDefender.get());

			if(iActivePlayerID == pkAttacker->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey());
				GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}

			if(pkAttacker->getVisualOwner(pkDefender->getTeam()) != pkAttacker->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pkDefender->getNameKey(), pkAttacker->getNameKey(), iDefenderDamageInflicted);
			}
			else
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pkDefender->getNameKey(), pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()), iDefenderDamageInflicted);
			}
			if(iActivePlayerID == pkDefender->getOwner())
			{
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*,GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}
			CvNotifications* pNotification = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
			if(pNotification)
			{
				Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
				pNotification->Add(NOTIFICATION_UNIT_DIED, strBuffer, strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int) pkDefender->getUnitType(), pkDefender->getOwner());
			}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
			pkAttacker->testPromotionReady();
#endif

			ApplyPostCombatTraitEffects(pkAttacker, pkDefender);

			// If defender captured, mark who captured him
			if (kCombatInfo.getDefenderCaptured())
			{
				pkDefender->setCapturingPlayer(pkAttacker->getOwner());
				pkDefender->SetCapturedAsIs(true);
			}
		}
		// Nobody died
		else
		{
			if(iActivePlayerID == pkAttacker->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}
			if(iActivePlayerID == pkDefender->getOwner())
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
			}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
			pkDefender->testPromotionReady();
			pkAttacker->testPromotionReady();
#endif

		}

		// Minors want Barbs near them dead
		if(bAttackerDead)
		{
			if(pkAttacker->isBarbarian())
				pkAttacker->DoTestBarbarianThreatToMinorsWithThisUnitsDeath(pkDefender->getOwner());
		}
		else if(bDefenderDead)
		{
			if(pkDefender->isBarbarian())
				pkDefender->DoTestBarbarianThreatToMinorsWithThisUnitsDeath(pkAttacker->getOwner());
		}
	}

	if(pkAttacker)
	{
		pkAttacker->setCombatUnit(NULL);
		pkAttacker->ClearMissionQueue(GetPostCombatDelay());
	}
	if(pkDefender)
	{
		pkDefender->setCombatUnit(NULL);
		pkDefender->ClearMissionQueue();
	}
	else
		bDefenderDead = true;

	if(pkAttacker)
	{
		if(pkAttacker->isSuicide())
		{
			pkAttacker->setCombatUnit(NULL);	// Must clear this if doing a delayed kill, should this be part of the kill method?
			pkAttacker->kill(true);
		}
		else
		{
#ifdef AUI_WARNING_FIXES
			if (pkTargetPlot && pkDefender)
#else
			if(pkTargetPlot)
#endif
			{
#ifdef NQ_HEAVY_CHARGE_DOWNHILL
				bool isAttackingFromHigherElevation = false;
				if (pkAttacker->GetHeavyChargeDownhill() > 0)
				{
					isAttackingFromHigherElevation = 
						(pkAttacker->plot()->isMountain() && !pkDefender->plot()->isMountain()) ||
						(pkAttacker->plot()->isHills() && pkDefender->plot()->isFlatlands());
				}
				if ((pkAttacker->IsCanHeavyCharge() || (pkAttacker->GetHeavyChargeDownhill() > 0 && isAttackingFromHigherElevation))
					&& !pkDefender->isDelayedDeath() && bAttackerDidMoreDamage)
#else
				if (pkAttacker->IsCanHeavyCharge() && !pkDefender->isDelayedDeath() && bAttackerDidMoreDamage)
#endif
				{
					pkDefender->DoFallBackFromMelee(*pkAttacker);
				}

				bool bCanAdvance = kCombatInfo.getAttackerAdvances() && pkTargetPlot->getNumVisibleEnemyDefenders(pkAttacker) == 0;
				if(bCanAdvance)
				{
					if(kCombatInfo.getAttackerAdvancedVisualization())
						// The combat vis has already 'moved' the unit.  Have the game side just do its movement calculations and pop the unit to the new location.
						pkAttacker->move(*pkTargetPlot, false);
					else
						pkAttacker->UnitMove(pkTargetPlot, true, pkAttacker);

					pkAttacker->PublishQueuedVisualizationMoves();
				}
				else
				{
#ifdef NQ_FIX_MOVES_THAT_CONSUME_ALL_MOVEMENT
					pkAttacker->changeMoves(-1 * std::max(GC.getMOVE_DENOMINATOR(), pkTargetPlot->movementCost(pkAttacker, pkAttacker->plot(), pkAttacker->getMoves())));
#else
					pkAttacker->changeMoves(-1 * std::max(GC.getMOVE_DENOMINATOR(), pkTargetPlot->movementCost(pkAttacker, pkAttacker->plot())));
#endif

					if(!pkAttacker->canMove() || !pkAttacker->isBlitz())
					{
						if(pkAttacker->IsSelected())
						{
							if(GC.GetEngineUserInterface()->GetLengthSelectionList() > 1)
							{
								auto_ptr<ICvUnit1> pDllAttacker = GC.WrapUnitPointer(pkAttacker);
								GC.GetEngineUserInterface()->RemoveFromSelectionList(pDllAttacker.get());
							}
						}
					}
				}
			}

			// If a Unit loses his moves after attacking, do so
#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
			if(!pkAttacker->canMoveAfterAttacking() && pkAttacker->isOutOfAttacks())
#else
			if(!pkAttacker->canMoveAfterAttacking())
#endif
			{
				pkAttacker->finishMoves();
				GC.GetEngineUserInterface()->changeCycleSelectionCounter(1);
			}

			// Now that the attacker is in their final location, show any damage popup
			if (!pkAttacker->IsDead() && iAttackerDamageDelta != 0)
				CvUnit::ShowDamageDeltaText(iAttackerDamageDelta, pkAttacker->plot());
		}

		// Report that combat is over in case we want to queue another attack
		GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved(pkAttacker, bDefenderDead);
	}
}

//	---------------------------------------------------------------------------
//	Function: GenerateRangedCombatInfo
//	Take the input parameters and fill in a CvCombatInfo definition assuming a
//	ranged combat.
//
//	Parameters:
//		pkDefender   	-	Defending unit.  Can be null, in which case the input plot must have a city
//		plot         	-	The plot of the defending unit/city
//		pkCombatInfo 	-	Output combat info
//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateRangedCombatInfo(CvUnit& kAttacker, CvUnit* pkDefender, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	pkCombatInfo->setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, pkDefender);
	pkCombatInfo->setPlot(&plot);

	//////////////////////////////////////////////////////////////////////

	bool bBarbarian = false;
	int iExperience = 0;
	int iMaxXP = 0;
	int iDamage = 0;
	int iTotalDamage = 0;
#ifdef DEL_RANGED_COUNTERATTACKS
	int iDamageToAttacker = 0;
	int iTotalDamageToAttacker = 0;
	const CvPlot* pFromPlot = kAttacker.plot();
	const int iMaxHP = GC.getMAX_HIT_POINTS();
#endif
	PlayerTypes eDefenderOwner;
	if(!plot.isCity())
	{
		CvAssert(pkDefender != NULL);

		eDefenderOwner = pkDefender->getOwner();

		iExperience = /*2*/ GC.getEXPERIENCE_ATTACKING_UNIT_RANGED();
		if(pkDefender->isBarbarian())
			bBarbarian = true;
		iMaxXP = pkDefender->maxXPValue();

		//CvAssert(pkDefender->IsCanDefend());

		iDamage = kAttacker.GetRangeCombatDamage(pkDefender, /*pCity*/ NULL, /*bIncludeRand*/ true);
#ifdef DEL_RANGED_COUNTERATTACKS
		iTotalDamage = pkDefender->getDamage() + iDamage;

		if (GC.getGame().isOption("GAMEOPTION_ENABLE_RANGED_COUNTERATTACKS"))
		{
			if (!kAttacker.isRangedSupportFire() && !pkDefender->IsCityAttackOnly() &&
				// Ranged unit counterattacks
				(pkDefender->canRangeStrike() && pkDefender->canEverRangeStrikeAt(pFromPlot->getX(), pFromPlot->getY())) || 
				// Melee unit counterattacks
				(pkDefender->IsCanAttackWithMove() && plot.isAdjacent(pFromPlot) && pkDefender->PlotValid(pFromPlot) && pkDefender->PlotValid(&plot)))
			{
				if (pkDefender->IsCanAttackRanged())
				{
					iDamageToAttacker = pkDefender->GetRangeCombatDamage(&kAttacker, NULL, true);
				}
				else if (iTotalDamage < iMaxHP)
				{
					// Melee unit (defender) is counterattacking by attacking into the plot from which they were bombarded, where the attacker is
					int iAttackerStrength = kAttacker.GetMaxDefenseStrength(pFromPlot, pkDefender);
					int iDefenderStrength = pkDefender->GetMaxAttackStrength(&plot, pFromPlot, &kAttacker);
					iDamageToAttacker = pkDefender->getCombatDamage(iDefenderStrength, iAttackerStrength, iTotalDamage, /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ false);
				}
			}

			iTotalDamageToAttacker = iDamageToAttacker + kAttacker.getDamage();

			// Will both units be killed by this? If so, take drastic corrective measures
			if (iTotalDamage >= iMaxHP && iTotalDamageToAttacker >= iMaxHP)
			{
				// He who hath the least amount of damage survives with 1 HP left
				if (iTotalDamage > iTotalDamageToAttacker)
				{
					iDamageToAttacker = iMaxHP - kAttacker.getDamage() - 1;
					iTotalDamageToAttacker = iMaxHP - 1;
					iTotalDamage = iMaxHP;
				}
				else
				{
					iDamage = iMaxHP - pkDefender->getDamage() - 1;
					iTotalDamage = iMaxHP - 1;
					iTotalDamageToAttacker = iMaxHP;
				}
			}
		}
#else
		if(iDamage + pkDefender->getDamage() > GC.getMAX_HIT_POINTS())
		{
			iDamage = GC.getMAX_HIT_POINTS() - pkDefender->getDamage();
		}

		iTotalDamage = std::max(pkDefender->getDamage(), pkDefender->getDamage() + iDamage);
#endif
	}
	else
	{
		if (kAttacker.isRangedSupportFire()) return; // can't attack cities with this

		CvCity* pCity = plot.getPlotCity();
		CvAssert(pCity != NULL);
		if(!pCity) return;

		eDefenderOwner = plot.getOwner();
		/*		iDefenderStrength = pCity->getStrengthValue() / 2;
		iOldDamage = pCity->getDamage();*/

		iExperience = /*3*/ GC.getEXPERIENCE_ATTACKING_CITY_RANGED();

		if(pCity->isBarbarian())
			bBarbarian = true;
		iMaxXP = (GET_PLAYER(pCity->getOwner()).isMinorCiv()) ? 30 : 1000; // NQMP GJS - cap XP from fighting CS to 30

		iDamage = kAttacker.GetRangeCombatDamage(/*pDefender*/ NULL, pCity, /*bIncludeRand*/ true);

		// Cities can't be knocked to less than 1 HP
		if(iDamage + pCity->getDamage() >= pCity->GetMaxHitPoints())
		{
			iDamage = pCity->GetMaxHitPoints() - pCity->getDamage() - 1;
		}

#ifdef DEL_RANGED_COUNTERATTACKS
		iTotalDamage = pCity->getDamage() + iDamage;

		if (GC.getGame().isOption("GAMEOPTION_ENABLE_RANGED_COUNTERATTACKS"))
		{
			if (pCity->canRangeStrikeAt(pFromPlot->getX(), pFromPlot->getY(), true))
			{
				iDamageToAttacker = pCity->rangeCombatDamage(&kAttacker, NULL, true);
			}

			iTotalDamageToAttacker = iDamageToAttacker + kAttacker.getDamage();
		}
#else
		iTotalDamage = std::max(pCity->getDamage(), pCity->getDamage() + iDamage);
#endif
	}
	//////////////////////////////////////////////////////////////////////

#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iTotalDamageToAttacker);				// Total damage to the unit
#else
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, 0);				// Total damage to the unit
#endif
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iDamage);		// Damage inflicted this round
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iTotalDamage);		// Total damage to the unit
#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDamageToAttacker);			// Damage inflicted this round
#else
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, 0);			// Damage inflicted this round
#endif

	// Fear Damage
	pkCombatInfo->setFearDamageInflicted(BATTLE_UNIT_ATTACKER, 0);
	// pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_DEFENDER, 0 );

	pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, iExperience);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, iMaxXP);
	pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == eDefenderOwner);
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() && 
		(plot.isCity() || (!pkDefender->isBarbarian() && !GET_PLAYER(pkDefender->getOwner()).isMinorCiv())) &&
		(!plot.isCity() || (!plot.getPlotCity()->isBarbarian() && !GET_PLAYER(plot.getPlotCity()->getOwner()).isMinorCiv()));
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

	iExperience = /*2*/ GC.getEXPERIENCE_DEFENDING_UNIT_RANGED();
	pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iExperience);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, kAttacker.maxXPValue());
	pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !bBarbarian && !kAttacker.isBarbarian());
#endif

	pkCombatInfo->setAttackIsRanged(true);
#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setDefenderRetaliates(iDamageToAttacker > 0);
#else
	// Defender doesn't retaliate.  We'll keep this separate from the ranged attack flag in case something changes to allow
	// some units to retaliate on a ranged attack (Archer vs. Archers maybe?)
	pkCombatInfo->setDefenderRetaliates(false);
#endif

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}

//	---------------------------------------------------------------------------
//	Function: GenerateRangedCombatInfo
//	Take the input parameters and fill in a CvCombatInfo definition assuming a
//	ranged combat from a city.
//
//	Parameters:
//		kAttacker		-	Attacking city.
//		pkDefender   	-	Defending unit.
//		plot         	-	The plot of the defending unit
//		pkCombatInfo 	-	Output combat info
//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateRangedCombatInfo(CvCity& kAttacker, CvUnit* pkDefender, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	pkCombatInfo->setCity(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, pkDefender);
	pkCombatInfo->setPlot(&plot);

	//////////////////////////////////////////////////////////////////////

	bool bBarbarian = false;
	int iDamage = 0;
	int iTotalDamage = 0;
	PlayerTypes eDefenderOwner = NO_PLAYER;
#ifdef DEL_RANGED_COUNTERATTACKS
	int iDamageToAttacker = 0;
	int iTotalDamageToAttacker = 0;
	const CvPlot* pFromPlot = kAttacker.plot();
	const int iMaxHP = GC.getMAX_HIT_POINTS();
#endif
	if(!plot.isCity())
	{
		CvAssert(pkDefender != NULL);

		eDefenderOwner = pkDefender->getOwner();

		if(pkDefender->isBarbarian())
			bBarbarian = true;

		//CvAssert(pkDefender->IsCanDefend());

		iDamage = kAttacker.rangeCombatDamage(pkDefender);

#ifdef DEL_RANGED_COUNTERATTACKS
		iTotalDamage = pkDefender->getDamage() + iDamage;

		if (GC.getGame().isOption("GAMEOPTION_ENABLE_RANGED_COUNTERATTACKS"))
		{
			if (// Ranged unit counterattacks
				(pkDefender->canRangeStrike() && pkDefender->canEverRangeStrikeAt(pFromPlot->getX(), pFromPlot->getY())) ||
				// Melee unit counterattacks
				(pkDefender->IsCanAttackWithMove() && plot.isAdjacent(pFromPlot) && pkDefender->PlotValid(pFromPlot) && pkDefender->PlotValid(&plot)))
			{
				if (pkDefender->IsCanAttackRanged())
				{
					iDamageToAttacker = pkDefender->GetRangeCombatDamage(NULL, &kAttacker, true);
				}
				else if (iTotalDamage < iMaxHP)
				{
					// Melee unit (defender) is counterattacking by attacking into the plot from which they were bombarded, where the attacker is
					int iAttackerStrength = kAttacker.getStrengthValue();
					int iDefenderStrength = pkDefender->GetMaxAttackStrength(&plot, pFromPlot, NULL);
					iDefenderStrength *= /*40*/ GC.getCITY_RANGED_ATTACK_STRENGTH_MULTIPLIER();
					iDefenderStrength /= 100;

					// Damage steps: city deals ranged damage to melee unit -> unit deals melee damage to city with the same -60% strength penalty that cities have for ranged attacks
					iDamageToAttacker = pkDefender->getCombatDamage(iDefenderStrength, iAttackerStrength, iTotalDamage, /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ true);
				}
			}

			iTotalDamageToAttacker = iDamageToAttacker + kAttacker.getDamage();

			// Cities can't be knocked to less than 1 HP by counterattacks
			if (iTotalDamageToAttacker >= kAttacker.GetMaxHitPoints())
			{
				iTotalDamageToAttacker = kAttacker.GetMaxHitPoints() - kAttacker.getDamage() - 1;
			}
		}
#else
		if(iDamage + pkDefender->getDamage() > GC.getMAX_HIT_POINTS())
		{
			iDamage = GC.getMAX_HIT_POINTS() - pkDefender->getDamage();
		}

		iTotalDamage = std::max(pkDefender->getDamage(), pkDefender->getDamage() + iDamage);
#endif
	}
	else
	{
		FAssertMsg(false, "City vs. City not supported.");	// Don't even think about it Jon....
	}

	//////////////////////////////////////////////////////////////////////

#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iTotalDamageToAttacker);				// Total damage to the unit
#else
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, 0);				// Total damage to the unit
#endif
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iDamage);		// Damage inflicted this round
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iTotalDamage);		// Total damage to the unit
#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDamageToAttacker);			// Damage inflicted this round
#else
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, 0);			// Damage inflicted this round
#endif

	// Fear Damage
	pkCombatInfo->setFearDamageInflicted(BATTLE_UNIT_ATTACKER, 0);
	// pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_DEFENDER, 0 );

	pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, 0);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, 0);
	pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == eDefenderOwner);
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() && !pkDefender->isBarbarian() && !GET_PLAYER(pkDefender->getOwner()).isMinorCiv();
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

	int iExperience = /*2*/ GC.getEXPERIENCE_DEFENDING_UNIT_RANGED();
	pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iExperience);
	int iMaxExperience = (GET_PLAYER(kAttacker.getOwner()).isMinorCiv()) ? 30 : MAX_INT; // NQMP GJS - cap XP from fighting CS to 30
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, iMaxExperience);
	pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !bBarbarian && !kAttacker.isBarbarian());
#endif

	pkCombatInfo->setAttackIsRanged(true);
#ifdef DEL_RANGED_COUNTERATTACKS
	pkCombatInfo->setDefenderRetaliates(iDamageToAttacker > 0);
#else
	// Defender doesn't retaliate.  We'll keep this separate from the ranged attack flag in case something changes to allow
	// some units to retaliate on a ranged attack (Archer vs. Archers maybe?)
	pkCombatInfo->setDefenderRetaliates(false);
#endif

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}

//	---------------------------------------------------------------------------
//	Function: ResolveRangedUnitVsCombat
//	Resolve ranged combat where the attacker is a unit.  This will handle
//  unit vs. unit and unit vs. city
//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveRangedUnitVsCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	bool bTargetDied = false;
	int iDamage = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
//	int iExperience = kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER);
//	int iMaxXP = kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER);
	bool bBarbarian = false;

	CvUnit* pkAttacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);
	CvAssert_Debug(pkAttacker);

	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	CvAssert_Debug(pkTargetPlot);

#ifdef DEL_RANGED_COUNTERATTACKS
	bool bAttackerDied = false;
	int iDamageToAttacker = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);
	int iAttackerFearDamageInflicted = 0;//pInfo->getFearDamageInflicted( BATTLE_UNIT_ATTACKER );
#endif

	ICvUserInterface2* pkDLLInterface = GC.GetEngineUserInterface();
	CvString strBuffer;

	if(pkTargetPlot)
	{
		if(!pkTargetPlot->isCity())
		{
			// Unit
			CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
			CvAssert_Debug(pkDefender != NULL);
			if(pkDefender)
			{
				bBarbarian = pkDefender->isBarbarian();

				if(pkAttacker)
				{
#ifdef DEL_RANGED_COUNTERATTACKS
					pkDefender->changeDamage(iDamage, pkAttacker->getOwner());
					pkAttacker->changeDamage(iDamageToAttacker, pkDefender->getOwner());

					pkDefender->changeExperience(
						kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
						kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
						true,
						kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
						kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));
					pkAttacker->changeExperience(
						kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
						kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
						true,
						kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
						kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));

																															// Anyone eat it?
					bAttackerDied = (pkAttacker->getDamage() >= GC.getMAX_HIT_POINTS());
					bTargetDied = (pkDefender->getDamage() >= GC.getMAX_HIT_POINTS());

					if (bTargetDied)
#else
					// Defender died
					if(iDamage + pkDefender->getDamage() >= GC.getMAX_HIT_POINTS())
#endif
					{
						if(pkAttacker->getOwner() == GC.getGame().getActivePlayer())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR_AND_DEATH", pkAttacker->getNameKey(), pkDefender->getNameKey());
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pkDefender->getUnitInfo().GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}

						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR_AND_DEATH", pkDefender->getNameKey(), pkAttacker->getNameKey());
						CvNotifications* pNotifications = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
						if(pNotifications)
						{
							Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
							pNotifications->Add(NOTIFICATION_UNIT_DIED, strBuffer, strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int) pkDefender->getUnitType(), pkDefender->getOwner());
						}

#ifdef DEL_RANGED_COUNTERATTACKS
#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
						pkAttacker->testPromotionReady();
#endif
#else
						bTargetDied = true;
#endif

						CvPlayerAI& kAttackerOwner = GET_PLAYER(pkAttacker->getOwner());
						kAttackerOwner.GetPlayerAchievements().KilledUnitWithUnit(pkAttacker, pkDefender);

						ApplyPostCombatTraitEffects(pkAttacker, pkDefender);

						if(bBarbarian)
						{
							pkDefender->DoTestBarbarianThreatToMinorsWithThisUnitsDeath(pkAttacker->getOwner());
						}

						//One Hit
						if(pkDefender->GetCurrHitPoints() == GC.getMAX_HIT_POINTS() && pkAttacker->isHuman() && !GC.getGame().isGameMultiPlayer())
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_ONEHITKILL);
						}
					}
#ifdef DEL_RANGED_COUNTERATTACKS
					else if (bAttackerDied)
					{
						CvPlayerAI& kDefenderOwner = GET_PLAYER(pkDefender->getOwner());
						kDefenderOwner.GetPlayerAchievements().KilledUnitWithUnit(pkDefender, pkAttacker);

						auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
						gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());

						if (GC.getGame().getActivePlayer() == pkAttacker->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", pkAttacker->getNameKey(), pkDefender->getNameKey(), iDamage, iAttackerFearDamageInflicted);
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
						if (GC.getGame().getActivePlayer() == pkDefender->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pkDefender->getNameKey(), iDamage, iAttackerFearDamageInflicted, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
							GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
						pkDefender->testPromotionReady();
#endif

						ApplyPostCombatTraitEffects(pkDefender, pkAttacker);
					}
#endif
					// Nobody died
					else
					{
						if(pkAttacker->getOwner() == GC.getGame().getActivePlayer())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", pkAttacker->getNameKey(), pkDefender->getNameKey(), iDamage);
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pkDefender->getUnitInfo().GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pkDefender->getNameKey(), pkAttacker->getNameKey(), iDamage);
#if defined(DEL_RANGED_COUNTERATTACKS) && !defined(AUI_UNIT_TEST_PROMOTION_READY_MOVED)
						pkAttacker->testPromotionReady();
						pkDefender->testPromotionReady();
#endif
					}

					//red icon over attacking unit
					if(pkDefender->getOwner() == GC.getGame().getActivePlayer())
					{
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pkAttacker->m_pUnitInfo->GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkAttacker->getX(), pkAttacker->getY(), true, true*/);
					}
					//white icon over defending unit
					//GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), false, 0, ""/*, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pkDefender->getUnitInfo().GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pkDefender->getX(), pkDefender->getY(), true, true*/);

#ifndef DEL_RANGED_COUNTERATTACKS
					//set damage but don't update entity damage visibility
					pkDefender->changeDamage(iDamage, pkAttacker->getOwner());

					// Update experience
					pkDefender->changeExperience(
					    kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
					    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
					    true,
					    kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
					    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));
#endif
				}

				pkDefender->setCombatUnit(NULL);
				if(!CvUnitMission::IsHeadMission(pkDefender, CvTypes::getMISSION_WAIT_FOR()))		// If the top mission was not a 'wait for', then clear it.
					pkDefender->ClearMissionQueue();
			}
			else
				bTargetDied = true;
		}
		else
		{
			// City
			CvCity* pCity = pkTargetPlot->getPlotCity();
			CvAssert_Debug(pCity != NULL);
			if(pCity)
			{
				if(pkAttacker)
				{
					bBarbarian = pCity->isBarbarian();
					pCity->changeDamage(iDamage);

#ifdef DEL_RANGED_COUNTERATTACKS
					pkAttacker->changeDamage(iDamageToAttacker, pCity->getOwner());

					pkAttacker->changeExperience(
						kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
						kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
						true,
						kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
						kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));

					// Anyone eat it?
					bAttackerDied = (pkAttacker->getDamage() >= GC.getMAX_HIT_POINTS());

					if (bAttackerDied)
					{
						auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
						gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());

						if (GC.getGame().getActivePlayer() == pkAttacker->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING_CITY", pkAttacker->getNameKey(), pCity->getNameKey(), iDamage, iAttackerFearDamageInflicted);
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
						if (GC.getGame().getActivePlayer() == pCity->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT_CITY", pCity->getNameKey(), iDamageToAttacker, iAttackerFearDamageInflicted, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pCity->getTeam()));
							GC.messagePlayer(uiParentEventID, pCity->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
					}
					else
					{
#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
						pkAttacker->testPromotionReady();
#endif
#endif
					if(pCity->getOwner() == GC.getGame().getActivePlayer())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOUR_ATTACKED_BY_THEM_AIR", pCity->getNameKey(), pkAttacker->getNameKey(), iDamage);
						//red icon over attacking unit
						GC.messagePlayer(uiParentEventID, pCity->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pkAttacker->m_pUnitInfo->GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkAttacker->getX(), pkAttacker->getY(), true, true*/);
					}
#ifdef DEL_RANGED_COUNTERATTACKS
						if (GC.getGame().getActivePlayer() == pkAttacker->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", pkAttacker->getNameKey(), pCity->getNameKey(), iDamage);
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
						}
					}
#endif
				}

				pCity->clearCombat();
			}
			else
				bTargetDied = true;
		}
	}
	else
		bTargetDied = true;

	if(pkAttacker)
	{
#ifndef DEL_RANGED_COUNTERATTACKS
		// Unit gains XP for executing a Range Strike
		if(iDamage > 0) // && iDefenderStrength > 0)
		{
			pkAttacker->changeExperience(
			    kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
			    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
			    true,
			    kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
			    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));
		}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
		pkAttacker->testPromotionReady();
#endif
#endif

		pkAttacker->setCombatUnit(NULL);
		pkAttacker->ClearMissionQueue(GetPostCombatDelay());

		// Report that combat is over in case we want to queue another attack
		GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved(pkAttacker, bTargetDied);
	}
}

//	---------------------------------------------------------------------------
//	Function: ResolveRangedCityVsUnitCombat
//	Resolve ranged combat where the attacker is a city
//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveRangedCityVsUnitCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	bool bTargetDied = false;
	int iDamage = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
	bool bBarbarian = false;

	CvCity* pkAttacker = kCombatInfo.getCity(BATTLE_UNIT_ATTACKER);
	CvAssert_Debug(pkAttacker);

	if(pkAttacker)
		pkAttacker->clearCombat();

	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	CvAssert_Debug(pkTargetPlot);

	ICvUserInterface2* pkDLLInterface = GC.GetEngineUserInterface();
	int iActivePlayerID = GC.getGame().getActivePlayer();

#ifdef DEL_RANGED_COUNTERATTACKS
	int iDamageToCity = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);

	CvString strBuffer;
#endif

	if(pkTargetPlot)
	{
		if(!pkTargetPlot->isCity())
		{
			CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
			CvAssert_Debug(pkDefender != NULL);
			if(pkDefender)
			{
				bBarbarian = pkDefender->isBarbarian();

				if(pkAttacker)
				{
#ifdef DEL_RANGED_COUNTERATTACKS
					pkDefender->changeDamage(iDamage, pkAttacker->getOwner());
					pkAttacker->changeDamage(iDamageToCity);

					pkDefender->changeExperience(
						kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
						kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
						true,
						kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
						kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));

					if (pkDefender->IsDead())
					{
						CvNotifications* pNotifications = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
						if (pNotifications)
						{
							Localization::String localizedText = Localization::Lookup("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_CITY");
							localizedText << pkDefender->getNameKey() << pkAttacker->getNameKey() << iDamage;
							Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
							pNotifications->Add(NOTIFICATION_UNIT_DIED, localizedText.toUTF8(), strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int)pkDefender->getUnitType(), pkDefender->getOwner());
						}

						if (pkAttacker->getOwner() == iActivePlayerID && iDamageToCity > 0)
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT_CITY", pkAttacker->getNameKey(), iDamageToCity, pkDefender->getNameKey(), pkDefender->getVisualCivAdjective(pkAttacker->getTeam()));
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkPlot->getX(), pkPlot->getY()*/);
						}

						// Earn bonuses for kills?
						CvPlayer& kAttackingPlayer = GET_PLAYER(pkAttacker->getOwner());
						kAttackingPlayer.DoYieldsFromKill(NO_UNIT, pkDefender->getUnitType(), pkDefender->getX(), pkDefender->getY(), pkDefender->isBarbarian(), 0);
					}
					// Neither side lost
					else if (iDamageToCity > 0)
					{
						if (pkDefender->getOwner() == iActivePlayerID)
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW_CITY", pkDefender->getNameKey(), iDamage, pkAttacker->getNameKey(), iDamageToCity);
							GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkPlot->getX(), pkPlot->getY()*/);
						}
						if (pkAttacker->getOwner() == iActivePlayerID)
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW_CITY", pkDefender->getNameKey(), iDamageToCity, pkAttacker->getNameKey(), iDamage);
							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkPlot->getX(), pkPlot->getY()*/);
						}

						ApplyPostCityCombatEffects(pkDefender, pkAttacker, iDamageToCity);
					}
#else
					// Info message for the attacking player
					if(iActivePlayerID == pkAttacker->getOwner())
					{
						Localization::String localizedText = Localization::Lookup("TXT_KEY_MISC_YOUR_CITY_RANGE_ATTACK");
						localizedText << pkAttacker->getNameKey() << pkDefender->getNameKey() << iDamage;
						GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), localizedText.toUTF8());//, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pDefender->getUnitInfo().GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX(), pPlot->getY());
					}

					// Red icon over defending unit
					if(iActivePlayerID == pkDefender->getOwner())
					{
						Localization::String localizedText = Localization::Lookup("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_CITY");
						localizedText << pkDefender->getNameKey() << pkAttacker->getNameKey() << iDamage;
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), localizedText.toUTF8());//, "AS2D_COMBAT", MESSAGE_TYPE_COMBAT_MESSAGE, pDefender->getUnitInfo().GetButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pDefender->getX(), pDefender->getY(), true, true);
					}

					if(iDamage + pkDefender->getDamage() >= GC.getMAX_HIT_POINTS())
					{
						CvNotifications* pNotifications = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
						if(pNotifications)
						{
							Localization::String localizedText = Localization::Lookup("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_CITY");
							localizedText << pkDefender->getNameKey() << pkAttacker->getNameKey() << iDamage;
							Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
							pNotifications->Add(NOTIFICATION_UNIT_DIED, localizedText.toUTF8(), strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int) pkDefender->getUnitType(), pkDefender->getOwner());
						}
						bTargetDied = true;

						// Earn bonuses for kills?
						CvPlayer& kAttackingPlayer = GET_PLAYER(pkAttacker->getOwner());
						kAttackingPlayer.DoYieldsFromKill(NO_UNIT, pkDefender->getUnitType(), pkDefender->getX(), pkDefender->getY(), pkDefender->isBarbarian(), 0);
					}

					//set damage but don't update entity damage visibility
					pkDefender->changeDamage(iDamage, pkAttacker->getOwner());

					// Update experience
					pkDefender->changeExperience(
					    kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
					    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
					    true,
					    kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
					    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));
#endif
				}

				pkDefender->setCombatUnit(NULL);
				if(!CvUnitMission::IsHeadMission(pkDefender, CvTypes::getMISSION_WAIT_FOR()))		// If the top mission was not a 'wait for', then clear it.
					pkDefender->ClearMissionQueue();
			}
			else
				bTargetDied = true;
		}
		else
		{
			CvAssert(false);	// Left as an exercise for the reader
			bTargetDied = true;
		}
	}

	// Report that combat is over in case we want to queue another attack
	if(pkAttacker)
		GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved((void*)pkAttacker, bTargetDied, true);
}

//	---------------------------------------------------------------------------
//	Function: ResolveCityMeleeCombat
//
//	Resolves combat between a melee unit and a city.
//  The unit does not have to be a hand-to-hand combat type unit, just a unit doing
//  a non-ranged attack to an adjacent city.  The visualization of the attack will
//	usually appear as if it is ranged, simply because we don't want the unit members
//	running through a city and they wouldn't have anything to attack.
//	This is also the case where a city is able to attack back.
void CvUnitCombat::ResolveCityMeleeCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	CvUnit* pkAttacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);
	CvCity* pkDefender = kCombatInfo.getCity(BATTLE_UNIT_DEFENDER);

	CvAssert_Debug(pkAttacker && pkDefender);

	CvPlot* pkPlot = kCombatInfo.getPlot();
	if(!pkPlot && pkDefender)
		pkPlot = pkDefender->plot();

	CvAssert_Debug(pkPlot);

	int iAttackerDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
	int iDefenderDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);

	if(pkAttacker && pkDefender)
	{
		pkAttacker->changeDamage(iDefenderDamageInflicted, pkDefender->getOwner());
		pkDefender->changeDamage(iAttackerDamageInflicted);

		pkAttacker->changeExperience(kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
		                             kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
		                             true,
		                             false,
		                             kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));
	}

	bool bCityConquered = false;

	if(pkDefender)
		pkDefender->clearCombat();
	else
		bCityConquered = true;

	if(pkAttacker)
	{
		pkAttacker->setCombatUnit(NULL);
		pkAttacker->ClearMissionQueue(GetPostCombatDelay());

		if(pkAttacker->isSuicide())
		{
			pkAttacker->setCombatUnit(NULL);	// Must clear this if doing a delayed kill, should this be part of the kill method?
			pkAttacker->kill(true);
		}
	}

	CvString strBuffer;
	int iActivePlayerID = GC.getGame().getActivePlayer();

	// Barbarians don't capture Cities
	if(pkAttacker && pkDefender)
	{
		if(pkAttacker->isBarbarian() && (pkDefender->getDamage() >= pkDefender->GetMaxHitPoints()))
		{
			// 1 HP left
			pkDefender->setDamage(pkDefender->GetMaxHitPoints() - 1);

			int iNumGoldStolen = GC.getBARBARIAN_CITY_GOLD_RANSOM();	// 200

			if(iNumGoldStolen > GET_PLAYER(pkDefender->getOwner()).GetTreasury()->GetGold())
			{
				iNumGoldStolen = GET_PLAYER(pkDefender->getOwner()).GetTreasury()->GetGold();
			}

			// City is ransomed for Gold
			GET_PLAYER(pkDefender->getOwner()).GetTreasury()->ChangeGold(-iNumGoldStolen);

			if(pkDefender->getOwner() == iActivePlayerID)
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_CITY_RANSOMED_BY_BARBARIANS", pkDefender->getNameKey(), iNumGoldStolen);
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*,GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkPlot->getX(), pkPlot->getY()*/);
			}

			if( pkDefender->GetPlayer()->GetID() == GC.getGame().getActivePlayer() && pkDefender->isHuman() && !GC.getGame().isGameMultiPlayer())
			{
				gDLL->UnlockAchievement(ACHIEVEMENT_REALLY_SUCK);
			}

			// Barb goes away after ransom
			pkAttacker->kill(true, NO_PLAYER);

			// Treat this as a conquest
			bCityConquered = true;
		}
		// Attacker died
		else if(pkAttacker->IsDead())
		{
			auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
			gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());
			if(pkAttacker->getOwner() == iActivePlayerID)
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING_CITY", pkAttacker->getNameKey(), pkDefender->getNameKey(), iAttackerDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkPlot->getX(), pkPlot->getY()*/);
			}
			if(pkDefender->getOwner() == iActivePlayerID)
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT_CITY", pkDefender->getNameKey(), iAttackerDamageInflicted, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkPlot->getX(), pkPlot->getY()*/);
			}
		}
		// City conquest
		else if(pkDefender->getDamage() >= pkDefender->GetMaxHitPoints())
		{
			if(!pkAttacker->isNoCapture())
			{
				if(pkAttacker->getOwner() == iActivePlayerID)
				{
					strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_CAPTURED_ENEMY_CITY", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey());
					GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkPlot->getX(), pkPlot->getY()*/);
				}
				if(pkDefender->getOwner() == iActivePlayerID)
				{
					strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_CITY_WAS_CAPTURED", pkDefender->getNameKey(), pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()), iDefenderDamageInflicted);
					GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*,GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkPlot->getX(), pkPlot->getY()*/);
				}

				pkAttacker->UnitMove(pkPlot, true, pkAttacker);

				bCityConquered = true;
			}
		}
		// Neither side lost
		else
		{
			if(pkAttacker->getOwner() == iActivePlayerID)
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW_CITY", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkPlot->getX(), pkPlot->getY()*/);
			}
			if(pkDefender->getOwner() == iActivePlayerID)
			{
				strBuffer = GetLocalizedText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW_CITY", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkPlot->getX(), pkPlot->getY()*/);
			}
			pkAttacker->changeMoves(-GC.getMOVE_DENOMINATOR());

			ApplyPostCityCombatEffects(pkAttacker, pkDefender, iAttackerDamageInflicted);
		}
	}

	if(pkAttacker)
	{
		pkAttacker->PublishQueuedVisualizationMoves();

#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
		if(!pkAttacker->canMoveAfterAttacking() && pkAttacker->isOutOfAttacks())
#else
		if(!pkAttacker->canMoveAfterAttacking())
#endif
		{
			pkAttacker->finishMoves();
			GC.GetEngineUserInterface()->changeCycleSelectionCounter(1);
		}

		// Report that combat is over in case we want to queue another attack
		GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved(pkAttacker, bCityConquered);
	}
}

//	GenerateAirCombatInfo
//	Function: GenerateRangedCombatInfo
//	Take the input parameters and fill in a CvCombatInfo definition assuming a
//	air bombing mission.
//
//	Parameters:
//		pkDefender   	-	Defending unit.  Can be null, in which case the input plot must have a city
//		plot         	-	The plot of the defending unit/city
//		pkCombatInfo 	-	Output combat info
//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateAirCombatInfo(CvUnit& kAttacker, CvUnit* pkDefender, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	int iExperience = 0;

	pkCombatInfo->setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, pkDefender);
	pkCombatInfo->setPlot(&plot);


	//////////////////////////////////////////////////////////////////////
	float attackInterceptModifier = 1; // we will deal this much less damage than we otherwise would have due to interception
	int iInterceptionDamage = 0; // how much damage does the attacker take from the interception
	{
		CvUnit* pInterceptor = kAttacker.GetBestInterceptor(plot, pkDefender);
		bool attackerIsInterceptable = kAttacker.evasionProbability() > 0;
		bool interceptorExists = pInterceptor != NULL;

		if (interceptorExists && attackerIsInterceptable)
		{
			pkCombatInfo->setUnit(BATTLE_UNIT_INTERCEPTOR, pInterceptor);
			iInterceptionDamage = pInterceptor->GetInterceptionDamage(&kAttacker);
			pkCombatInfo->setDamageInflicted(BATTLE_UNIT_INTERCEPTOR, iInterceptionDamage);		// Damage inflicted this round

			const int evasion = kAttacker.evasionProbability();
			const int interception = pInterceptor->currInterceptionProbability();
			if (evasion > 0 && interception > 0)
			{
				float total = evasion + interception;
				attackInterceptModifier = evasion / total;
			}
		}
	}

	// non nuclear missile
	{
		CvUnit* interceptor = getMissileInterceptor(kAttacker, plot);
		bool wasShotDown = interceptor != NULL;
		if (wasShotDown)
		{
			attackInterceptModifier = 0;
			pkCombatInfo->setUnit(BATTLE_UNIT_INTERCEPTOR, interceptor);
			pkCombatInfo->setDamageInflicted(BATTLE_UNIT_INTERCEPTOR, kAttacker.GetMaxHitPoints());
		}
	}

	//////////////////////////////////////////////////////////////////////

	bool bBarbarian = false;
	int iMaxXP = 0;

	int iAttackerDamageInflicted;
	int iDefenderDamageInflicted;

	int iAttackerTotalDamageInflicted;
	int iDefenderTotalDamageInflicted;

	PlayerTypes eDefenderOwner;

	// Target is a Unit
	if(!plot.isCity())
	{
		CvAssert(pkDefender != NULL);
		if(!pkDefender)
			return;

		eDefenderOwner = pkDefender->getOwner();

		iExperience = /*4*/ GC.getEXPERIENCE_ATTACKING_UNIT_AIR();
		if(pkDefender->isBarbarian())
			bBarbarian = true;
		iMaxXP = pkDefender->maxXPValue();

		// Calculate attacker damage
		iAttackerDamageInflicted = kAttacker.GetAirCombatDamage(pkDefender, /*pCity*/ NULL, /*bIncludeRand*/ true, iInterceptionDamage);

		if(iAttackerDamageInflicted + pkDefender->getDamage() > GC.getMAX_HIT_POINTS())
		{
			iAttackerDamageInflicted = GC.getMAX_HIT_POINTS() - pkDefender->getDamage();
		}

		iAttackerTotalDamageInflicted = std::max(pkDefender->getDamage(), pkDefender->getDamage() + iAttackerDamageInflicted);

		// Calculate defense damage
		iDefenderDamageInflicted = pkDefender->GetAirStrikeDefenseDamage(&kAttacker);

		if(iDefenderDamageInflicted + kAttacker.getDamage() > GC.getMAX_HIT_POINTS())
		{
			iDefenderDamageInflicted = GC.getMAX_HIT_POINTS() - kAttacker.getDamage();
		}

		iDefenderTotalDamageInflicted = std::max(kAttacker.getDamage(), kAttacker.getDamage() + (iDefenderDamageInflicted + iInterceptionDamage));
	}
	// Target is a City
	else
	{
		CvCity* pCity = plot.getPlotCity();
		CvAssert(pCity != NULL);
		if(!pCity) return;

		eDefenderOwner = plot.getOwner();

		/*		iDefenderStrength = pCity->getStrengthValue() / 2;
		iOldDamage = pCity->getDamage();*/

		iExperience = /*4*/ GC.getEXPERIENCE_ATTACKING_CITY_AIR();

		if(pCity->isBarbarian())
			bBarbarian = true;
		iMaxXP = (GET_PLAYER(pCity->getOwner()).isMinorCiv()) ? 30 : 1000; // NQMP GJS - cap XP from fighting CS to 30

		iAttackerDamageInflicted = kAttacker.GetAirCombatDamage(/*pUnit*/ NULL, pCity, /*bIncludeRand*/ true, iInterceptionDamage);

		// Cities can't be knocked to less than 1 HP
		if(iAttackerDamageInflicted + pCity->getDamage() >= pCity->GetMaxHitPoints())
		{
			iAttackerDamageInflicted = pCity->GetMaxHitPoints() - pCity->getDamage() - 1;
		}

		iAttackerTotalDamageInflicted = std::max(pCity->getDamage(), pCity->getDamage() + iAttackerDamageInflicted);

		// Calculate defense damage
		iDefenderDamageInflicted = pCity->GetAirStrikeDefenseDamage(&kAttacker);

		if(iDefenderDamageInflicted + kAttacker.getDamage() > pCity->GetMaxHitPoints())
		{
			iDefenderDamageInflicted = GC.getMAX_HIT_POINTS() - kAttacker.getDamage();
		}

		iDefenderTotalDamageInflicted = std::max(kAttacker.getDamage(), kAttacker.getDamage() + (iDefenderDamageInflicted + iInterceptionDamage));

		//Achievement for Washington
		CvUnitEntry* pkUnitInfo = GC.getUnitInfo(kAttacker.getUnitType());
		if(pkUnitInfo)
		{
			if(kAttacker.isHuman() && !GC.getGame().isGameMultiPlayer() && _stricmp(pkUnitInfo->GetType(), "UNIT_AMERICAN_B17") == 0)
			{
				gDLL->UnlockAchievement(ACHIEVEMENT_SPECIAL_B17);
			}
		}
	}

	iAttackerDamageInflicted *= attackInterceptModifier;

	//////////////////////////////////////////////////////////////////////

	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iDefenderTotalDamageInflicted);				// Total damage to the unit
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iAttackerDamageInflicted);		// Damage inflicted this round
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iAttackerTotalDamageInflicted);		// Total damage to the unit
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDefenderDamageInflicted);			// Damage inflicted this round

	// Fear Damage
	pkCombatInfo->setFearDamageInflicted(BATTLE_UNIT_ATTACKER, 0);
	// pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_DEFENDER, 0 );

	pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, iExperience);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, iMaxXP);
	pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == eDefenderOwner);
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() &&
		(plot.isCity() || (!pkDefender->isBarbarian() && !GET_PLAYER(pkDefender->getOwner()).isMinorCiv())) &&
		(!plot.isCity() || (!plot.getPlotCity()->isBarbarian() && !GET_PLAYER(plot.getPlotCity()->getOwner()).isMinorCiv()));
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

	iExperience = /*2*/ GC.getEXPERIENCE_DEFENDING_UNIT_AIR();
	pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iExperience);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, MAX_INT);
	pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !bBarbarian);
#endif

	if (iInterceptionDamage > 0)
	{
		iExperience = /*2*/ GC.getEXPERIENCE_DEFENDING_AIR_SWEEP_GROUND();
		pkCombatInfo->setExperience( BATTLE_UNIT_INTERCEPTOR, iExperience );
		pkCombatInfo->setMaxExperienceAllowed( BATTLE_UNIT_INTERCEPTOR, MAX_INT );
		pkCombatInfo->setInBorders( BATTLE_UNIT_INTERCEPTOR, plot.getOwner() == kAttacker.getOwner() );
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_INTERCEPTOR, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal( BATTLE_UNIT_INTERCEPTOR, !bBarbarian );
#endif
	}

	pkCombatInfo->setAttackIsBombingMission(true);
	pkCombatInfo->setDefenderRetaliates(true);

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}



//void handleMessages(
//	int uiParentEventID,
//
//	PlayerTypes attackPlayer,
//	string attackPlayerName,
//	string attackUnitName,
//	int attackDamage,
//	bool attackDied,
//
//	PlayerTypes defendPlayer,
//	string defendPlayerName,
//	string defendUnitName,
//	int defendDamage,
//	bool defendDied,
//
//	PlayerTypes interceptPlayer,
//	string interceptPlayerName,
//	string interceptUnitName,
//	int interceptDamage
//)
//{
//	CvString strAttacker;
//	CvString strDefender;
//
//
//	if (pkTargetPlot)
//	{
//		if (!pkTargetPlot->isCity())
//		{
//			// Target was a Unit
//			CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
//			CvAssert_Debug(pkDefender != NULL);
//
//			if (pkDefender)
//			{
//				if (pkAttacker)
//				{
//					// Update experience
//					pkDefender->changeExperience(
//						kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
//						kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
//						true,
//						kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
//						kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));
//
//					// Attacker died
//					if (attackDied)
//					{
//						strAttacker = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", attackUnitName, defendUnitName, iAttackerDamageInflicted);
//						strDefender = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", defendUnitName, iAttackerDamageInflicted, 0, attackUnitName, pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
//
//						ApplyPostCombatTraitEffects(pkDefender, pkAttacker);
//					}
//					// Defender died
//					else if (defendDied)
//					{
//						CvPlayerAI& kAttackerOwner = GET_PLAYER(pkAttacker->getOwner());
//						kAttackerOwner.GetPlayerAchievements().KilledUnitWithUnit(pkAttacker, pkDefender);
//
//						strAttacker = GetLocalizedText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR_AND_DEATH", attackUnitName, defendUnitName);
//						strDefender = GetLocalizedText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR_AND_DEATH", defendUnitName, attackUnitName);
//
//						// special death notification
//						CvNotifications* pNotifications = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
//						if (pNotifications)
//						{
//							Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
//							pNotifications->Add(NOTIFICATION_UNIT_DIED, strBuffer, strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int)pkDefender->getUnitType(), pkDefender->getOwner());
//						}
//
//						ApplyPostCombatTraitEffects(pkAttacker, pkDefender);
//					}
//					else // Nobody died
//					{
//						strAttacker = GetLocalizedText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", attackUnitName, defendUnitName, iDefenderDamageInflicted);
//						strDefender = GetLocalizedText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", defendUnitName, attackUnitName, iAttackerDamageInflicted);
//					}
//				}
//
//
//				if (interceptDamage > 0) // handle interception
//				{
//					CvString strInterceptor = GetLocalizedText("TXT_KEY_MISC_ENEMY_AIR_UNIT_INTERCEPTED", interceptUnitName, attackPlayerName, attackUnitName, interceptDamage);
//					GC.messagePlayer(uiParentEventID, interceptPlayer, true, GC.getEVENT_MESSAGE_TIME(), strInterceptor);
//
//					CvString strInterceptee = GetLocalizedText("TXT_KEY_MISC_FRIENDLY_AIR_UNIT_INTERCEPTED", attackUnitName, interceptPlayerName, interceptUnitName, interceptDamage);
//					GC.messagePlayer(uiParentEventID, attackPlayer, true, GC.getEVENT_MESSAGE_TIME(), strBuffer);
//				}
//
//				GC.messagePlayer(uiParentEventID, attackPlayer, true, GC.getEVENT_MESSAGE_TIME(), strAttacker);
//				GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strDefender);
//
//				//set damage but don't update entity damage visibility
//				//pkDefender->changeDamage(iDamage, pkAttacker->getOwner());
//
//				pkDefender->setCombatUnit(NULL);
//				if (!CvUnitMission::IsHeadMission(pkDefender, CvTypes::getMISSION_WAIT_FOR()))		// If the top mission was not a 'wait for', then clear it.
//					pkDefender->ClearMissionQueue();
//			}
//			else
//			{
//				bTargetDied = true;
//			}
//		}
//		else
//		{
//			// Target was a City
//			CvCity* pCity = pkTargetPlot->getPlotCity();
//			CvAssert_Debug(pCity != NULL);
//
//			if (pCity)
//			{
//				pCity->clearCombat();
//				if (pkAttacker)
//				{
//					pCity->changeDamage(iAttackerDamageInflicted);
//					pkAttacker->changeDamage(iDefenderDamageInflicted, pCity->getOwner());
//
//					//		iUnitDamage = std::max(pCity->getDamage(), pCity->getDamage() + iDamage);
//
//					if (pkAttacker->IsDead())
//					{
//						if (iActivePlayerID == pkAttacker->getOwner())
//						{
//							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING_CITY", pkAttacker->getNameKey(), pCity->getNameKey(), iAttackerDamageInflicted);
//							GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
//						}
//					}
//
//					if (pCity->getOwner() == iActivePlayerID)
//					{
//						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOUR_ATTACKED_BY_THEM_AIR", pCity->getNameKey(), pkAttacker->getNameKey(), iDefenderDamageInflicted);
//						//red icon over attacking unit
//						GC.messagePlayer(uiParentEventID, pCity->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), strBuffer);
//					}
//				}
//			}
//			else
//			{
//				bTargetDied = true;
//			}
//		}
//	}
//	else
//	{
//		bTargetDied = true;
//	}
//}




//	ResolveAirUnitVsCombat
//	Function: ResolveRangedUnitVsCombat
//	Resolve air combat where the attacker is a unit.  This will handle
//  unit vs. unit and unit vs. city
//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveAirUnitVsCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	CvUnit* attacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);	
	CvAssert_Debug(attacker); 
	if (attacker == NULL) return; // If there's no valid attacker, then get out of here

	PlayerTypes attackOwner = NO_PLAYER;
	PlayerTypes defendOwner = NO_PLAYER;
	CvString strAttacker = "";
	CvString strDefender = "";
	CvString strAttackerDied = "";
	CvString strDefenderDied = "";
	bool bTargetDied = false;
	int totalDamageToDefender = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
	int damageToAttackerBase = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);
	int totalDamageToAttacker = damageToAttackerBase;

	attackOwner = attacker->getOwner();

	// Interception?
	CvUnit* interceptor = kCombatInfo.getUnit(BATTLE_UNIT_INTERCEPTOR);
	if (interceptor)
	{
		int interceptDamage = kCombatInfo.getDamageInflicted(BATTLE_UNIT_INTERCEPTOR);
		if (interceptDamage > 0)
			totalDamageToAttacker += interceptDamage;

		if (!attacker->isSuicide())
			interceptor->setMadeInterception(true); // dont consume interceptions for suicide units
		interceptor->setCombatUnit(NULL);
		interceptor->changeExperience(
			kCombatInfo.getExperience(BATTLE_UNIT_INTERCEPTOR),
			kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_INTERCEPTOR),
			true,
			kCombatInfo.getInBorders(BATTLE_UNIT_INTERCEPTOR),
			kCombatInfo.getUpdateGlobal(BATTLE_UNIT_INTERCEPTOR));

		if (interceptor)
		{
			if (interceptDamage == attacker->GetMaxHitPoints()) // interceptor totally shot down the attacker
			{
				string message = "Your {1_UnitName} shot down a {2_owner_adjective} {3_EnUName}";
				CvString strInterceptor = GetLocalizedText(message.c_str(), interceptor->getNameKey(), attacker->getVisualCivAdjective(attacker->getTeam()), attacker->getNameKey());
				GC.messagePlayer(uiParentEventID, interceptor->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strInterceptor);

				CvString strInterceptee = GetLocalizedText("Your {1_UnitName} was shot down by a {2_owner_adjective} {3_EnUName}.", attacker->getNameKey(), interceptor->getVisualCivAdjective(interceptor->getTeam()), interceptor->getNameKey());
				GC.messagePlayer(uiParentEventID, attacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strInterceptee);
			}
			else if (interceptDamage > 0) // handle non total interception
			{
				string message = "Your {1_UnitName} intercepted a {2_owner_adjective} {3_EnUName} and dealt {4_damage} damage to it.";
				CvString strInterceptor = GetLocalizedText(message.c_str(), interceptor->getNameKey(), attacker->getVisualCivAdjective(attacker->getTeam()), attacker->getNameKey(), interceptDamage);
				GC.messagePlayer(uiParentEventID, interceptor->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strInterceptor);

				CvString strInterceptee = GetLocalizedText("Your {1_UnitName} was intercepted by a {2_owner_adjective} {3_EnUName} which dealt {4_damage} damage to it.", attacker->getNameKey(), interceptor->getVisualCivAdjective(interceptor->getTeam()), interceptor->getNameKey(), interceptDamage);
				GC.messagePlayer(uiParentEventID, attacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strInterceptee);
			}
		}
	}

	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	CvAssert_Debug(pkTargetPlot);

	if(pkTargetPlot)
	{
		if(pkTargetPlot->isCity()) // Target was City
		{
			CvCity* defenderCity = pkTargetPlot->getPlotCity();
			CvAssert_Debug(defenderCity != NULL);

			if (defenderCity)
			{
				defendOwner = defenderCity->getOwner();
				defenderCity->clearCombat();

				defenderCity->changeDamage(totalDamageToDefender);
				attacker->changeDamage(totalDamageToAttacker, defenderCity->getOwner());

				strAttacker = GetLocalizedText("Your {1_EnUName} took {2_Num} damage attacking the City of {3_CityName} which took {4_Num} damage!", attacker->getNameKey(), damageToAttackerBase, defenderCity->getNameKey(), totalDamageToDefender);
				strDefender = GetLocalizedText("The City of {1_CityName} took {2_Num} damage from an enemy {3_EnUName} which took {4_Num} damage!", defenderCity->getNameKey(), totalDamageToDefender, attacker->getNameKey(), damageToAttackerBase);
				if (attacker->IsDead())
					strAttackerDied = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED", attacker->getNameKey());
			}
			else
				bTargetDied = true;
			
		}
		else // Target was Unit
		{
			CvUnit* defender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
			CvAssert_Debug(defender != NULL);

			if (defender)
			{
				defendOwner = defender->getOwner();

				//One Hit
				if (defender->GetCurrHitPoints() == GC.getMAX_HIT_POINTS() && attacker->isHuman() && !GC.getGame().isGameMultiPlayer())
				{
					gDLL->UnlockAchievement(ACHIEVEMENT_ONEHITKILL);
				}

				attacker->changeDamage(totalDamageToAttacker, defender->getOwner());
				defender->changeDamage(totalDamageToDefender, attacker->getOwner());

				// Update experience
				defender->changeExperience(
					kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
					kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
					true,
					kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
					kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));

				// Attacker died
				if (attacker->IsDead())
				{
					auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(attacker);
					gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());
					CvPlayerAI& kDefenderOwner = GET_PLAYER(defender->getOwner());
					kDefenderOwner.GetPlayerAchievements().KilledUnitWithUnit(defender, attacker);
					ApplyPostCombatTraitEffects(defender, attacker);
				}
				// Defender died
				else if (defender->IsDead())
				{
					bTargetDied = true;
					CvPlayerAI& kAttackerOwner = GET_PLAYER(attacker->getOwner());
					kAttackerOwner.GetPlayerAchievements().KilledUnitWithUnit(attacker, defender);

					// special death notification
					CvNotifications* pNotifications = GET_PLAYER(defender->getOwner()).GetNotifications();
					if (pNotifications)
					{
						Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
						pNotifications->Add(NOTIFICATION_UNIT_DIED, strDefender, strSummary.toUTF8(), defender->getX(), defender->getY(), (int)defender->getUnitType(), defender->getOwner());
					}

					ApplyPostCombatTraitEffects(attacker, defender);
				}
				else // Nobody died
				{

				}

				strAttacker = GetLocalizedText("Your {1_UnitName} took {2_Damage} damage attacking an enemy {3_EnUName} for {4_Damage} damage.", attacker->getNameKey(), damageToAttackerBase, defender->getNameKey(), totalDamageToDefender);
				strDefender = GetLocalizedText("Your {1_UnitName} took {2_Num} damage from an enemy {3_EnUName} which took {4_Damage} damage!", defender->getNameKey(), totalDamageToDefender, attacker->getNameKey(), damageToAttackerBase);
				if (attacker->IsDead() && !attacker->isSuicide())
					strAttackerDied = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED", attacker->getNameKey());
				if (defender->IsDead() && !attacker->isSuicide())
					strDefenderDied = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED", defender->getNameKey());

				defender->setCombatUnit(NULL);
				if (!CvUnitMission::IsHeadMission(defender, CvTypes::getMISSION_WAIT_FOR())) // If the top mission was not a 'wait for', then clear it.
					defender->ClearMissionQueue();
			}
			else
				bTargetDied = true;
		}
	}
	else
	{
		bTargetDied = true;
	}

	
	if(attacker->isSuicide())
	{
		attacker->setCombatUnit(NULL); // Must clear this if doing a delayed kill, should this be part of the kill method?
		attacker->kill(true);
	}
	else
	{
		// Experience
		if(totalDamageToDefender > 0)
		{
			attacker->changeExperience(kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
				                            kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
				                            true,
				                            kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
				                            kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));
			// Promotion time?
			attacker->testPromotionReady();
		}
		// Clean up some stuff
		attacker->setCombatUnit(NULL);
		attacker->ClearMissionQueue(GetPostCombatDelay());
		// Spend a move for this attack
		attacker->changeMoves(-GC.getMOVE_DENOMINATOR());
		// Can't move or attack again
		if(!attacker->canMoveAfterAttacking() && attacker->isOutOfAttacks())
		{
			attacker->finishMoves();
		}
	}

	// Report that combat is over in case we want to queue another attack
	GET_PLAYER(attacker->getOwner()).GetTacticalAI()->CombatResolved(attacker, bTargetDied);


	if (strAttacker.length())
		GC.messagePlayer(uiParentEventID, attackOwner, true, GC.getEVENT_MESSAGE_TIME(), strAttacker);
	if (strAttackerDied.length() > 0)
		GC.messagePlayer(uiParentEventID, attackOwner, true, GC.getEVENT_MESSAGE_TIME(), strAttackerDied);

	if (strDefender.length() > 0)
		GC.messagePlayer(uiParentEventID, defendOwner, true, GC.getEVENT_MESSAGE_TIME(), strDefender);
	if (strDefenderDied.length() > 0)
		GC.messagePlayer(uiParentEventID, defendOwner, true, GC.getEVENT_MESSAGE_TIME(), strDefenderDied);
}

//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateAirSweepCombatInfo(CvUnit& kAttacker, CvUnit* pkDefender, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	int iMaxHP = GC.getMAX_HIT_POINTS();

	pkCombatInfo->setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, pkDefender);
	pkCombatInfo->setPlot(&plot);

	// Unit vs. Unit
	CvAssert(pkDefender != NULL);

	int iAttackerStrength = kAttacker.GetMaxRangedCombatStrength(pkDefender, /*pCity*/ NULL, true, false);

	// Mod to air sweep strength
	iAttackerStrength *= (100 + kAttacker.GetAirSweepCombatModifier());
	iAttackerStrength /= 100;

	int iDefenderStrength = 0;

	int iDefenderExperience = 0;
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	bool bIsGlobalXPAwarded = !kAttacker.isBarbarian() && !GET_PLAYER(kAttacker.getOwner()).isMinorCiv() && !pkDefender->isBarbarian() && !GET_PLAYER(pkDefender->getOwner()).isMinorCiv();
#endif

	// Ground AA interceptor
	if(pkDefender->getDomainType() != DOMAIN_AIR)
	{
		int iInterceptionDamage = pkDefender->GetInterceptionDamage(&kAttacker);

		// Reduce damage for performing a sweep
		iInterceptionDamage *= /*40*/ GC.getAIR_SWEEP_INTERCEPTION_DAMAGE_MOD();
		iInterceptionDamage /= 100;

		iDefenderExperience = /*2*/ GC.getEXPERIENCE_DEFENDING_AIR_SWEEP_GROUND();

		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iInterceptionDamage);		// Damage inflicted this round
#ifdef AUI_UNIT_COMBAT_FIX_AIR_SWEEP_VS_GROUND_INTERCEPTOR
		int iTotalInterceptionDamage = iInterceptionDamage + kAttacker.getDamage();
		pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iTotalInterceptionDamage);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, 0);
		pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, pkDefender->getDamage());

		pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iDefenderExperience);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, pkDefender->maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == pkDefender->getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

		pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, 0);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, kAttacker.maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !pkDefender->isBarbarian());
#endif
#endif
	}
	// Air interceptor
	else
	{
		iDefenderStrength = pkDefender->GetMaxRangedCombatStrength(&kAttacker, /*pCity*/ NULL, false, false);
		iDefenderStrength *= (100 + pkDefender->GetAirSweepCombatModifier());
		iDefenderStrength /= 100;

		int iAttackerDamageInflicted = kAttacker.getCombatDamage(iAttackerStrength, iDefenderStrength, kAttacker.getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ false);
		int iDefenderDamageInflicted = pkDefender->getCombatDamage(iDefenderStrength, iAttackerStrength, pkDefender->getDamage(), /*bIncludeRand*/ true, /*bAttackerIsCity*/ false, /*bDefenderIsCity*/ false);

		int iAttackerTotalDamageInflicted = iAttackerDamageInflicted + pkDefender->getDamage();
		int iDefenderTotalDamageInflicted = iDefenderDamageInflicted + kAttacker.getDamage();

		// Will both units be killed by this? :o If so, take drastic corrective measures
		if(iAttackerTotalDamageInflicted >= iMaxHP && iDefenderTotalDamageInflicted >= iMaxHP)
		{
			// He who hath the least amount of damage survives with 1 HP left
			if(iAttackerTotalDamageInflicted > iDefenderTotalDamageInflicted)
			{
				iDefenderDamageInflicted = iMaxHP - kAttacker.getDamage() - 1;
				iDefenderTotalDamageInflicted = iMaxHP - 1;
				iAttackerTotalDamageInflicted = iMaxHP;
			}
			else
			{
				iAttackerDamageInflicted = iMaxHP - pkDefender->getDamage() - 1;
				iAttackerTotalDamageInflicted = iMaxHP - 1;
				iDefenderTotalDamageInflicted = iMaxHP;
			}
		}

		iDefenderExperience = /*6*/ GC.getEXPERIENCE_DEFENDING_AIR_SWEEP_AIR();

		pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, iDefenderTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, iAttackerDamageInflicted);
		pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, iAttackerTotalDamageInflicted);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, iDefenderDamageInflicted);

		// Fear Damage
		//pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_ATTACKER, kAttacker.getCombatDamage(iAttackerStrength, iDefenderStrength, kAttacker.getDamage(), true, false, true) );
		//	pkCombatInfo->setFearDamageInflicted( BATTLE_UNIT_DEFENDER, getCombatDamage(iDefenderStrength, iAttackerStrength, pDefender->getDamage(), true, false, true) );

#ifndef NQM_PRUNING
		int iAttackerEffectiveStrength = iAttackerStrength * (iMaxHP - range(kAttacker.getDamage(), 0, iMaxHP-1)) / iMaxHP;
		iAttackerEffectiveStrength = iAttackerEffectiveStrength > 0 ? iAttackerEffectiveStrength : 1;
		int iDefenderEffectiveStrength = iDefenderStrength * (iMaxHP - range(pkDefender->getDamage(), 0, iMaxHP-1)) / iMaxHP;
		iDefenderEffectiveStrength = iDefenderEffectiveStrength > 0 ? iDefenderEffectiveStrength : 1;
#endif

		//int iExperience = kAttacker.defenseXPValue();
		//iExperience = ((iExperience * iAttackerEffectiveStrength) / iDefenderEffectiveStrength); // is this right? looks like more for less [Jon: Yes, it's XP for the defender]
		//iExperience = range(iExperience, GC.getMIN_EXPERIENCE_PER_COMBAT(), GC.getMAX_EXPERIENCE_PER_COMBAT());
		pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, iDefenderExperience);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, pkDefender->maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() == pkDefender->getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

		//iExperience = ((iExperience * iDefenderEffectiveStrength) / iAttackerEffectiveStrength);
		//iExperience = range(iExperience, GC.getMIN_EXPERIENCE_PER_COMBAT(), GC.getMAX_EXPERIENCE_PER_COMBAT());
		int iExperience = /*6*/ GC.getEXPERIENCE_ATTACKING_AIR_SWEEP();
		pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, iExperience);
		pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, kAttacker.maxXPValue());
		pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, bIsGlobalXPAwarded);
#else
		pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, !pkDefender->isBarbarian());
#endif
	}

	pkCombatInfo->setAttackIsRanged(false);
	pkCombatInfo->setAttackIsAirSweep(true);
	pkCombatInfo->setAttackerAdvances(false);
	pkCombatInfo->setDefenderRetaliates(true);

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}

//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveAirSweep(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	// After combat stuff
	CvString strBuffer;
	bool bAttackerDead = false;
	bool bDefenderDead = false;

	CvUnit* pkAttacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);
	CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	if(!pkTargetPlot && pkDefender)
		pkTargetPlot = pkDefender->plot();

	CvAssert_Debug(pkAttacker && pkDefender && pkTargetPlot);

	// Internal variables
	int iAttackerDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_ATTACKER);
	int iDefenderDamageInflicted = kCombatInfo.getDamageInflicted(BATTLE_UNIT_DEFENDER);

	// Made interception!
	if(pkDefender)
	{
		pkDefender->setMadeInterception(true);
		if(pkAttacker && pkTargetPlot)
		{
			//One Hit
			if(pkDefender->GetCurrHitPoints() == GC.getMAX_HIT_POINTS() && pkAttacker->isHuman() && !GC.getGame().isGameMultiPlayer())
			{
				gDLL->UnlockAchievement(ACHIEVEMENT_ONEHITKILL);
			}

			pkDefender->changeDamage(iAttackerDamageInflicted, pkAttacker->getOwner());
			pkAttacker->changeDamage(iDefenderDamageInflicted, pkDefender->getOwner());

			// Update experience for both sides.
			pkDefender->changeExperience(
			    kCombatInfo.getExperience(BATTLE_UNIT_DEFENDER),
			    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_DEFENDER),
			    true,
			    kCombatInfo.getInBorders(BATTLE_UNIT_DEFENDER),
			    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_DEFENDER));

			pkAttacker->changeExperience(
			    kCombatInfo.getExperience(BATTLE_UNIT_ATTACKER),
			    kCombatInfo.getMaxExperienceAllowed(BATTLE_UNIT_ATTACKER),
			    true,
			    kCombatInfo.getInBorders(BATTLE_UNIT_ATTACKER),
			    kCombatInfo.getUpdateGlobal(BATTLE_UNIT_ATTACKER));

			// Anyone eat it?
			bAttackerDead = (pkAttacker->getDamage() >= GC.getMAX_HIT_POINTS());
			bDefenderDead = (pkDefender->getDamage() >= GC.getMAX_HIT_POINTS());

			int iActivePlayerID = GC.getGame().getActivePlayer();

			//////////////////////////////////////////////////////////////////////////

#ifndef AUI_UNIT_COMBAT_FIX_AIR_SWEEP_VS_GROUND_INTERCEPTOR
			// Ground AA interceptor
			if(pkDefender->getDomainType() != DOMAIN_AIR)
			{
				// Attacker died
				if(bAttackerDead)
				{
					auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
					gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());

					if(iActivePlayerID == pkAttacker->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", pkAttacker->getNameKey(), pkDefender->getNameKey(), iAttackerDamageInflicted, 0);
						GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}
					if(iActivePlayerID == pkDefender->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pkDefender->getNameKey(), iAttackerDamageInflicted, 0, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}
#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
					pkDefender->testPromotionReady();
#endif
				}
			}
			// Air AA interceptor
			else
#endif
			{
				// Attacker died
				if(bAttackerDead)
				{
					auto_ptr<ICvUnit1> pAttacker = GC.WrapUnitPointer(pkAttacker);
					gDLL->GameplayUnitDestroyedInCombat(pAttacker.get());

					if(iActivePlayerID == pkAttacker->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", pkAttacker->getNameKey(), pkDefender->getNameKey(), iAttackerDamageInflicted, 0);
						GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}
					if(iActivePlayerID == pkDefender->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pkDefender->getNameKey(), iAttackerDamageInflicted, 0, pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()));
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
					pkDefender->testPromotionReady();
#endif

					ApplyPostCombatTraitEffects(pkDefender, pkAttacker);
				}
				// Defender died
				else if(bDefenderDead)
				{
					auto_ptr<ICvUnit1> pDefender = GC.WrapUnitPointer(pkDefender);
					gDLL->GameplayUnitDestroyedInCombat(pDefender.get());

					if(iActivePlayerID == pkAttacker->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey());
						GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}
					if(iActivePlayerID == pkDefender->getOwner())
					{
						if(pkAttacker->getVisualOwner(pkDefender->getTeam()) != pkAttacker->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pkDefender->getNameKey(), pkAttacker->getNameKey(), iDefenderDamageInflicted);
						}
						else
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pkDefender->getNameKey(), pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()), iDefenderDamageInflicted);
						}
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*,GC.getEraInfo(GC.getGame().getCurrentEra())->getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}

					CvNotifications* pNotification = GET_PLAYER(pkDefender->getOwner()).GetNotifications();
					if(pNotification)
					{
						if(pkAttacker->getVisualOwner(pkDefender->getTeam()) != pkAttacker->getOwner())
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pkDefender->getNameKey(), pkAttacker->getNameKey(), iDefenderDamageInflicted);
						}
						else
						{
							strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pkDefender->getNameKey(), pkAttacker->getNameKey(), pkAttacker->getVisualCivAdjective(pkDefender->getTeam()), iDefenderDamageInflicted);
						}
						Localization::String strSummary = Localization::Lookup("TXT_KEY_UNIT_LOST");
						pNotification->Add(NOTIFICATION_UNIT_DIED, strBuffer, strSummary.toUTF8(), pkDefender->getX(), pkDefender->getY(), (int) pkDefender->getUnitType(), pkDefender->getOwner());
					}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
					pkAttacker->testPromotionReady();
#endif

					ApplyPostCombatTraitEffects(pkAttacker, pkDefender);
				}
				// Nobody died
				else
				{
					if(iActivePlayerID == pkAttacker->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
						GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}
					if(iActivePlayerID == pkDefender->getOwner())
					{
						strBuffer = GetLocalizedText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", pkAttacker->getNameKey(), iDefenderDamageInflicted, pkDefender->getNameKey(), iAttackerDamageInflicted);
						GC.messagePlayer(uiParentEventID, pkDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer/*, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pkTargetPlot->getX(), pkTargetPlot->getY()*/);
					}

#ifndef AUI_UNIT_TEST_PROMOTION_READY_MOVED
					pkDefender->testPromotionReady();
					pkAttacker->testPromotionReady();
#endif
				}
			}
		}
	}
	else
		bDefenderDead = true;

	// Clean up some stuff
	if(pkDefender)
	{
		pkDefender->setCombatUnit(NULL);
		pkDefender->ClearMissionQueue();
#ifdef NQM_UNIT_COMBAT_WITHDRAW_INTERCEPT_AFTER_SWEEP_IF_AT_OR_BELOW_TARGET_HEALTH
		if (pkDefender->GetActivityType() == ACTIVITY_INTERCEPT)
		{
			int iEffectiveDefenderHP = pkDefender->GetCurrHitPoints();
			if (pkDefender->isAlwaysHeal() && pkDefender->isOutOfInterceptions() && !GET_PLAYER(pkDefender->getOwner()).isEndTurn() && pkDefender->canHeal(pkDefender->plot()))
				iEffectiveDefenderHP += pkDefender->healRate(pkDefender->plot());
			if (iEffectiveDefenderHP * NQM_UNIT_COMBAT_WITHDRAW_INTERCEPT_AFTER_SWEEP_IF_AT_OR_BELOW_TARGET_HEALTH <= GC.getMAX_HIT_POINTS() * 100)
				pkDefender->SetActivityType(ACTIVITY_AWAKE);
		}
#endif
	}

	if(pkAttacker)
	{
		pkAttacker->setCombatUnit(NULL);
		pkAttacker->ClearMissionQueue(GetPostCombatDelay());

		// Spend a move for this attack
		pkAttacker->changeMoves(-GC.getMOVE_DENOMINATOR());

		// Can't move or attack again
#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
		if(!pkAttacker->canMoveAfterAttacking() && pkAttacker->isOutOfAttacks())
#else
		if(!pkAttacker->canMoveAfterAttacking())
#endif
		{
			pkAttacker->finishMoves();
		}

		// Report that combat is over in case we want to queue another attack
		GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved(pkAttacker, bDefenderDead);
	}
}

//	GenerateNuclearCombatInfo
//	Function: GenerateNuclearCombatInfo
//	Take the input parameters and fill in a CvCombatInfo definition assuming a
//	mission to do a nuclear attack.
//
//	Parameters:
//		pkDefender   	-	Defending unit.  Can be null, in which case the input plot must have a city
//		plot         	-	The plot of the defending unit/city
//		pkCombatInfo 	-	Output combat info
//	---------------------------------------------------------------------------
void CvUnitCombat::GenerateNuclearCombatInfo(CvUnit& kAttacker, CvPlot& plot, CvCombatInfo* pkCombatInfo)
{
	pkCombatInfo->setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
	pkCombatInfo->setUnit(BATTLE_UNIT_DEFENDER, NULL);
	pkCombatInfo->setPlot(&plot);

	//////////////////////////////////////////////////////////////////////

	CvString strBuffer;
	bool abTeamsAffected[MAX_TEAMS];
	int iI;
	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		abTeamsAffected[iI] = kAttacker.isNukeVictim(&plot, ((TeamTypes)iI));
	}

	int iPlotTeam = plot.getTeam();
	bool bWar = false;
	bool bBystander = false;

	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		if(abTeamsAffected[iI])
		{
			if(!kAttacker.isEnemy((TeamTypes)iI))
			{
				GET_TEAM(kAttacker.getTeam()).declareWar(((TeamTypes)iI));

				if (iPlotTeam == iI) 
				{
					bWar = true;
				} 
				else 
				{
					bBystander = true;
				}
			}
		}
	}

	ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
	if (pkScriptSystem) 
	{	
		CvLuaArgsHandle args;

		args->Push(kAttacker.getOwner());
		args->Push(plot.getX());
		args->Push(plot.getY());
		args->Push(bWar);
		args->Push(bBystander);

		bool bResult;
		LuaSupport::CallHook(pkScriptSystem, "NuclearDetonation", args.get(), bResult);
	}

	kAttacker.setReconPlot(&plot);

	//////////////////////////////////////////////////////////////////////

	pkCombatInfo->setFinalDamage(BATTLE_UNIT_ATTACKER, 0);		// Total damage to the unit
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_ATTACKER, 0);	// Damage inflicted this round
	pkCombatInfo->setFinalDamage(BATTLE_UNIT_DEFENDER, 0);		// Total damage to the unit
	pkCombatInfo->setDamageInflicted(BATTLE_UNIT_DEFENDER, 0);	// Damage inflicted this round

	pkCombatInfo->setFearDamageInflicted(BATTLE_UNIT_ATTACKER, 0);

	pkCombatInfo->setExperience(BATTLE_UNIT_ATTACKER, 0);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_ATTACKER, 0);
	pkCombatInfo->setInBorders(BATTLE_UNIT_ATTACKER, plot.getOwner() != kAttacker.getOwner());	// Not really correct
#ifdef NQ_NO_GG_POINTS_FROM_CS_OR_BARBS
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, false); // Since experience earned is 0 anyway, no need to bother
#else
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_ATTACKER, !kAttacker.isBarbarian());
#endif

	pkCombatInfo->setExperience(BATTLE_UNIT_DEFENDER, 0);
	pkCombatInfo->setMaxExperienceAllowed(BATTLE_UNIT_DEFENDER, 0);
	pkCombatInfo->setInBorders(BATTLE_UNIT_DEFENDER, plot.getOwner() == kAttacker.getOwner());
	pkCombatInfo->setUpdateGlobal(BATTLE_UNIT_DEFENDER, false);

	pkCombatInfo->setAttackIsBombingMission(true);
	pkCombatInfo->setDefenderRetaliates(false);

	CvUnit* interceptor = getMissileInterceptor(kAttacker, plot);
	bool wasShotDown = interceptor != NULL;
	int iDamageMembers = 0;
	if (wasShotDown)
	{
		pkCombatInfo->setUnit(BATTLE_UNIT_INTERCEPTOR, interceptor);
		pkCombatInfo->setDamageInflicted(BATTLE_UNIT_INTERCEPTOR, kAttacker.GetMaxHitPoints());
	}
	else
	{
		pkCombatInfo->setAttackNuclearLevel(kAttacker.GetNukeDamageLevel() + 1);
		// Set all of the units in the blast radius to defenders and calculate their damage
		GenerateNuclearExplosionDamage(&plot, kAttacker.GetNukeDamageLevel(), &kAttacker, pkCombatInfo->getDamageMembers(), &iDamageMembers, pkCombatInfo->getMaxDamageMemberCount());
	}
	pkCombatInfo->setDamageMemberCount(iDamageMembers);

	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
}

//	-------------------------------------------------------------------------------------
uint CvUnitCombat::ApplyNuclearExplosionDamage(uint uiParentEventID, CvPlot* pkTargetPlot, int iDamageLevel, CvUnit* /* pkAttacker = NULL*/)
{
	CvCombatMemberEntry kDamageMembers[MAX_NUKE_DAMAGE_MEMBERS];
	int iDamageMembers = 0;
	GenerateNuclearExplosionDamage(pkTargetPlot, iDamageLevel, NULL, &kDamageMembers[0], &iDamageMembers, MAX_NUKE_DAMAGE_MEMBERS);
	return ApplyNuclearExplosionDamage(uiParentEventID, &kDamageMembers[0], iDamageMembers, NULL, pkTargetPlot, iDamageLevel);
}

// find how much population to remove from this city
int getNukePopulationDamage(CvCity* nukedCity, int dist)
{
	float distanceReduction = 0.50f;

	float nukePopPercentModifier = max(0.0f, nukedCity->getNukeModifier() / 100.0f + 1.0f);
	float nukePopPercentBase = GC.getNUKE_LEVEL1_POPULATION_DEATH_BASE() / 100.0f;
	float nukePopPercentRand = GC.rand(GC.getNUKE_LEVEL1_POPULATION_DEATH_RAND_1(), "Pop Nuked 1") / 100.0f;
	float percentToRemove = pow(distanceReduction, dist) * nukePopPercentModifier * (nukePopPercentBase + nukePopPercentRand);

	int startPop = nukedCity->getPopulation();
	int popToRemove = startPop * percentToRemove;
	popToRemove = max(1, popToRemove); // remove at least 1 pop
	return min((startPop - 1), popToRemove); // don't go lower than 1 pop
}

// damage done to units from a nuke
int getNukeUnitDamage(int dist, CvCity* cityOnHex)
{
	float distanceReduction = 0.50f;

	int damage = GC.getNUKE_UNIT_DAMAGE_BASE() + GC.rand(GC.getNUKE_UNIT_DAMAGE_RAND_1(), "Nuke Damage 1");
	damage *= pow(distanceReduction, dist);

	if (cityOnHex) // do less damage to units in a city
	{
		float cityModifier = max(0.0f, cityOnHex->getNukeModifier() / 100.0f + 1.0f); // like 0.25 if modifier was -75
		damage *= cityModifier * distanceReduction; // reduce damage for units in a city by 1 distance always
	}
	return damage;
}

// find how many hitpoints should be removed from this city
int getNukeCityHitpointDamage(CvCity* nukedCity, int dist)
{
	float distanceReduction = 0.50f;

	int damage = (nukedCity->GetMaxHitPoints() - nukedCity->getDamage()) * (GC.getNUKE_CITY_HIT_POINT_DAMAGE() / 100.0f);
	damage *= pow(distanceReduction, dist);

	return damage;
}

//	-------------------------------------------------------------------------------------
uint CvUnitCombat::ApplyNuclearExplosionDamage(uint uiParentEventID, const CvCombatMemberEntry* pkDamageArray, int iDamageMembers, CvUnit* pkAttacker, CvPlot* pkTargetPlot, int iDamageLevel)
{
	CvString attackerName = pkAttacker->getVisualCivAdjective(pkAttacker->getTeam());

	uint uiOpposingDamageCount = 0;
	PlayerTypes eAttackerOwner = pkAttacker?pkAttacker->getOwner():NO_PLAYER;

	{
		string message = "Your {1_UnitName} has struck its target.";
		CvString messageFull = GetLocalizedText(message.c_str(), pkAttacker->getNameKey());
		GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), messageFull);
	}

	// Do all the units first
	for(int i = 0; i < iDamageMembers; ++i)
	{
		const CvCombatMemberEntry& kEntry = pkDamageArray[i];
		if(kEntry.IsUnit())
		{
			CvUnit* pkUnit = GET_PLAYER(kEntry.GetPlayer()).getUnit(kEntry.GetUnitID());
			if(pkUnit)
			{
				{
					string message = "Your {1_UnitName} took {2_damage} damage from a {3_civ} nuclear weapon.";
					CvString messageFull = GetLocalizedText(message.c_str(), pkUnit->getNameKey(), kEntry.GetDamage(), attackerName);
					GC.messagePlayer(uiParentEventID, pkUnit->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), messageFull);
				}

				// Apply the damage
				pkUnit->setCombatUnit(NULL);
				pkUnit->ClearMissionQueue();
				pkUnit->SetAutomateType(NO_AUTOMATE); // kick unit out of automation

				if((eAttackerOwner == NO_PLAYER || pkUnit->getOwner() != eAttackerOwner) && !pkUnit->isBarbarian())
					uiOpposingDamageCount++;	// Count the number of non-barbarian opposing units

				if(pkUnit->IsCombatUnit() || pkUnit->IsCanAttackRanged())
				{
					pkUnit->changeDamage(kEntry.GetDamage(), eAttackerOwner);
				}
				else if(kEntry.GetDamage() >= /*6*/ GC.getNUKE_NON_COMBAT_DEATH_THRESHOLD())
				{
					pkUnit->kill(false, eAttackerOwner);
				}

#ifdef AUI_WARNING_FIXES
				if (pkAttacker)
#endif
				GET_PLAYER(kEntry.GetPlayer()).GetDiplomacyAI()->ChangeNumTimesNuked(pkAttacker->getOwner(), 1);
			}
		}
	}

	// Then the terrain effects
	int iBlastRadius = GC.getNUKE_BLAST_RADIUS();

#ifdef AUI_HEXSPACE_DX_LOOPS
	int iMaxDX, iDX;
	CvPlot* pLoopPlot;
	if (pkTargetPlot)
	{
		for (int iDY = -iBlastRadius; iDY <= iBlastRadius; iDY++)
		{
			iMaxDX = iBlastRadius - MAX(0, iDY);
			for (iDX = -iBlastRadius - MIN(0, iDY); iDX <= iMaxDX; iDX++) // MIN() and MAX() stuff is to reduce loops (hexspace!)
			{
				// No need for range check because loops are set up properly
				pLoopPlot = plotXY(pkTargetPlot->getX(), pkTargetPlot->getY(), iDX, iDY);
#else
	for(int iDX = -(iBlastRadius); iDX <= iBlastRadius; iDX++)
	{
		for(int iDY = -(iBlastRadius); iDY <= iBlastRadius; iDY++)
		{
			CvPlot* pLoopPlot = plotXYWithRangeCheck(pkTargetPlot->getX(), pkTargetPlot->getY(), iDX, iDY, iBlastRadius);
#endif

			if(pLoopPlot != NULL)
			{
				// if we remove roads, don't remove them on the city... XXX
				CvCity* pLoopCity = pLoopPlot->getPlotCity();

				if(pLoopCity == NULL)
				{
					if(!(pLoopPlot->isWater()) && !(pLoopPlot->isImpassable()))
					{
						if(pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							CvFeatureInfo* pkFeatureInfo = GC.getFeatureInfo(pLoopPlot->getFeatureType());
							if(pkFeatureInfo && !pkFeatureInfo->isNukeImmune())
							{
								if(pLoopPlot == pkTargetPlot || GC.getGame().getJonRandNum(100, "Nuke Fallout") < GC.getNUKE_FALLOUT_PROB())
								{
									if(pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
									{
										pLoopPlot->SetImprovementPillaged(true);
									}
									pLoopPlot->setFeatureType((FeatureTypes)(GC.getNUKE_FEATURE()));
								}
							}
						}
						else
						{
							if(pLoopPlot == pkTargetPlot || GC.getGame().getJonRandNum(100, "Nuke Fallout") < GC.getNUKE_FALLOUT_PROB())
							{
								if(pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
								{
									pLoopPlot->SetImprovementPillaged(true);
								}
								pLoopPlot->setFeatureType((FeatureTypes)(GC.getNUKE_FEATURE()));
							}
						}
					}
				}
			}
		}
	}
#ifdef AUI_HEXSPACE_DX_LOOPS
	}
#endif

	// Then the cities
	for(int i = 0; i < iDamageMembers; ++i)
	{
		const CvCombatMemberEntry& kEntry = pkDamageArray[i];
		if(kEntry.IsCity())
		{
			CvCity* pkCity = GET_PLAYER(kEntry.GetPlayer()).getCity(kEntry.GetCityID());
			if(pkCity)
			{
				int damage = kEntry.GetFinalDamage();
				{
					string message = "The City of {1_UnitName} was hit by a {2_attacker} nuclear weapon and took {3_damage} damage!";
					CvString messageFull = GetLocalizedText(message.c_str(), pkCity->getNameKey(), attackerName, damage);
					GC.messagePlayer(uiParentEventID, pkAttacker->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), messageFull);
				}

				int dist = hexDistance(pkCity->plot()->getX() - pkTargetPlot->getX(), pkCity->plot()->getY() - pkTargetPlot->getY());
				pkCity->setCombatUnit(NULL);

				if(eAttackerOwner == NO_PLAYER || pkCity->getOwner() != eAttackerOwner)
					uiOpposingDamageCount++;

				if(damage >= pkCity->GetMaxHitPoints() && !pkCity->IsOriginalCapital())
				{
					{
						string message = "{1_city} was completely destroyed in the nuclear attack!";
						CvString messageFull = GetLocalizedText(message.c_str(), pkCity->getNameKey());
						GC.messagePlayer(uiParentEventID, pkCity->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), messageFull);
					}
					auto_ptr<ICvCity1> pkDllCity(new CvDllCity(pkCity));
					gDLL->GameplayCitySetDamage(pkDllCity.get(), 0, pkCity->getDamage()); // to stop the fires
					gDLL->GameplayCityDestroyed(pkDllCity.get(), NO_PLAYER);

					PlayerTypes eOldOwner = pkCity->getOwner();
					pkCity->kill();

					// slewis - check for killing a player
#ifdef AUI_WARNING_FIXES
					if (pkAttacker)
#endif
					GET_PLAYER(pkAttacker->getOwner()).CheckForMurder(eOldOwner);
				}
				else
				{
					int popToRemove = getNukePopulationDamage(pkCity, dist);
					pkCity->changePopulation(-popToRemove);
					if (popToRemove > 0)
					{
						string message = "{1_city} lost {2_pop} [ICON_CITIZEN] million inhabitants in the nuclear attack!";
						CvString messageFull = GetLocalizedText(message.c_str(), pkCity->getNameKey(), popToRemove);
						GC.messagePlayer(uiParentEventID, pkCity->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), messageFull);
					}

					// Add damage to the city
					pkCity->setDamage(damage);

#ifdef AUI_WARNING_FIXES
					if (pkAttacker)
#endif
					GET_PLAYER(pkCity->getOwner()).GetDiplomacyAI()->ChangeNumTimesNuked(pkAttacker->getOwner(), 1);
				}
			}
		}
	}
	return uiOpposingDamageCount;
}

//	-------------------------------------------------------------------------------------
//	Generate nuclear explosion damage for all the units and cities in the radius of the specified plot.
//	The attacker is optional, this is also called for a meltdown
void CvUnitCombat::GenerateNuclearExplosionDamage(CvPlot* pkTargetPlot, int iDamageLevel, CvUnit* pkAttacker, CvCombatMemberEntry* pkDamageArray, int* piDamageMembers, int iMaxDamageMembers)
{
	int iBlastRadius = GC.getNUKE_BLAST_RADIUS();

	*piDamageMembers = 0;

#ifdef AUI_HEXSPACE_DX_LOOPS
	int iMaxDX, iDX;
	CvPlot* pLoopPlot;
	for (int iDY = -iBlastRadius; iDY <= iBlastRadius; iDY++)
	{
		iMaxDX = iBlastRadius - MAX(0, iDY);
		for (iDX = -iBlastRadius - MIN(0, iDY); iDX <= iMaxDX; iDX++) // MIN() and MAX() stuff is to reduce loops (hexspace!)
		{
			// No need for range check because loops are set up properly
			pLoopPlot = plotXY(pkTargetPlot->getX(), pkTargetPlot->getY(), iDX, iDY);
#else
	for(int iDX = -(iBlastRadius); iDX <= iBlastRadius; iDX++)
	{
		for(int iDY = -(iBlastRadius); iDY <= iBlastRadius; iDY++)
		{
			CvPlot* pLoopPlot = plotXYWithRangeCheck(pkTargetPlot->getX(), pkTargetPlot->getY(), iDX, iDY, iBlastRadius);
#endif
			int dist = hexDistance(iDX, iDY);

			if(pLoopPlot != NULL)
			{
				CvCity* pLoopCity = pLoopPlot->getPlotCity();

				FFastSmallFixedList<IDInfo, 25, true, c_eCiv5GameplayDLL > oldUnits;
				IDInfo* pUnitNode = pLoopPlot->headUnitNode();

				while(pUnitNode != NULL)
				{
					oldUnits.insertAtEnd(pUnitNode);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
				}

				pUnitNode = oldUnits.head();

				while(pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(*pUnitNode);
					pUnitNode = oldUnits.next(pUnitNode);

					if(pLoopUnit != NULL)
					{
						if(pLoopUnit != pkAttacker)
						{
							if(!pLoopUnit->isNukeImmune() && !pLoopUnit->isDelayedDeath())
							{
								int iNukeDamage = getNukeUnitDamage(dist, pLoopCity);

								CvCombatMemberEntry* pkDamageEntry = AddCombatMember(pkDamageArray, piDamageMembers, iMaxDamageMembers, pLoopUnit);
								if(pkDamageEntry)
								{
									pkDamageEntry->SetDamage(iNukeDamage);
									pkDamageEntry->SetFinalDamage(std::min(iNukeDamage + pLoopUnit->getDamage(), GC.getMAX_HIT_POINTS()));
									pkDamageEntry->SetMaxHitPoints(GC.getMAX_HIT_POINTS());
									if(pkAttacker)
										pLoopUnit->setCombatUnit(pkAttacker);
								}
								else
								{
									CvAssertMsg(*piDamageMembers < iMaxDamageMembers, "Ran out of entries for the nuclear damage array");
								}
							}
						}
					}
				}

				if(pLoopCity != NULL)
				{
					bool bKillCity = false;

					// Is the city wiped out? - no capitals!
					if(!pLoopCity->IsOriginalCapital())
					{
						if(iDamageLevel > 2)
						{
							bKillCity = true;
						}
						else if(iDamageLevel > 1)
						{
							if(pLoopCity->getPopulation() < /*5*/ GC.getNUKE_LEVEL2_ELIM_POPULATION_THRESHOLD())
							{
								bKillCity = true;
							}
						}
					}

					int iTotalDamage;
					if(bKillCity)
					{
						iTotalDamage = pLoopCity->GetMaxHitPoints();
					}
					else
					{
						// Add damage to the city
						int netDamage = getNukeCityHitpointDamage(pLoopCity, dist);
						iTotalDamage = netDamage + pLoopCity->getDamage();

						// Can't bring a city below 1 HP
						iTotalDamage = min(iTotalDamage, pLoopCity->GetMaxHitPoints() - 1);
					}

					CvCombatMemberEntry* pkDamageEntry = AddCombatMember(pkDamageArray, piDamageMembers, iMaxDamageMembers, pLoopCity);
					if(pkDamageEntry)
					{
						pkDamageEntry->SetDamage(iTotalDamage - pLoopCity->getDamage());
						pkDamageEntry->SetFinalDamage(iTotalDamage);
						pkDamageEntry->SetMaxHitPoints(pLoopCity->GetMaxHitPoints());

						if(pkAttacker)
							pLoopCity->setCombatUnit(pkAttacker);
					}
					else
					{
						CvAssertMsg(*piDamageMembers < iMaxDamageMembers, "Ran out of entries for the nuclear damage array");
					}
				}
			}
		}
	}
}

//	---------------------------------------------------------------------------
//	Function: ResolveNuclearCombat
//	Resolve combat from a nuclear attack.
void CvUnitCombat::ResolveNuclearCombat(const CvCombatInfo& kCombatInfo, uint uiParentEventID)
{
	UNREFERENCED_PARAMETER(uiParentEventID);

	CvUnit* pkAttacker = kCombatInfo.getUnit(BATTLE_UNIT_ATTACKER);
	CvAssert_Debug(pkAttacker);

	CvPlot* pkTargetPlot = kCombatInfo.getPlot();
	CvAssert_Debug(pkTargetPlot);

	CvString strBuffer;


	if(pkAttacker)
	{
		CvUnit* interceptor = kCombatInfo.getUnit(BATTLE_UNIT_INTERCEPTOR);
		bool wasShotDown = interceptor != NULL;
		if (wasShotDown)
		{
			// dead code
			// the interception code switches the combat to non nuclear, so it resolves in the standard air resolver function
		}
		else
		{
			// Make sure we are disconnected from any unit transporting the attacker (i.e. its a missile)
			pkAttacker->setTransportUnit(NULL);

			if (pkTargetPlot)
			{
				GC.getGame().changeNukesExploded(1);
				if (ApplyNuclearExplosionDamage(uiParentEventID, kCombatInfo.getDamageMembers(), kCombatInfo.getDamageMemberCount(), pkAttacker, pkTargetPlot, kCombatInfo.getAttackNuclearLevel() - 1) > 0)
				{
					if (pkAttacker->getOwner() == GC.getGame().getActivePlayer())
					{
						// Must damage someone to get the achievement.
						gDLL->UnlockAchievement(ACHIEVEMENT_DROP_NUKE);

						if (GC.getGame().getGameTurnYear() == 2012)
						{
							CvPlayerAI& kPlayer = GET_PLAYER(GC.getGame().getActivePlayer());
							if (strncmp(kPlayer.getCivilizationTypeKey(), "CIVILIZATION_MAYA", 32) == 0)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP1_36);
							}
						}
					}
				}
			}

			// Suicide Unit (currently all nuclear attackers are)
			if (pkAttacker->isSuicide())
			{
				pkAttacker->setCombatUnit(NULL);	// Must clear this if doing a delayed kill, should this be part of the kill method?
				pkAttacker->setAttackPlot(NULL, false);
				pkAttacker->kill(true);
			}
			else
			{
				CvAssertMsg(pkAttacker->isSuicide(), "A nuke unit that is not a one time use?");

				// Clean up some stuff
				pkAttacker->setCombatUnit(NULL);
				pkAttacker->ClearMissionQueue(GetPostCombatDelay());
				pkAttacker->SetAutomateType(NO_AUTOMATE); // kick unit out of automation

				// Spend a move for this attack
				pkAttacker->changeMoves(-GC.getMOVE_DENOMINATOR());

				// Can't move or attack again
#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
				if (!pkAttacker->canMoveAfterAttacking() && pkAttacker->isOutOfAttacks())
#else
				if (!pkAttacker->canMoveAfterAttacking())
#endif
				{
					pkAttacker->finishMoves();
				}
			}

			// Report that combat is over in case we want to queue another attack
			GET_PLAYER(pkAttacker->getOwner()).GetTacticalAI()->CombatResolved(pkAttacker, true);
		}
	}
}

//	---------------------------------------------------------------------------
void CvUnitCombat::ResolveCombat(const CvCombatInfo& kInfo, uint uiParentEventID /* = 0 */)
{
	PlayerTypes eAttackingPlayer = NO_PLAYER;
	// Restore visibility
	CvUnit* pAttacker = kInfo.getUnit(BATTLE_UNIT_ATTACKER);

	const TeamTypes eActiveTeam = GC.getGame().getActiveTeam();

	if(pAttacker)
	{
		eAttackingPlayer = pAttacker->getOwner();
		auto_ptr<ICvUnit1> pDllUnit(new CvDllUnit(pAttacker));
		gDLL->GameplayUnitVisibility(pDllUnit.get(), !pAttacker->isInvisible(eActiveTeam, false));
	}

	CvUnit* pDefender = kInfo.getUnit(BATTLE_UNIT_DEFENDER);
	if(pDefender)
	{
		auto_ptr<ICvUnit1> pDllUnit(new CvDllUnit(pDefender));
		gDLL->GameplayUnitVisibility(pDllUnit.get(), !pDefender->isInvisible(eActiveTeam, false));
	}

	CvUnit* pDefenderSupport = kInfo.getUnit(BATTLE_UNIT_INTERCEPTOR);
	if(pDefenderSupport)
	{
		auto_ptr<ICvUnit1> pDllUnit(new CvDllUnit(pDefenderSupport));
		gDLL->GameplayUnitVisibility(pDllUnit.get(), !pDefenderSupport->isInvisible(eActiveTeam, false));
	}
	// Nuclear Mission
	if(kInfo.getAttackIsNuclear()) //  && kInfo.getUnit(BATTLE_UNIT_INTERCEPTOR) != NULL
	{
		ResolveNuclearCombat(kInfo, uiParentEventID);
	}

	// Bombing Mission
	else if(kInfo.getAttackIsBombingMission())
	{
		ResolveAirUnitVsCombat(kInfo, uiParentEventID);
	}

	// Air Sweep Mission
	else if(kInfo.getAttackIsAirSweep())
	{
		ResolveAirSweep(kInfo, uiParentEventID);
	}

	// Ranged Attack
	else if(kInfo.getAttackIsRanged())
	{
		if(kInfo.getUnit(BATTLE_UNIT_ATTACKER))
		{
			ResolveRangedUnitVsCombat(kInfo, uiParentEventID);
			CvPlot *pPlot = kInfo.getPlot();
			if (kInfo.getUnit(BATTLE_UNIT_ATTACKER) && kInfo.getUnit(BATTLE_UNIT_DEFENDER) && pPlot)
			{
				pPlot->AddArchaeologicalRecord(CvTypes::getARTIFACT_BATTLE_RANGED(), kInfo.getUnit(BATTLE_UNIT_ATTACKER)->getOwner(), kInfo.getUnit(BATTLE_UNIT_DEFENDER)->getOwner());
			}
		}
		else
		{
			ResolveRangedCityVsUnitCombat(kInfo, uiParentEventID);
			CvPlot *pPlot = kInfo.getPlot();
			if (kInfo.getCity(BATTLE_UNIT_ATTACKER) && kInfo.getUnit(BATTLE_UNIT_DEFENDER) && pPlot)
			{
				pPlot->AddArchaeologicalRecord(CvTypes::getARTIFACT_BATTLE_RANGED(), kInfo.getCity(BATTLE_UNIT_ATTACKER)->getOwner(), kInfo.getUnit(BATTLE_UNIT_DEFENDER)->getOwner());
			}
		}
	}

	// Melee Attack
	else
	{
		if(kInfo.getCity(BATTLE_UNIT_DEFENDER))
		{
			ResolveCityMeleeCombat(kInfo, uiParentEventID);
		}
		else
		{
			ResolveMeleeCombat(kInfo, uiParentEventID);
			CvPlot *pPlot = kInfo.getPlot();
			if (kInfo.getUnit(BATTLE_UNIT_ATTACKER) && kInfo.getUnit(BATTLE_UNIT_DEFENDER) && pPlot)
			{
				pPlot->AddArchaeologicalRecord(CvTypes::getARTIFACT_BATTLE_MELEE(), kInfo.getUnit(BATTLE_UNIT_ATTACKER)->getOwner(), kInfo.getUnit(BATTLE_UNIT_DEFENDER)->getOwner());
			}
		}
	}

	// Clear popup blocking after combat resolves
	if(eAttackingPlayer == GC.getGame().getActivePlayer())
	{
		GC.GetEngineUserInterface()->SetDontShowPopups(false);
	}
}

//	----------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::Attack(CvUnit& kAttacker, CvPlot& targetPlot, ATTACK_OPTION eOption)
{
	CvString strBuffer;

	//VALIDATE_OBJECT
	CvAssert(kAttacker.canMoveInto(targetPlot, CvUnit::MOVEFLAG_ATTACK | CvUnit::MOVEFLAG_PRETEND_CORRECT_EMBARK_STATE));
	CvAssert(kAttacker.getCombatTimer() == 0);

	CvUnitCombat::ATTACK_RESULT eResult = CvUnitCombat::ATTACK_ABORTED;

	CvAssert(kAttacker.getCombatTimer() == 0);
	//	CvAssert(pDefender != NULL);
	CvAssert(!kAttacker.isFighting());

	// Unit that attacks loses his Fort bonus
	kAttacker.setFortifyTurns(0);

	UnitHandle pDefender;
	pDefender = targetPlot.getBestDefender(NO_PLAYER, kAttacker.getOwner(), &kAttacker, true);

	// JAR - without pDefender, nothing in here is going to work, just crash
	if(!pDefender)
	{
		return eResult;
	}

	kAttacker.SetAutomateType(NO_AUTOMATE);
	pDefender->SetAutomateType(NO_AUTOMATE);

	// slewis - tutorial'd
	if(kAttacker.getOwner() == GC.getGame().getActivePlayer())
	{
		GC.getGame().SetEverAttackedTutorial(true);
	}
	// end tutorial'd

	// handle the Zulu special thrown spear first attack
	ATTACK_RESULT eFireSupportResult = ATTACK_ABORTED;
	if (kAttacker.isRangedSupportFire() && pDefender->IsCanDefend())
	{
		eFireSupportResult = AttackRanged(kAttacker, pDefender->getX(), pDefender->getY(), CvUnitCombat::ATTACK_OPTION_NO_DEFENSIVE_SUPPORT);
		if (pDefender->isDelayedDeath())
		{
			// Killed him, move to the plot if we can.
			if(targetPlot.getNumVisibleEnemyDefenders(&kAttacker) == 0)
			{
				if (kAttacker.UnitMove(&targetPlot, true, &kAttacker, true))
					kAttacker.finishMoves();	// Burn all the moves we have
			}
			return eFireSupportResult;
		}
	}

	CvCombatInfo kCombatInfo;
	GenerateMeleeCombatInfo(kAttacker, pDefender.pointer(), targetPlot, &kCombatInfo);

	CvAssertMsg(!kAttacker.isDelayedDeath() && !pDefender->isDelayedDeath(), "Trying to battle and one of the units is already dead!");

#ifdef AUI_UNIT_FIX_NO_RETREAT_ON_CIVILIAN_GUARD
	if (pDefender->getExtraWithdrawal() > 0 && pDefender->CanWithdrawFromMelee(kAttacker, &kCombatInfo))
#else
	if(pDefender->getExtraWithdrawal() > 0 && pDefender->CanWithdrawFromMelee(kAttacker))
#endif
	{
		pDefender->DoWithdrawFromMelee(kAttacker);

		if(kAttacker.getOwner() == GC.getGame().getActivePlayer())
		{
			strBuffer = GetLocalizedText("TXT_KEY_MISC_ENEMY_UNIT_WITHDREW", pDefender->getNameKey());
			GC.messagePlayer(0, kAttacker.getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer);
		}
		else if(pDefender->getOwner() == GC.getGame().getActivePlayer())
		{
			strBuffer = GetLocalizedText("TXT_KEY_MISC_FRIENDLY_UNIT_WITHDREW", pDefender->getNameKey());
			GC.messagePlayer(0, pDefender->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), strBuffer);
		}

		// Move forward
		if(targetPlot.getNumVisibleEnemyDefenders(&kAttacker) == 0)
		{
			kAttacker.UnitMove(&targetPlot, true, &kAttacker);
		}

//		kAttacker.setMadeAttack(true);   /* EFB: Doesn't work, causes tactical AI to not dequeue this attack; but we've decided you don't lose your attack anyway */
		eResult = ATTACK_COMPLETED;
	}

	else if(!pDefender->IsCanDefend())
	{
		CvMissionDefinition kMission;
		kMission.setMissionTime(kAttacker.getCombatTimer() * gDLL->getSecsPerTurn());
		kMission.setMissionType(CvTypes::getMISSION_SURRENDER());
		kMission.setUnit(BATTLE_UNIT_ATTACKER, &kAttacker);
		kMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender.pointer());
		kMission.setPlot(&targetPlot);

		// Surrender mission
		CvMissionInfo* pkSurrenderMission = GC.getMissionInfo(CvTypes::getMISSION_SURRENDER());
		if(pkSurrenderMission == NULL)
		{
			CvAssert(false);
		}
		else
		{
			kAttacker.setCombatTimer(pkSurrenderMission->getTime());
		}

		// Kill them!
		pDefender->setDamage(GC.getMAX_HIT_POINTS());

		Localization::String strMessage;
		Localization::String strSummary;

		// Some units can't capture civilians. Embarked units are also not captured, they're simply killed. And some aren't a type that gets captured.
		if(!kAttacker.isNoCapture() && (!pDefender->isEmbarked() || pDefender->getUnitInfo().IsCaptureWhileEmbarked()) && pDefender->getCaptureUnitType(GET_PLAYER(pDefender->getOwner()).getCivilizationType()) != NO_UNIT)
		{
			pDefender->setCapturingPlayer(kAttacker.getOwner());

			if(kAttacker.isBarbarian())
			{
				strMessage = Localization::Lookup("TXT_KEY_UNIT_CAPTURED_BARBS_DETAILED");
				strMessage << pDefender->getUnitInfo().GetTextKey() << GET_PLAYER(kAttacker.getOwner()).getNameKey();
				strSummary = Localization::Lookup("TXT_KEY_UNIT_CAPTURED_BARBS");
			}
			else
			{
				strMessage = Localization::Lookup("TXT_KEY_UNIT_CAPTURED_DETAILED");
				strMessage << pDefender->getUnitInfo().GetTextKey();
				strSummary = Localization::Lookup("TXT_KEY_UNIT_CAPTURED");
			}
		}
		// Unit was killed instead
		else
		{
			strMessage = Localization::Lookup("TXT_KEY_UNIT_LOST");
			strSummary = strMessage;
		}

		CvNotifications* pNotification = GET_PLAYER(pDefender->getOwner()).GetNotifications();
		if(pNotification)
			pNotification->Add(NOTIFICATION_UNIT_DIED, strMessage.toUTF8(), strSummary.toUTF8(), pDefender->getX(), pDefender->getY(), (int) pDefender->getUnitType(), pDefender->getOwner());

		bool bAdvance;
		bAdvance = kAttacker.canAdvance(targetPlot, ((pDefender->IsCanDefend()) ? 1 : 0));

		// Move forward
		if(targetPlot.getNumVisibleEnemyDefenders(&kAttacker) == 0)
		{
			kAttacker.UnitMove(&targetPlot, true, ((bAdvance) ? &kAttacker : NULL));
		}

		// KWG: Should this be called? The defender is killed above and the unit.
		//      If anything, the above code should be put in the ResolveCombat method.
		ResolveCombat(kCombatInfo);
		eResult = ATTACK_COMPLETED;
	}
	else
	{
		ATTACK_RESULT eSupportResult = ATTACK_ABORTED;
		if(eOption != ATTACK_OPTION_NO_DEFENSIVE_SUPPORT)
		{
			// Ranged fire support from artillery units
			CvUnit* pFireSupportUnit = GetFireSupportUnit(pDefender->getOwner(), pDefender->getX(), pDefender->getY(), kAttacker.getX(), kAttacker.getY());
			if(pFireSupportUnit != NULL)
			{
				CvAssertMsg(!pFireSupportUnit->isDelayedDeath(), "Supporting battle unit is already dead!");
				eSupportResult = AttackRanged(*pFireSupportUnit, kAttacker.getX(), kAttacker.getY(), CvUnitCombat::ATTACK_OPTION_NO_DEFENSIVE_SUPPORT);
				// Turn off Fortify Turns, as this is the trigger for whether or not a ranged Unit can provide support fire (in addition to hasMadeAttack)
				pFireSupportUnit->setFortifyTurns(0);
			}

			if(eSupportResult == ATTACK_QUEUED)
			{
				// The supporting unit has queued their attack (against the attacker), we must have the attacker queue its attack.
				// Also, flag the current mission that the next time through, the defender doesn't get any defensive support.
				const_cast<MissionData*>(kAttacker.GetHeadMissionData())->iFlags |= MISSION_MODIFIER_NO_DEFENSIVE_SUPPORT;
				CvUnitMission::WaitFor(&kAttacker, pFireSupportUnit);
				eResult = ATTACK_QUEUED;
			}
		}

		if(eResult != ATTACK_QUEUED)
		{
			kAttacker.setMadeAttack(true);

			uint uiParentEventID = 0;
			// Send the combat message if the target plot is visible.
			bool isTargetVisibleToActivePlayer = targetPlot.isActiveVisible(false);
			bool quickCombat = CvPreGame::quickCombat();
			if(!quickCombat)
			{
				// Center camera here!
				if(isTargetVisibleToActivePlayer)
				{
					auto_ptr<ICvPlot1> pDefenderPlot = GC.WrapPlotPointer(pDefender->plot());
					GC.GetEngineUserInterface()->lookAt(pDefenderPlot.get(), CAMERALOOKAT_NORMAL);
				}
				kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

				auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
				uiParentEventID = gDLL->GameplayUnitCombat(pDllCombatInfo.get());

				// Set the combat units so that other missions do not continue until combat is over.
				kAttacker.setCombatUnit(pDefender.pointer(), true);
				pDefender->setCombatUnit(&kAttacker, false);

				eResult = ATTACK_QUEUED;
			}
			else
				eResult = ATTACK_COMPLETED;

			// Resolve combat here.
			ResolveCombat(kCombatInfo, uiParentEventID);

		}
	}

	return eResult;
}

//	---------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::AttackRanged(CvUnit& kAttacker, int iX, int iY, CvUnitCombat::ATTACK_OPTION /* eOption */)
{
	//VALIDATE_OBJECT
	CvPlot* pPlot = GC.getMap().plot(iX, iY);
	ATTACK_RESULT eResult = ATTACK_ABORTED;

	CvAssertMsg(kAttacker.getDomainType() != DOMAIN_AIR, "Air units should not use AttackRanged, they should just MoveTo the target");

	if(NULL == pPlot)
	{
		return eResult;
	}

	if (!kAttacker.isRangedSupportFire())
	{
		if(!kAttacker.canRangeStrikeAt(iX, iY))
		{
			return eResult;
		}

		if(GC.getRANGED_ATTACKS_USE_MOVES() == 0)
		{
			kAttacker.setMadeAttack(true);
		}
		kAttacker.changeMoves(-GC.getMOVE_DENOMINATOR());
	}

	// Unit that attacks loses his Fort bonus
	kAttacker.setFortifyTurns(0);

	// New test feature - attacking/range striking uses up all moves for most Units
#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
	if(!kAttacker.canMoveAfterAttacking() && !kAttacker.isRangedSupportFire() && kAttacker.isOutOfAttacks())
#else
	if(!kAttacker.canMoveAfterAttacking() && !kAttacker.isRangedSupportFire())
#endif
	{
		kAttacker.finishMoves();
		GC.GetEngineUserInterface()->changeCycleSelectionCounter(1);
	}

	kAttacker.SetAutomateType(NO_AUTOMATE);

	bool bDoImmediate = CvPreGame::quickCombat();
	// Range-striking a Unit
	if(!pPlot->isCity())
	{
		CvUnit* pDefender = kAttacker.airStrikeTarget(*pPlot, true);
		CvAssert(pDefender != NULL);
		if(!pDefender) return ATTACK_ABORTED;

		pDefender->SetAutomateType(NO_AUTOMATE);

		CvCombatInfo kCombatInfo;
		CvUnitCombat::GenerateRangedCombatInfo(kAttacker, pDefender, *pPlot, &kCombatInfo);
		CvAssertMsg(!kAttacker.isDelayedDeath() && !pDefender->isDelayedDeath(), "Trying to battle and one of the units is already dead!");

		uint uiParentEventID = 0;
		if(!bDoImmediate)
		{
			// Center camera here!
			bool isTargetVisibleToActivePlayer = pPlot->isActiveVisible(false);
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllPlot = GC.WrapPlotPointer(pPlot);
				GC.GetEngineUserInterface()->lookAt(pDllPlot.get(), CAMERALOOKAT_NORMAL);
			}
			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayUnitCombat(pDllCombatInfo.get());

			// Set the combat units so that other missions do not continue until combat is over.
			kAttacker.setCombatUnit(pDefender, true);
			pDefender->setCombatUnit(&kAttacker, false);
			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}
	// Range-striking a City
	else
	{
		if (kAttacker.isRangedSupportFire())
		{
			return eResult;
		}

		CvCombatInfo kCombatInfo;
		GenerateRangedCombatInfo(kAttacker, NULL, *pPlot, &kCombatInfo);
		CvAssertMsg(!kAttacker.isDelayedDeath(), "Trying to battle and the attacker is already dead!");

		uint uiParentEventID = 0;
		if(!bDoImmediate)
		{
			// Center camera here!
			bool isTargetVisibleToActivePlayer = pPlot->isActiveVisible(false);
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllPlot = GC.WrapPlotPointer(pPlot);
				GC.GetEngineUserInterface()->lookAt(pDllPlot.get(), CAMERALOOKAT_NORMAL);
			}

			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayCityCombat(pDllCombatInfo.get());

			CvCity* pkDefender = pPlot->getPlotCity();
			kAttacker.setCombatCity(pkDefender);
			if(pkDefender)
				pkDefender->setCombatUnit(&kAttacker);
			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}

	return eResult;
}

//	----------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::AttackAir(CvUnit& kAttacker, CvPlot& targetPlot, ATTACK_OPTION /* eOption */)
{
	//VALIDATE_OBJECT
	CvAssert(kAttacker.getCombatTimer() == 0);

	CvUnitCombat::ATTACK_RESULT eResult = CvUnitCombat::ATTACK_ABORTED;

	// Can we actually hit the target?
	if(!kAttacker.canRangeStrikeAt(targetPlot.getX(), targetPlot.getY()))
	{
		return eResult;
	}

	bool bDoImmediate = CvPreGame::quickCombat();
	kAttacker.SetAutomateType(NO_AUTOMATE);
	kAttacker.setMadeAttack(true);

	// Bombing a Unit
	if(!targetPlot.isCity())
	{
		CvUnit* pDefender = kAttacker.airStrikeTarget(targetPlot, true);
		CvAssert(pDefender != NULL);
		if(!pDefender) return CvUnitCombat::ATTACK_ABORTED;

		pDefender->SetAutomateType(NO_AUTOMATE);

		CvCombatInfo kCombatInfo;
		CvUnitCombat::GenerateAirCombatInfo(kAttacker, pDefender, targetPlot, &kCombatInfo);
		CvAssertMsg(!kAttacker.isDelayedDeath() && !pDefender->isDelayedDeath(), "Trying to battle and one of the units is already dead!");

		uint uiParentEventID = 0;
		if(!bDoImmediate)
		{
			// Center camera here!
			bool isTargetVisibleToActivePlayer = targetPlot.isActiveVisible(false);
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllTargetPlot = GC.WrapPlotPointer(&targetPlot);
				GC.GetEngineUserInterface()->lookAt(pDllTargetPlot.get(), CAMERALOOKAT_NORMAL);
			}
			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayUnitCombat(pDllCombatInfo.get());

			// Set the combat units so that other missions do not continue until combat is over.
			kAttacker.setCombatUnit(pDefender, true);
			pDefender->setCombatUnit(&kAttacker, false);
			CvUnit* pDefenderSupport = kCombatInfo.getUnit(BATTLE_UNIT_INTERCEPTOR);
			if(pDefenderSupport)
				pDefenderSupport->setCombatUnit(&kAttacker, false);

			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}
	// Bombing a City
	else
	{
		CvCombatInfo kCombatInfo;
		GenerateAirCombatInfo(kAttacker, NULL, targetPlot, &kCombatInfo);
		CvAssertMsg(!kAttacker.isDelayedDeath(), "Trying to battle and the attacker is already dead!");

		uint uiParentEventID = 0;
		if(!bDoImmediate)
		{
			// Center camera here!
			bool isTargetVisibleToActivePlayer = targetPlot.isActiveVisible(false);
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllTargetPlot = GC.WrapPlotPointer(&targetPlot);
				GC.GetEngineUserInterface()->lookAt(pDllTargetPlot.get(), CAMERALOOKAT_NORMAL);
			}

			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);
			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayCityCombat(pDllCombatInfo.get());

			CvCity* pkDefender = targetPlot.getPlotCity();
			kAttacker.setCombatCity(pkDefender);
			if(pkDefender)
				pkDefender->setCombatUnit(&kAttacker);
			CvUnit* pDefenderSupport = kCombatInfo.getUnit(BATTLE_UNIT_INTERCEPTOR);
			if(pDefenderSupport)
				pDefenderSupport->setCombatUnit(&kAttacker, false);
			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}

	return eResult;
}

//	----------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::AttackAirSweep(CvUnit& kAttacker, CvPlot& targetPlot, ATTACK_OPTION /* eOption */)
{
	//VALIDATE_OBJECT
	CvAssert(kAttacker.getCombatTimer() == 0);

	CvUnitCombat::ATTACK_RESULT eResult = CvUnitCombat::ATTACK_ABORTED;

	// Can we actually hit the target?
	if(!kAttacker.canAirSweepAt(targetPlot.getX(), targetPlot.getY()))
	{
		return eResult;
	}

	CvUnit* pInterceptor = kAttacker.GetBestInterceptor(targetPlot);
	kAttacker.SetAutomateType(NO_AUTOMATE);

	// Any interceptor to sweep for?
	if(pInterceptor != NULL)
	{
		kAttacker.setMadeAttack(true);
		CvCombatInfo kCombatInfo;
		CvUnitCombat::GenerateAirSweepCombatInfo(kAttacker, pInterceptor, targetPlot, &kCombatInfo);
		CvUnit* pkDefender = kCombatInfo.getUnit(BATTLE_UNIT_DEFENDER);
		pkDefender->SetAutomateType(NO_AUTOMATE);
		CvAssertMsg(!kAttacker.isDelayedDeath() && !pkDefender->isDelayedDeath(), "Trying to battle and one of the units is already dead!");

		uint uiParentEventID = 0;
		bool bDoImmediate = CvPreGame::quickCombat();
		if(!bDoImmediate)
		{
			// Center camera here!
			bool isTargetVisibleToActivePlayer = targetPlot.isActiveVisible(false);
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllTargetPlot = GC.WrapPlotPointer(&targetPlot);
				GC.GetEngineUserInterface()->lookAt(pDllTargetPlot.get(), CAMERALOOKAT_NORMAL);
			}
			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayUnitCombat(pDllCombatInfo.get());

			// Set the combat units so that other missions do not continue until combat is over.
			kAttacker.setCombatUnit(pInterceptor, true);
			pInterceptor->setCombatUnit(&kAttacker, false);
			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}
	else
	{
		// attempted to do a sweep in a plot that had no interceptors
		// consume the movement and finish its moves
		if(kAttacker.getOwner() == GC.getGame().getActivePlayer())
		{
			Localization::String localizedText = Localization::Lookup("TXT_KEY_AIR_PATROL_FOUND_NOTHING");
			localizedText << kAttacker.getUnitInfo().GetTextKey();
			GC.messagePlayer(0, kAttacker.getOwner(), false, GC.getEVENT_MESSAGE_TIME(), localizedText.toUTF8());
		}

		// Spend a move for this attack
		kAttacker.changeMoves(-GC.getMOVE_DENOMINATOR());

		// Can't move or attack again
#ifdef NQ_UNIT_TURN_ENDS_ON_FINAL_ATTACK
		if(!kAttacker.canMoveAfterAttacking() && kAttacker.isOutOfAttacks())
#else
		if(!kAttacker.canMoveAfterAttacking())
#endif
		{
			kAttacker.finishMoves();
		}
	}

	return eResult;
}

//	---------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::AttackCity(CvUnit& kAttacker, CvPlot& plot, CvUnitCombat::ATTACK_OPTION eOption)
{
	//VALIDATE_OBJECT

	ATTACK_RESULT eResult = ATTACK_ABORTED;
	CvCity* pCity = plot.getPlotCity();
	CvAssertMsg(pCity != NULL, "If this unit is attacking a NULL city then something funky is goin' down");
	if(!pCity) return eResult;

	kAttacker.SetAutomateType(NO_AUTOMATE);

	if(eOption != ATTACK_OPTION_NO_DEFENSIVE_SUPPORT)
	{
		// See if the city has some supporting fire to fend off the attacker
		CvUnit* pFireSupportUnit = GetFireSupportUnit(pCity->getOwner(), pCity->getX(), pCity->getY(), kAttacker.getX(), kAttacker.getY());

		ATTACK_RESULT eSupportResult = ATTACK_ABORTED;
		if(pFireSupportUnit)
		{
			eSupportResult = AttackRanged(*pFireSupportUnit, kAttacker.getX(), kAttacker.getY(), CvUnitCombat::ATTACK_OPTION_NO_DEFENSIVE_SUPPORT);
			// Turn off Fortify Turns, as this is the trigger for whether or not a ranged Unit can provide support fire (in addition to hasMadeAttack)
			pFireSupportUnit->setFortifyTurns(0);
		}

		if(eSupportResult == ATTACK_QUEUED)
		{
			// The supporting unit has queued their attack (against the attacker), we must have the attacker queue its attack.
			// Also, flag the current mission that the next time through, the defender doesn't get any defensive support.
			const_cast<MissionData*>(kAttacker.GetHeadMissionData())->iFlags |= MISSION_MODIFIER_NO_DEFENSIVE_SUPPORT;
			CvUnitMission::WaitFor(&kAttacker, pFireSupportUnit);
			eResult = ATTACK_QUEUED;
		}
	}

	if(eResult != ATTACK_QUEUED)
	{
		kAttacker.setMadeAttack(true);

		// We are doing a non-ranged attack on a city
		CvCombatInfo kCombatInfo;
		GenerateMeleeCombatInfo(kAttacker, NULL, plot, &kCombatInfo);
		CvAssertMsg(!kAttacker.isDelayedDeath(), "Trying to battle and the attacker is already dead!");

		// Send the combat message if the target plot is visible.
		bool isTargetVisibleToActivePlayer = plot.isActiveVisible(false);

		uint uiParentEventID = 0;
		bool bDoImmediate = CvPreGame::quickCombat();
		if(!bDoImmediate)
		{
			// Center camera here!
			if(isTargetVisibleToActivePlayer)
			{
				auto_ptr<ICvPlot1> pDllPlot = GC.WrapPlotPointer(&plot);
				GC.GetEngineUserInterface()->lookAt(pDllPlot.get(), CAMERALOOKAT_NORMAL);
			}
			kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

			auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
			uiParentEventID = gDLL->GameplayCityCombat(pDllCombatInfo.get());

			CvCity* pkDefender = plot.getPlotCity();
			kAttacker.setCombatCity(pkDefender);
			if(pkDefender)
				pkDefender->setCombatUnit(&kAttacker);
			eResult = ATTACK_QUEUED;
		}
		else
			eResult = ATTACK_COMPLETED;

		ResolveCombat(kCombatInfo, uiParentEventID);
	}
	return eResult;
}

//	-------------------------------------------------------------------------------------------
//	Return a ranged unit that will defend the supplied location against the attacker at the specified location.
CvUnit* CvUnitCombat::GetFireSupportUnit(PlayerTypes eDefender, int iDefendX, int iDefendY, int iAttackX, int iAttackY)
{
	VALIDATE_OBJECT

	if(GC.getFIRE_SUPPORT_DISABLED() == 1)
		return NULL;

	CvPlot* pAdjacentPlot = NULL;
	CvPlot* pPlot = GC.getMap().plot(iDefendX, iDefendY);

	for(int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pAdjacentPlot = plotDirection(pPlot->getX(), pPlot->getY(), ((DirectionTypes)iI));

		if(pAdjacentPlot != NULL)
		{
#ifdef AUI_WARNING_FIXES
			for (uint iUnitLoop = 0; iUnitLoop < pAdjacentPlot->getNumUnits(); iUnitLoop++)
#else
			for(int iUnitLoop = 0; iUnitLoop < pAdjacentPlot->getNumUnits(); iUnitLoop++)
#endif
			{
				CvUnit* pLoopUnit = pAdjacentPlot->getUnitByIndex(iUnitLoop);

				// Unit owned by same player?
				if(pLoopUnit->getOwner() == eDefender)
				{
					// Can this unit perform a ranged strike on the attacker's plot?
					if(pLoopUnit->canRangeStrikeAt(iAttackX, iAttackY))
					{
						// Range strike would be calculated here, so get the estimated damage
						return pLoopUnit;
					}
				}
			}
		}
	}

	return NULL;
}

//	----------------------------------------------------------------------------
CvUnitCombat::ATTACK_RESULT CvUnitCombat::AttackNuclear(CvUnit& kAttacker, int iX, int iY, ATTACK_OPTION /* eOption */)
{
	ATTACK_RESULT eResult = ATTACK_ABORTED;

	CvPlot* pPlot = GC.getMap().plot(iX, iY);
	if(NULL == pPlot)
		return eResult;

	bool bDoImmediate = CvPreGame::quickCombat();
	CvCombatInfo kCombatInfo;
	CvUnitCombat::GenerateNuclearCombatInfo(kAttacker, *pPlot, &kCombatInfo);
	CvAssertMsg(!kAttacker.isDelayedDeath(), "Trying to battle and the attacker is already dead!");
	kAttacker.setMadeAttack(true);
	uint uiParentEventID = 0;
	if(!bDoImmediate)
	{
		// Nuclear attacks are different in that you can target a plot you can't see, so check to see if the active player
		// is involved in the combat
		TeamTypes eActiveTeam = GC.getGame().getActiveTeam();

		bool isTargetVisibleToActivePlayer = pPlot->isActiveVisible(false);
		if(!isTargetVisibleToActivePlayer)
		{
			// Is the attacker part of the local team?
			isTargetVisibleToActivePlayer = (kAttacker.getTeam() != NO_TEAM && eActiveTeam == kAttacker.getTeam());

			if(!isTargetVisibleToActivePlayer)
			{
				// Are any of the teams effected by the blast in the local team?
				for(int i = 0; i < MAX_TEAMS && !isTargetVisibleToActivePlayer; ++i)
				{
					if(kAttacker.isNukeVictim(pPlot, ((TeamTypes)i)))
					{
						isTargetVisibleToActivePlayer = eActiveTeam == ((TeamTypes)i);
					}
				}
			}
		}

		if(isTargetVisibleToActivePlayer)
		{
			auto_ptr<ICvPlot1> pDllPlot = GC.WrapPlotPointer(pPlot);
			GC.GetEngineUserInterface()->lookAt(pDllPlot.get(), CAMERALOOKAT_NORMAL);
		}
		kCombatInfo.setVisualizeCombat(isTargetVisibleToActivePlayer);

		// Set a combat unit/city.  Not really needed for the combat since we are killing everyone, but it is currently the only way a unit is marked that it is 'in-combat'
		if(pPlot->getPlotCity())
			kAttacker.setCombatCity(pPlot->getPlotCity());
		else
		{
			if(pPlot->getNumUnits())
				kAttacker.setCombatUnit(pPlot->getUnitByIndex(0), true);
			else
				kAttacker.setAttackPlot(pPlot, false);
		}

		auto_ptr<ICvCombatInfo1> pDllCombatInfo(new CvDllCombatInfo(&kCombatInfo));
		uiParentEventID = gDLL->GameplayUnitCombat(pDllCombatInfo.get());

		eResult = ATTACK_QUEUED;
	}
	else
	{
		eResult = ATTACK_COMPLETED;
		// Set the plot, just so the unit is marked as 'in-combat'
		kAttacker.setAttackPlot(pPlot, false);
	}

	ResolveCombat(kCombatInfo,  uiParentEventID);

	return eResult;
}

//	---------------------------------------------------------------------------
void CvUnitCombat::ApplyPostCombatTraitEffects(CvUnit* pkWinner, CvUnit* pkLoser)
{
	int iExistingDelay = 0;

	// "Heal if defeat enemy" promotion; doesn't apply if defeat a barbarian
	if(pkWinner->getHPHealedIfDefeatEnemy() > 0 && (pkLoser->getOwner() != BARBARIAN_PLAYER || !(pkWinner->IsHealIfDefeatExcludeBarbarians())))
	{
		if(pkWinner->getHPHealedIfDefeatEnemy() > pkWinner->getDamage())
		{
			pkWinner->changeDamage(-pkWinner->getDamage());
		}
		else
		{
			pkWinner->changeDamage(-pkWinner->getHPHealedIfDefeatEnemy());
		}
	}

	CvPlayer& kPlayer = GET_PLAYER(pkWinner->getOwner());
	if (pkWinner->GetGoldenAgeValueFromKills() > 0)
	{
		int iCombatStrength = max(pkLoser->getUnitInfo().GetCombat(), pkLoser->getUnitInfo().GetRangedCombat());
		if(iCombatStrength > 0)
		{
			int iValue = iCombatStrength * pkWinner->GetGoldenAgeValueFromKills() / 100;
			kPlayer.ChangeGoldenAgeProgressMeter(iValue);

			CvString yieldString = "[COLOR_WHITE]+%d[ENDCOLOR][ICON_GOLDEN_AGE]";

			if(pkWinner->getOwner() == GC.getGame().getActivePlayer())
			{
				char text[256] = {0};
				float fDelay = GC.getPOST_COMBAT_TEXT_DELAY() * 1.5f;
				sprintf_s(text, yieldString, iValue);
				GC.GetEngineUserInterface()->AddPopupText(pkLoser->getX(), pkLoser->getY(), text, fDelay);

				iExistingDelay++;
			}
		}
	}

	// Earn bonuses for kills?
	kPlayer.DoYieldsFromKill(pkWinner->getUnitType(), pkLoser->getUnitType(), pkLoser->getX(), pkLoser->getY(), pkLoser->isBarbarian(), iExistingDelay);

	//Achievements and Stats
	if(pkWinner->isHuman() && !GC.getGame().isGameMultiPlayer())
	{
		CvString szUnitType;
		CvUnitEntry* pkUnitInfo = GC.getUnitInfo(pkWinner->getUnitType());
		if(pkUnitInfo)
			szUnitType = pkUnitInfo->GetType();

		//Elizabeth Special Achievement
		if((CvString)kPlayer.getLeaderTypeKey() == "LEADER_ELIZABETH" && pkLoser->getDomainType() == DOMAIN_SEA)
		{
			gDLL->IncrementSteamStatAndUnlock(ESTEAMSTAT_BRITISHNAVY, 357, ACHIEVEMENT_SPECIAL_ARMADA);
		}
		//Ramkang's Special Achievement
		if(szUnitType == "UNIT_SIAMESE_WARELEPHANT")
		{
			//CvString szUnitTypeLoser = (CvString) GC.getUnitInfo(pkLoser->getUnitType())->GetType();
		}

		//Oda's Special Achievement
		if((CvString)kPlayer.getLeaderTypeKey() == "LEADER_ODA_NOBUNAGA" && (pkWinner->GetMaxHitPoints() - pkWinner->getDamage() == 1))
		{
			gDLL->UnlockAchievement(ACHIEVEMENT_SPECIAL_KAMIKAZE);
		}
		//Napoleon's Special Achievement
		if(szUnitType == "UNIT_FRENCH_MUSKETEER")
		{
			if(pkLoser->GetNumSpecificEnemyUnitsAdjacent(pkLoser, pkWinner) >=3)
			{
				gDLL->UnlockAchievement(ACHIEVEMENT_SPECIAL_MUSKETEERS);
			}
		}

		//DLC_05 Sejong's Turtle Boat Achievement
		if(szUnitType == "UNIT_KOREAN_TURTLE_SHIP")
		{
			CvString szLoserUnitType;
			CvUnitEntry* pkLoserUnitInfo = GC.getUnitInfo(pkLoser->getUnitType());
			if(pkLoserUnitInfo)
			{
				szLoserUnitType = pkLoserUnitInfo->GetType();
			}
			if(szLoserUnitType == "UNIT_IRONCLAD")
			{
				gDLL->UnlockAchievement(ACHIEVEMENT_SPECIAL_IRONCLAD_TURTLE);
			}
		}

		//DLC_05 Sejong's Hwacha Achievement
		if(szUnitType == "UNIT_KOREAN_HWACHA")
		{
			gDLL->IncrementSteamStatAndUnlock(ESTEAMSTAT_HWACHAKILLS, 99, ACHIEVEMENT_SPECIAL_HWATCH_OUT);
		}

	}
}

void CvUnitCombat::ApplyPostCityCombatEffects(CvUnit* pkAttacker, CvCity* pkDefender, int iAttackerDamageInflicted)
{
	CvString colorString;
	int iPlunderModifier;
	float fDelay = GC.getPOST_COMBAT_TEXT_DELAY() * 3;
	iPlunderModifier = pkAttacker->GetCityAttackPlunderModifier();
	if(iPlunderModifier > 0)
	{
		int iGoldPlundered = iAttackerDamageInflicted * iPlunderModifier;
		iGoldPlundered /= 100;

		if(iGoldPlundered > 0)
		{
			GET_PLAYER(pkAttacker->getOwner()).GetTreasury()->ChangeGold(iGoldPlundered);

			CvPlayer& kCityPlayer = GET_PLAYER(pkDefender->getOwner());
			int iDeduction = min(iGoldPlundered, kCityPlayer.GetTreasury()->GetGold());
			kCityPlayer.GetTreasury()->ChangeGold(-iDeduction);

			if(pkAttacker->getOwner() == GC.getGame().getActivePlayer())
			{
				char text[256] = {0};
				colorString = "[COLOR_YELLOW]+%d[ENDCOLOR][ICON_GOLD]";
				sprintf_s(text, colorString, iGoldPlundered);
				GC.GetEngineUserInterface()->AddPopupText(pkAttacker->getX(), pkAttacker->getY(), text, fDelay);
			}
		}
	}
}