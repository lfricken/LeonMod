/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.
	All other marks and trademarks are the property of their respective owners.
	All rights reserved.
	------------------------------------------------------------------------------------------------------- */

#include "CvGameCoreDLLPCH.h"
#include "CvBarbarians.h"
#include "CvGameCoreUtils.h"
#include "CvTypes.h"
#include "CvUnitMission.h"

	//static 
#ifdef AUI_WARNING_FIXES
int* CvBarbarians::m_aiPlotBarbCampSpawnCounter = NULL;
int* CvBarbarians::m_aiPlotBarbCampNumUnitsSpawned = NULL;
#else
short* CvBarbarians::m_aiPlotBarbCampSpawnCounter = NULL;
short* CvBarbarians::m_aiPlotBarbCampNumUnitsSpawned = NULL;
#endif
FStaticVector<DirectionTypes, 6, true, c_eCiv5GameplayDLL, 0> CvBarbarians::m_aeValidBarbSpawnDirections;

//	---------------------------------------------------------------------------
bool CvBarbarians::IsPlotValidForBarbCamp(CvPlot* pPlot)
{
	int iRange = 4;
	int iDY;

#ifdef AUI_HEXSPACE_DX_LOOPS
	int iMaxDX, iDX;
	CvPlot* pLoopPlot;
	for (iDY = -iRange; iDY <= iRange; iDY++)
	{
		iMaxDX = iRange - MAX(0, iDY);
		for (iDX = -iRange - MIN(0, iDY); iDX <= iMaxDX; iDX++) // MIN() and MAX() stuff is to reduce loops (hexspace!)
		{
			// No need for range check because loops are set up properly
			pLoopPlot = plotXY(pPlot->getX(), pPlot->getY(), iDX, iDY);
#else
	int iPlotX = pPlot->getX();
	int iPlotY = pPlot->getY();

	CvMap& kMap = GC.getMap();
	for (int iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			int iLoopPlotX = iPlotX + iDX;
			int iLoopPlotY = iPlotY + iDY;

			// Cut off corners
			if (plotDistance(iPlotX, iPlotY, iLoopPlotX, iLoopPlotY) > iRange)
				continue;

			// If the counter is below -1 that means a camp was cleared recently
			CvPlot* pLoopPlot = kMap.plot(iLoopPlotX, iLoopPlotY);
#endif
			if (pLoopPlot)
			{
				if (m_aiPlotBarbCampSpawnCounter[pLoopPlot->GetPlotIndex()] < -1)
					return false;
			}
		}
	}

	return true;
}

//	--------------------------------------------------------------------------------
/// Camp cleared, so reset counter
void CvBarbarians::DoBarbCampCleared(CvPlot * pPlot, PlayerTypes ePlayer)
{
	m_aiPlotBarbCampSpawnCounter[pPlot->GetPlotIndex()] = -16;

	pPlot->AddArchaeologicalRecord(CvTypes::getARTIFACT_BARBARIAN_CAMP(), ePlayer, NO_PLAYER);
}

bool CvBarbarians::CanBarbariansSpawn()
{
	CvGame& kGame = GC.getGame();
	if (kGame.isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return false;
	}

	const T100 doneNess = GC.getPercentTurnsDoneT10000();
	const T100 requiredDoneness = GC.getFIRST_BARB_SPAWNT10000();
	if (doneNess < requiredDoneness)
	{
		return false;
	}

	return true;
}

//	--------------------------------------------------------------------------------
/// Determines when to Spawn a new Barb Unit from a Camp
bool CvBarbarians::ShouldSpawnBarbFromCamp(CvPlot * pPlot)
{
	if (m_aiPlotBarbCampSpawnCounter[pPlot->GetPlotIndex()] == 0)
	{
		return true;
	}

	return false;
}

//	--------------------------------------------------------------------------------
/// Gameplay informing us when a Camp has either been created or spawned a Unit so we can reseed the spawn counter
void CvBarbarians::DoCampActivationNotice(CvPlot * pPlot)
{
	CvGame& kGame = GC.getGame();
	// Default to between 8 and 12 turns per spawn
	int iNumTurnsToSpawn = 8 + kGame.getJonRandNum(5, "Barb Spawn Rand call", pPlot, 487);

	// Raging
	if (kGame.isOption(GAMEOPTION_RAGING_BARBARIANS))
		iNumTurnsToSpawn /= 2;

	// Num Units Spawned
	int iNumUnitsSpawned = m_aiPlotBarbCampNumUnitsSpawned[pPlot->GetPlotIndex()];

	// Reduce turns between spawn if we've pumped out more guys (meaning we're further into the game)
	iNumTurnsToSpawn -= min(3, iNumUnitsSpawned);	// -1 turns if we've spawned one Unit, -3 turns if we've spawned three

	// Increment # of barbs spawned from this camp
	m_aiPlotBarbCampNumUnitsSpawned[pPlot->GetPlotIndex()]++;	// This starts at -1 so when a camp is first created it will bump up to 0, which is correct

	//// If it's too early to spawn then add in a small amount to delay things a bit - between 3 and 6 extra turns
	//if (CanBarbariansSpawn())
	//{
	//	iNumTurnsToSpawn += 3;
	//	iNumTurnsToSpawn += auto_ptr<ICvGame1> pGame = GameCore::GetGame();\n.getJonRandNum(4, "Early game Barb Spawn Rand call");
	//}

	// Difficulty level can add time between spawns (e.g. Settler is +8 turns)
	CvHandicapInfo* pHandicapInfo = GC.getHandicapInfo(kGame.getHandicapType());
	if (pHandicapInfo)
		iNumTurnsToSpawn += pHandicapInfo->getBarbSpawnMod();

	// Game Speed can increase or decrease amount of time between spawns (ranges from 67 on Quick to 400 on Marathon)
	CvGameSpeedInfo* pGameSpeedInfo = GC.getGameSpeedInfo(kGame.getGameSpeedType());
	if (pGameSpeedInfo)
	{
		iNumTurnsToSpawn *= pGameSpeedInfo->getBarbPercent();
		iNumTurnsToSpawn /= 100;
	}

	m_aiPlotBarbCampSpawnCounter[pPlot->GetPlotIndex()] = iNumTurnsToSpawn;
}

//	--------------------------------------------------------------------------------
/// Gameplay informing a camp has been attacked - make it more likely to spawn
void CvBarbarians::DoCampAttacked(CvPlot * pPlot)
{
	int iCounter = m_aiPlotBarbCampSpawnCounter[pPlot->GetPlotIndex()];

	// Halve the amount of time to spawn
	int iNewValue = iCounter / 2;

	m_aiPlotBarbCampSpawnCounter[pPlot->GetPlotIndex()] = iNewValue;
}

//	---------------------------------------------------------------------------
/// Called every turn
void CvBarbarians::BeginTurn()
{
	CvGame& kGame = GC.getGame();
	const ImprovementTypes eCamp = kGame.GetBarbarianCampImprovementType();

	CvMap& kMap = GC.getMap();
	int iWorldNumPlots = kMap.numPlots();
	for (int iPlotLoop = 0; iPlotLoop < iWorldNumPlots; iPlotLoop++)
	{
		if (m_aiPlotBarbCampSpawnCounter[iPlotLoop] > 0)
		{
			// No Camp here any more
			CvPlot* pPlot = kMap.plotByIndex(iPlotLoop);

			if (pPlot->getImprovementType() != eCamp)
			{
				m_aiPlotBarbCampSpawnCounter[iPlotLoop] = -1;
				m_aiPlotBarbCampNumUnitsSpawned[iPlotLoop] = -1;
			}
			else
			{
				m_aiPlotBarbCampSpawnCounter[iPlotLoop]--;
			}
		}

		// Counter is negative, meaning a camp was cleared here recently and isn't allowed to respawn in the area for a while
		else if (m_aiPlotBarbCampSpawnCounter[iPlotLoop] < -1)
		{
			m_aiPlotBarbCampSpawnCounter[iPlotLoop]++;
		}
	}
}

//	--------------------------------------------------------------------------------
void CvBarbarians::MapInit(int iWorldNumPlots)
{
	if (m_aiPlotBarbCampSpawnCounter != NULL)
	{
		SAFE_DELETE_ARRAY(m_aiPlotBarbCampSpawnCounter);
	}
	if (m_aiPlotBarbCampNumUnitsSpawned != NULL)
	{
		SAFE_DELETE_ARRAY(m_aiPlotBarbCampNumUnitsSpawned);
	}

	int iI;

	if (iWorldNumPlots > 0)
	{
		if (m_aiPlotBarbCampSpawnCounter == NULL)
		{
#ifdef AUI_WARNING_FIXES
			m_aiPlotBarbCampSpawnCounter = FNEW(int[iWorldNumPlots], c_eCiv5GameplayDLL, 0);
#else
			m_aiPlotBarbCampSpawnCounter = FNEW(short[iWorldNumPlots], c_eCiv5GameplayDLL, 0);
#endif
		}
		if (m_aiPlotBarbCampNumUnitsSpawned == NULL)
		{
#ifdef AUI_WARNING_FIXES
			m_aiPlotBarbCampNumUnitsSpawned = FNEW(int[iWorldNumPlots], c_eCiv5GameplayDLL, 0);
#else
			m_aiPlotBarbCampNumUnitsSpawned = FNEW(short[iWorldNumPlots], c_eCiv5GameplayDLL, 0);
#endif
		}

		// Default values
		for (iI = 0; iI < iWorldNumPlots; ++iI)
		{
			m_aiPlotBarbCampSpawnCounter[iI] = -1;
			m_aiPlotBarbCampNumUnitsSpawned[iI] = -1;
		}
	}
}

//	---------------------------------------------------------------------------
/// Uninit
void CvBarbarians::Uninit()
{
	if (m_aiPlotBarbCampSpawnCounter != NULL)
	{
		SAFE_DELETE_ARRAY(m_aiPlotBarbCampSpawnCounter);
	}

	if (m_aiPlotBarbCampNumUnitsSpawned != NULL)
	{
		SAFE_DELETE_ARRAY(m_aiPlotBarbCampNumUnitsSpawned);
	}
}

//	---------------------------------------------------------------------------
/// Serialization Read
#ifdef AUI_WARNING_FIXES
void CvBarbarians::Read(FDataStream & kStream, uint /*uiParentVersion*/)
#else
void CvBarbarians::Read(FDataStream & kStream, uint uiParentVersion)
#endif
{
	// Version number to maintain backwards compatibility
	uint uiVersion = 0;

	kStream >> uiVersion;

	int iWorldNumPlots = GC.getMap().numPlots();
	MapInit(iWorldNumPlots);	// Map will have been initialized/unserialized by now so this is ok.

#ifdef AUI_WARNING_FIXES
	ArrayWrapper<int> kWrapper1(iWorldNumPlots, m_aiPlotBarbCampSpawnCounter);
	kStream >> kWrapper1;
	ArrayWrapper<int> kWrapper2(iWorldNumPlots, m_aiPlotBarbCampNumUnitsSpawned);
	kStream >> kWrapper2;
#else
	kStream >> ArrayWrapper<short>(iWorldNumPlots, m_aiPlotBarbCampSpawnCounter);
	kStream >> ArrayWrapper<short>(iWorldNumPlots, m_aiPlotBarbCampNumUnitsSpawned);
#endif
}

//	---------------------------------------------------------------------------
/// Serialization Write
void CvBarbarians::Write(FDataStream & kStream)
{
	// Current version number
	uint uiVersion = 1;
	kStream << uiVersion;

	int iWorldNumPlots = GC.getMap().numPlots();
#ifdef AUI_WARNING_FIXES
	ArrayWrapper<int> kWrapper1(iWorldNumPlots, m_aiPlotBarbCampSpawnCounter);
	kStream << kWrapper1;
	ArrayWrapper<int> kWrapper2(iWorldNumPlots, m_aiPlotBarbCampNumUnitsSpawned);
	kStream << kWrapper2;
#else
	kStream << ArrayWrapper<short>(iWorldNumPlots, m_aiPlotBarbCampSpawnCounter);
	kStream << ArrayWrapper<short>(iWorldNumPlots, m_aiPlotBarbCampNumUnitsSpawned);
#endif
}

//	--------------------------------------------------------------------------------
void CvBarbarians::DoCamps()
{
	CvGame& kGame = GC.getGame();

	if (kGame.isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}

	int iNumNotVisiblePlots = 0;
	int iNumCampsInExistence = 0;
	CvPlot* pLoopPlot = NULL;

	ImprovementTypes eCamp = (ImprovementTypes)GC.getBARBARIAN_CAMP_IMPROVEMENT();

	bool bAlwaysRevealedBarbCamp = false;

	// Is there an appropriate Improvement to place as a Barb Camp?
	if (eCamp != NO_IMPROVEMENT)
	{
		CvMap& kMap = GC.getMap();
		// Figure out how many Nonvisible tiles we have to base # of camps to spawn on
#ifdef AUI_WARNING_FIXES
		for (uint iI = 0; iI < kMap.numPlots(); iI++)
#else
		for (int iI = 0; iI < kMap.numPlots(); iI++)
#endif
		{
			pLoopPlot = kMap.plotByIndexUnchecked(iI);

			// See how many camps we already have
			if (pLoopPlot->getImprovementType() == eCamp)
			{
				iNumCampsInExistence++;
			}

			if (!pLoopPlot->isWater())
			{
				if (!pLoopPlot->isVisibleToCivTeam())
				{
					iNumNotVisiblePlots++;
				}
			}
		}

		int iNumValidCampPlots;

		iNumValidCampPlots = iNumNotVisiblePlots;

		int iFogTilesPerBarbarianCamp = kMap.getWorldInfo().getFogTilesPerBarbarianCamp();
		int iCampTargetNum = (iFogTilesPerBarbarianCamp != 0) ? iNumValidCampPlots / iFogTilesPerBarbarianCamp : 0;//getHandicapInfo().getFogTilesPerBarbarianCamp();
		int iNumCampsToAdd = iCampTargetNum - iNumCampsInExistence;

		int iMaxCampsThisArea;

		if (iNumCampsToAdd > 0 && GC.getBARBARIAN_CAMP_ODDS_OF_NEW_CAMP_SPAWNING() > 0) // slewis - added the barbarian chance for the FoR scenario
		{
			// First turn of the game add 1/3 of the Target number of Camps
			if (kGame.getElapsedGameTurns() == 0)
			{
				iNumCampsToAdd *= /*33*/ GC.getBARBARIAN_CAMP_FIRST_TURN_PERCENT_OF_TARGET_TO_ADD();
				iNumCampsToAdd /= 100;
			}
			// Every other turn of the game there's a 1 in 2 chance of adding a new camp if we're still below the target
			else
			{
				if (kGame.getJonRandNum(/*2*/ GC.getBARBARIAN_CAMP_ODDS_OF_NEW_CAMP_SPAWNING(), "Random roll to see if Barb Camp spawns this turn", pLoopPlot, 6) > 0)
				{
					iNumCampsToAdd = 1;
				}
				else
				{
					iNumCampsToAdd = 0;
				}
			}

			// Don't want to get stuck in an infinite or almost so loop
			int iCount = 0;
			int iPlotIndex = -1;
			int iNumPlots = kMap.numPlots();

			UnitTypes eBestUnit;

			int iNumLandPlots = kMap.getLandPlots();

			// Do a random roll to bias in favor of Coastal land Tiles so that the Barbs will spawn Boats :) - required 1/6 of the time
			bool bWantsCoastal = kGame.getJonRandNum(/*6*/ GC.getBARBARIAN_CAMP_COASTAL_SPAWN_ROLL(), "Barb Camp Plot-Finding Roll - Coastal Bias 1", pLoopPlot, 3) == 0 ? true : false;

			int iPlayerCapitalMinDistance = /*4*/ GC.getBARBARIAN_CAMP_MINIMUM_DISTANCE_CAPITAL();
			int iBarbCampMinDistance = /*7*/ GC.getBARBARIAN_CAMP_MINIMUM_DISTANCE_ANOTHER_CAMP();
			int iMaxDistanceToLook = iPlayerCapitalMinDistance > iBarbCampMinDistance ? iPlayerCapitalMinDistance : iBarbCampMinDistance;
			int iPlotDistance;

			int iDX, iDY;
			CvPlot* pNearbyCampPlot;
			bool bSomethingTooClose;

			CvString strBuffer;

			int iPlayerLoop;

			// Find Plots to put the Camps
			do
			{
				iCount++;

				iPlotIndex = kGame.getJonRandNum(iNumPlots, "Barb Camp Plot-Finding Roll", pLoopPlot, iCount);

				pLoopPlot = kMap.plotByIndex(iPlotIndex);

				// Plot must be valid (not Water, nonvisible)
				if (!pLoopPlot->isWater())
				{
					if (!pLoopPlot->isImpassable() && !pLoopPlot->isMountain())
					{
						if (!pLoopPlot->isOwned() && !pLoopPlot->isVisibleToCivTeam())
						{
							// JON: NO RESOURCES FOR NOW, MAY REPLACE WITH SOMETHING COOLER
							if (pLoopPlot->getResourceType() == NO_RESOURCE)
							{
								// No camps on 1-tile islands
								if (kMap.getArea(pLoopPlot->getArea())->getNumTiles() > 1)
								{
									if (pLoopPlot->isCoastalLand() || !bWantsCoastal)
									{
										// Max Camps for this area
										iMaxCampsThisArea = iCampTargetNum * pLoopPlot->area()->getNumTiles() / iNumLandPlots;
										// Add 1 just in case the above algorithm rounded something off
										iMaxCampsThisArea++;

										// Already enough Camps in this Area?
										if (pLoopPlot->area()->getNumImprovements(eCamp) <= iMaxCampsThisArea)
										{
											// Don't look at Tiles that already have a Camp
											if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
											{
												// Don't look at Tiles that can't have an improvement
												if (pLoopPlot->getFeatureType() == NO_FEATURE || !GC.getFeatureInfo(pLoopPlot->getFeatureType())->isNoImprovement())
												{
													bSomethingTooClose = false;

#ifdef AUI_HEXSPACE_DX_LOOPS
													int iMaxDX;
													for (iDY = -(iMaxDistanceToLook); iDY <= iMaxDistanceToLook; iDY++)
													{
														iMaxDX = iMaxDistanceToLook - MAX(0, iDY);
														for (iDX = -(iMaxDistanceToLook)-MIN(0, iDY); iDX <= iMaxDX; iDX++) // MIN() and MAX() stuff is to reduce loops (hexspace!)
#else
													// Look at nearby Plots to make sure another camp isn't too close
													for (iDX = -(iMaxDistanceToLook); iDX <= iMaxDistanceToLook; iDX++)
													{
														for (iDY = -(iMaxDistanceToLook); iDY <= iMaxDistanceToLook; iDY++)
#endif
														{
															pNearbyCampPlot = plotXY(pLoopPlot->getX(), pLoopPlot->getY(), iDX, iDY);

															if (pNearbyCampPlot != NULL)
															{
#ifdef AUI_FIX_HEX_DISTANCE_INSTEAD_OF_PLOT_DISTANCE
																iPlotDistance = hexDistance(iDX, iDY);
#else
																iPlotDistance = plotDistance(pNearbyCampPlot->getX(), pNearbyCampPlot->getY(), pLoopPlot->getX(), pLoopPlot->getY());
#endif

																// Can't be too close to a player
																if (iPlotDistance <= iPlayerCapitalMinDistance)
																{
																	if (pNearbyCampPlot->isCity())
																	{
																		if (pNearbyCampPlot->getPlotCity()->isCapital())
																		{
																			// Only care about Majors' capitals
																			if (pNearbyCampPlot->getPlotCity()->getOwner() < MAX_MAJOR_CIVS)
																			{
																				bSomethingTooClose = true;
																				break;
																			}
																		}
																	}
																}

																// Can't be too close to another Camp
																if (iPlotDistance <= iBarbCampMinDistance)
																{
																	if (pNearbyCampPlot->getImprovementType() == eCamp)
																	{
																		bSomethingTooClose = true;
																		break;
																	}
																}
															}
														}
														if (bSomethingTooClose)
														{
															break;
														}
													}

													// Found a camp too close, check another Plot
													if (bSomethingTooClose)
														continue;

													// Last check
													if (!CvBarbarians::IsPlotValidForBarbCamp(pLoopPlot))
														continue;

													pLoopPlot->setImprovementType(eCamp);
													DoCampActivationNotice(pLoopPlot);

													eBestUnit = GetRandomBarbarianUnitType(kMap.getArea(pLoopPlot->getArea()), UNITAI_DEFENSE);

													if (eBestUnit != NO_UNIT)
													{
#ifdef AUI_WARNING_FIXES
														CvUnitEntry* pUnitInfo = GC.getUnitInfo(eBestUnit);
														if (pUnitInfo)
															GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pLoopPlot->getX(), pLoopPlot->getY(), (UnitAITypes)pUnitInfo->GetDefaultUnitAIType());
#else
														GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pLoopPlot->getX(), pLoopPlot->getY(), (UnitAITypes)GC.getUnitInfo(eBestUnit)->GetDefaultUnitAIType());
#endif
													}

													// If we should update Camp visibility (for Policy), do so
													PlayerTypes ePlayer;
													TeamTypes eTeam;
													for (iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
													{
														ePlayer = (PlayerTypes)iPlayerLoop;
														eTeam = GET_PLAYER(ePlayer).getTeam();

														if (GET_PLAYER(ePlayer).IsAlwaysSeeBarbCamps())
														{
															if (pLoopPlot->isRevealed(eTeam))
															{
																pLoopPlot->setRevealedImprovementType(eTeam, eCamp);
																if (GC.getGame().getActivePlayer() == ePlayer)
																	bAlwaysRevealedBarbCamp = true;
															}
														}
													}

													// Add another Unit adjacent to the Camp to stir up some trouble (JON: Disabled for now 09/12/09)
													//doSpawnBarbarianUnit(pLoopPlot);

													iNumCampsToAdd--;

													// Seed the next Camp for Coast or not
													bWantsCoastal = kGame.getJonRandNum(/*5*/ GC.getBARBARIAN_CAMP_COASTAL_SPAWN_ROLL(), "Barb Camp Plot-Finding Roll - Coastal Bias 2", pLoopPlot, iCount) == 0 ? true : false;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			} while (iNumCampsToAdd > 0 && iCount < iNumLandPlots);
		}
	}

	if (bAlwaysRevealedBarbCamp)
		GC.getMap().updateDeferredFog();
}

//	--------------------------------------------------------------------------------
UnitTypes CvBarbarians::GetRandomBarbarianUnitType(CvArea * pArea, UnitAITypes eUnitAI)
{
	UnitTypes eBestUnit = NO_UNIT;
	int iBestValue = 0;
	int iValue = 0;

	CvPlayerAI& kBarbarianPlayer = GET_PLAYER(BARBARIAN_PLAYER);

	CvGame& kGame = GC.getGame();

#ifdef AUI_WARNING_FIXES
	for (uint iUnitClassLoop = 0; iUnitClassLoop < GC.getNumUnitClassInfos(); iUnitClassLoop++)
#else
	for (int iUnitClassLoop = 0; iUnitClassLoop < GC.getNumUnitClassInfos(); iUnitClassLoop++)
#endif
	{
		bool bValid = false;
		CvUnitClassInfo* pkUnitClassInfo = GC.getUnitClassInfo((UnitClassTypes)iUnitClassLoop);
		if (pkUnitClassInfo == NULL)
			continue;

		const UnitTypes eLoopUnit = ((UnitTypes)(kBarbarianPlayer.getCivilizationInfo().getCivilizationUnits(iUnitClassLoop)));
		if (eLoopUnit != NO_UNIT)
		{
			CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eLoopUnit);
			if (pkUnitInfo == NULL)
			{
				continue;
			}

			CvUnitEntry& kUnit = *pkUnitInfo;

			bValid = (kUnit.GetCombat() > 0);
			if (bValid)
			{
				// Unit has combat strength, make sure it isn't only defensive (and with no ranged combat ability)
				if (kUnit.GetRange() == 0)
				{
#ifdef AUI_WARNING_FIXES
					for (uint iLoop = 0; iLoop < GC.getNumPromotionInfos(); iLoop++)
#else
					for (int iLoop = 0; iLoop < GC.getNumPromotionInfos(); iLoop++)
#endif
					{
						const PromotionTypes ePromotion = static_cast<PromotionTypes>(iLoop);
						CvPromotionEntry* pkPromotionInfo = GC.getPromotionInfo(ePromotion);
						if (pkPromotionInfo)
						{
							if (kUnit.GetFreePromotions(iLoop))
							{
								if (pkPromotionInfo->IsOnlyDefensive())
								{
									bValid = false;
									break;
								}
							}
						}
					}
				}
			}

			if (bValid)
			{
				if (pArea->isWater() && kUnit.GetDomainType() != DOMAIN_SEA)
				{
					bValid = false;
				}
				else if (!pArea->isWater() && kUnit.GetDomainType() != DOMAIN_LAND)
				{
					bValid = false;
				}
			}

			if (bValid)
			{
				if (!GET_PLAYER(BARBARIAN_PLAYER).canTrain(eLoopUnit))
				{
					bValid = false;
				}
			}

			if (bValid)
			{
				bool bFound = false;
				bool bRequires = false;
				for (int i = 0; i < GC.getNUM_UNIT_AND_TECH_PREREQS(); ++i)
				{
					TechTypes eTech = (TechTypes)kUnit.GetPrereqAndTechs(i);

					if (NO_TECH != eTech)
					{
						bRequires = true;

						if (GET_TEAM(BARBARIAN_TEAM).GetTeamTechs()->HasTech(eTech))
						{
							bFound = true;
							break;
						}
					}
				}

				if (bRequires && !bFound)
				{
					bValid = false;
				}
			}

			if (bValid)
			{
				iValue = (1 + kGame.getJonRandNum(1000, "Barb Unit Selection", NULL, pArea->GetID() * iUnitClassLoop));

				if (kUnit.GetUnitAIType(eUnitAI))
				{
					iValue += 200;
				}

				if (iValue > iBestValue)
				{
					eBestUnit = eLoopUnit;
					iBestValue = iValue;
				}
			}
		}
	}

	return eBestUnit;
}

//	--------------------------------------------------------------------------------
void CvBarbarians::DoUnits(const std::vector<int>&spawnPointsX, const std::vector<int>&spawnPointsY, int* spawnCounter)
{
	if (!CanBarbariansSpawn())
	{
		return;
	}

	const CvMap& kMap = GC.getMap();


	// more like apeshit barbarians
	// spawn all over map every so often
	if (GC.getGame().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		const CvPlayer& barbPlayer = GET_PLAYER(BARBARIAN_PLAYER);
		const int numUnits = barbPlayer.getNumUnits();
		const int maxX = kMap.getGridWidth();
		const int maxY = kMap.getGridHeight();
		const int targetUnits = (maxX * maxY) / GC.getTILES_PER_BARB();
		int numUnitsToSpawn = max(0, targetUnits - numUnits);

		// limit spawn rate
		if (numUnits == 0) // assume this means initial spawn, so spawn all
		{
			numUnitsToSpawn = numUnitsToSpawn;
		}
		else // respawning, so limit
		{
			const int minToSpawn = numUnitsToSpawn != 0 ? 1 : 0; // if we should spawn any, always spawn at least 1
			numUnitsToSpawn = max(minToSpawn, targetUnits / 10);
		}

		const int inverseX = 10000 / maxX;
		const int inverseY = 10000 / maxY;
		// spawn all over map
		for (int i = 0; i < numUnitsToSpawn; i++)
		{
			const int x = max(0, spawnPointsX[*spawnCounter] / inverseX) % maxX;
			const int y = max(0, spawnPointsY[*spawnCounter] / inverseY) % maxY;
			const CvPlot* pLoopPlot = kMap.plot(x, y);
			const bool didSpawn = DoSpawnBarbarianUnit(pLoopPlot, false, true, false);
			if (!didSpawn)
				i--; // back up and try again
			(*spawnCounter) = ((*spawnCounter) + 1) % spawnPointsX.size();
		}
	}

	// spawn from camps, but ONLY on center plot
	const ImprovementTypes eCamp = (ImprovementTypes)GC.getBARBARIAN_CAMP_IMPROVEMENT();
	for (int iPlotLoop = 0; iPlotLoop < kMap.numPlots(); iPlotLoop++)
	{
		CvPlot* pLoopPlot = kMap.plotByIndexUnchecked(iPlotLoop);

		// Found a Camp to spawn near
		if (pLoopPlot->getImprovementType() == eCamp)
		{
			if (ShouldSpawnBarbFromCamp(pLoopPlot))
			{
				DoSpawnBarbarianUnit(pLoopPlot, false, true, true);
				DoCampActivationNotice(pLoopPlot);
			}
		}
	}
}

UnitAITypes getRandBarbUnitType(const int randVal, const bool isWater)
{
	if (isWater)
	{
		const int count = 2;
		UnitAITypes possible[] = {
			UNITAI_ATTACK_SEA,
			UNITAI_ASSAULT_SEA,
		};
		return possible[randVal % count];
	}
	else
	{
		const int count = 5;
		UnitAITypes possible[] = {
			UNITAI_ATTACK,
			UNITAI_CITY_BOMBARD,
			UNITAI_FAST_ATTACK,
			UNITAI_DEFENSE,
			UNITAI_COUNTER, // 5
		};
		return possible[randVal % count];
	}
}

bool CvBarbarians::DoSpawnBarbarianUnit(const CvPlot * pPlot, bool, bool bFinishMoves, const bool onlyConsiderCenterPlot)
{
	if (pPlot == NULL)
		return false;

	CvGame& kGame = GC.getGame();

	//// is this camp empty - first priority is to fill it
	//const ImprovementTypes eCamp = (ImprovementTypes)GC.getBARBARIAN_CAMP_IMPROVEMENT();
	//if (pPlot->getImprovementType() == eCamp && pPlot->GetNumCombatUnits() == 0)
	//{
	//	const UnitTypes eUnit = GetRandomBarbarianUnitType(GC.getMap().getArea(pPlot->getArea()), UNITAI_FAST_ATTACK);

	//	if (eUnit != NO_UNIT)
	//	{
	//		CvUnit* pUnit = GET_PLAYER(BARBARIAN_PLAYER).initUnit(eUnit, pPlot->getX(), pPlot->getY(), UNITAI_FAST_ATTACK);
	//		pUnit->finishMoves();
	//		return;
	//	}
	//}

//	const int iRange = GC.getMAX_BARBARIANS_FROM_CAMP_NEARBY_RANGE();
//	// Look at nearby Plots to see if there are already too many Barbs nearby
//	int iNumNearbyUnits = 0;
//
//#ifdef AUI_HEXSPACE_DX_LOOPS
//	int iMaxDX;
//	for (iY = -iRange; iY <= iRange; iY++)
//	{
//		iMaxDX = iRange - MAX(0, iY);
//		for (iX = -iRange - MIN(0, iY); iX <= iMaxDX; iX++) // MIN() and MAX() stuff is to reduce loops (hexspace!)
//		{
//			// No need for range check because loops are set up properly
//			const CvPlot* pNearbyPlot = plotXY(pPlot->getX(), pPlot->getY(), iX, iY);
//#else
//	for(int iX = -iRange; iX <= iRange; iX++)
//	{
//		for(int iY = -iRange; iY <= iRange; iY++)
//		{
//			// Cut off the corners of the area we're looking at that we don't want
//			const CvPlot* pNearbyPlot = plotXYWithRangeCheck(pPlot->getX(), pPlot->getY(), iX, iY, iRange);
//#endif
//
//			if(pNearbyPlot != NULL)
//			{
//				if(pNearbyPlot->getNumUnits() > 0)
//				{
//					for (int iNearbyUnitLoop = 0; iNearbyUnitLoop < pNearbyPlot->getNumUnits(); iNearbyUnitLoop++)
//					{
//						const CvUnit* const unit = pNearbyPlot->getUnitByIndex(iNearbyUnitLoop);
//						if (unit != NULL && unit->isBarbarian())
//						{
//							iNumNearbyUnits++;
//						}
//					}
//				}
//			}
//		}
//	}

	const int randVal = kGame.getJonRandNum(50000, "Barb Unit Location Spawn Roll", pPlot, kGame.getGameTurn() * 137);
	const int canSpawnInBordersChance = 40;
	const bool canSpawnInBorders = ((randVal % 100) < canSpawnInBordersChance); // 40% chance

	//if(iNumNearbyUnits <= /*2*/ GC.getMAX_BARBARIANS_FROM_CAMP_NEARBY() || bIgnoreMaxBarbarians)
	{
		// Barbs only get boats after some period of time has passed
		// GC.getBARBARIAN_NAVAL_UNIT_START_TURN_SPAWN();

		// Look to see if adjacent Tiles are valid locations to spawn a Unit
		m_aeValidBarbSpawnDirections.clear();
		const DirectionTypes maxDir = onlyConsiderCenterPlot ? (DirectionTypes)(NO_DIRECTION + 1) : NUM_DIRECTION_TYPES;
		for (int iDirectionLoop = NO_DIRECTION; iDirectionLoop < maxDir; iDirectionLoop++)
		{
			const DirectionTypes eDirection = (DirectionTypes)iDirectionLoop;
			const CvPlot* pLoopPlot = plotDirection(pPlot->getX(), pPlot->getY(), eDirection);

			if (pLoopPlot != NULL)
			{
				const bool isInBorders = pLoopPlot->getOwner() != NO_PLAYER;
				if (!canSpawnInBorders && isInBorders)
					continue; // is within borders and can't spawn there

				if (pLoopPlot->getNumUnits() == 0) // must not be existing units
				{
					// valid location?
					if (!pLoopPlot->isImpassable() && !pLoopPlot->isMountain() && !pLoopPlot->isCity() && !pLoopPlot->isLake())
					{
						m_aeValidBarbSpawnDirections.push_back(eDirection);
					}
				}
			}
		}

		// Any valid locations?
		if (m_aeValidBarbSpawnDirections.size() > 0)
		{
			const int iIndex = randVal % m_aeValidBarbSpawnDirections.size();
			const DirectionTypes eDirection = (DirectionTypes)m_aeValidBarbSpawnDirections[iIndex];
			const CvPlot* pSpawnPlot = plotDirection(pPlot->getX(), pPlot->getY(), eDirection);

			const UnitAITypes eUnitAI = getRandBarbUnitType(randVal, pSpawnPlot->isWater());
			const UnitTypes eUnit = GetRandomBarbarianUnitType(GC.getMap().getArea(pSpawnPlot->getArea()), eUnitAI);

			if (eUnit != NO_UNIT)
			{
				CvUnit* pUnit = GET_PLAYER(BARBARIAN_PLAYER).initUnit(eUnit, pSpawnPlot->getX(), pSpawnPlot->getY(), eUnitAI);
				if (bFinishMoves)
				{
					pUnit->finishMoves();
				}
			}
			return true;
		}
	}
	return false;
}
bool plotHasEnemyStuff(CvUnit* pLoopUnit, CvPlot* plot)
{
	const bool dontConsiderDeadUnits = true; // if we DO consider dead units, attacks can fail
	const bool isCity = plot->isEnemyCity(*pLoopUnit);
	const bool isUnit = plot->isVisibleEnemyUnit(pLoopUnit, dontConsiderDeadUnits);
	return isCity || isUnit;
}
void doAttack(CvUnit * pLoopUnit, bool isRangedAttack, int x, int y)
{
	// TODO TASK_RANGED_ATTACK is for cities doing attacks
	//const CvPlot& targetPlot = *GC.getMap().plot(x, y);
	//if (targetPlot.isCity()) 
	//{
	//	pCity->doTask(TASK_RANGED_ATTACK, pTarget->GetTargetX(), pTarget->GetTargetY(), 0);
	//}
	//else
	//{
	const DomainTypes domainForAttacker = pLoopUnit->getDomainType();
	if (pLoopUnit->IsNuclearWeapon())
	{
		pLoopUnit->PushMission(CvTypes::getMISSION_NUKE(), x, y);
	}
	else if (isRangedAttack && domainForAttacker != DOMAIN_AIR)	// Air attack is ranged, but it goes through the move mission
	{
		pLoopUnit->PushMission(CvTypes::getMISSION_RANGE_ATTACK(), x, y);
	}
	else
	{
		pLoopUnit->PushMission(CvTypes::getMISSION_MOVE_TO(), x, y);
	}
}
bool shouldMoveToward(CvUnit * pLoopUnit, CvPlot * plot)
{
	const DomainTypes domainForAttacker = pLoopUnit->getDomainType();

	const bool shouldMoveTowards = plotHasEnemyStuff(pLoopUnit, plot);
	if (shouldMoveTowards)
	{
		const bool isCity = plot->isEnemyCity(*pLoopUnit);
		const bool canMoveToPlot = plot->isValidDomainForAction(*pLoopUnit);
		if (canMoveToPlot)
		{
			return true;
		}
		else if (isCity)
		{
			return true;
		}
	}
	return false;
}
void doMove(CvUnit * pLoopUnit, bool isRangedAttack, int x, int y)
{
	pLoopUnit->PushMission(CvTypes::getMISSION_MOVE_TO(), x, y);
}
// checks whether the plot can work for an attack, ignoring range
bool isAcceptableDomainAttackTarget(CvUnit * pLoopUnit, CvPlot * plot, bool isRangeAttack)
{
	if (isRangeAttack) // a range attack can hit any domain
		return true;

	// a non range attack is a mobile attack
	if (pLoopUnit->getDomainType() == DOMAIN_LAND)
		return plot->CanBeUsedAsLand();
	if (pLoopUnit->getDomainType() == DOMAIN_SEA)
		return plot->CanBeUsedAsWater(pLoopUnit->getOwner()) || plot->isEnemyCity(*pLoopUnit);
	if (pLoopUnit->getDomainType() == DOMAIN_IMMOBILE)
		return false;
	if (pLoopUnit->getDomainType() == DOMAIN_HOVER) // hover units can go anywhere
		return true;
	if (pLoopUnit->getDomainType() == DOMAIN_AIR) // air units can attack anywhere
		return true;

	CvAssert(false); // domain type wasn't handled
	return false;
}
// should we attack the given plot?
bool shouldAttackPlot(CvUnit * pLoopUnit, CvPlot * plot, bool isRangeAttack)
{
	// can we ATTACK the plot type? eg, are we a land unit attacking into water?
	const bool canAttackPlot = isAcceptableDomainAttackTarget(pLoopUnit, plot, isRangeAttack);
	if (!canAttackPlot)
		return false;

	// can we range attack or move there now?
	int x = plot->getX();
	int y = plot->getY();
	bool shouldAttack = false;
	const bool plotHasEnemy = plotHasEnemyStuff(pLoopUnit, plot);
	if (plotHasEnemy)
	{
		if (pLoopUnit->IsNuclearWeapon())
		{
			const bool canNukeThePlot = pLoopUnit->canNukeAt(pLoopUnit->plot(), x, y);
			shouldAttack = canNukeThePlot;
		}
		else if (isRangeAttack)
		{
			const bool canRangeAttackNow = pLoopUnit->canRangeStrikeAt(x, y, false);
			shouldAttack = canRangeAttackNow;
		}
		else
		{
			const bool couldMoveThereThisTurn = TurnsToReachTarget(pLoopUnit, plot, false, false, false) <= 1;
			shouldAttack = couldMoveThereThisTurn;
		}
	}
	return shouldAttack;
}
// try to attack nearby stuff
void tryDoAttacks(CvUnit * pLoopUnit)
{
	// TODO ask unit if we should be attacking at all
	// air units should not attack if low health
	const bool isRangeAttack = pLoopUnit->IsCanAttackRanged();
	const bool shouldAvoidCombatBecauseInjured = pLoopUnit->getHealthPercent() < 35;
	const bool isAirUnit = pLoopUnit->getDomainType() == DOMAIN_AIR;
	const bool isMeleeUnit = !isRangeAttack;
	if ((isAirUnit || isMeleeUnit) && shouldAvoidCombatBecauseInjured)
		return;

	bool canDoAttacks = true;
	for (int attempts = 0; canDoAttacks && attempts < 5; attempts++) // limit the number of attempts
	{
		// update whether we can actually attack, fail early in the loop
		canDoAttacks &= !pLoopUnit->IsDead() && !pLoopUnit->isDelayedDeath() && pLoopUnit->getMoves() > 0 && !pLoopUnit->isOutOfAttacks();

		const int unitAttackRange = isRangeAttack ? pLoopUnit->GetRange() : pLoopUnit->getMovesPlots();

		// get plots in attack range
		CvPlot* unitPlot = pLoopUnit->plot();
		const int randSeed = GC.getGame().getGameTurn() * 45 + unitPlot->getX() * 134 + unitPlot->getY() * 235;
		vector<CvPlot*> plots = unitPlot->GetAdjacentPlotsRadiusRange(1, unitAttackRange);
		shuffleVector(&plots, randSeed); // randomize attacks, be unpredictable!

		// find attack options
		bool didAnyAttacks = false;
		for (int i = 0; i < (int)plots.size(); ++i)
		{
			CvPlot* plot = plots[i];
			int x = plot->getX();
			int y = plot->getY();

			bool willNowAttackPlot = shouldAttackPlot(pLoopUnit, plot, isRangeAttack);
			if (willNowAttackPlot)
			{
				doAttack(pLoopUnit, isRangeAttack, x, y);
				didAnyAttacks = true;
				break;
			}
		}
		if (!didAnyAttacks) // we found no attacks to do on this pass, so we must not be able to attack
			canDoAttacks = false; // so get out of here
	}
}
// try to move in some intelligent way
void tryMove(CvUnit * pLoopUnit)
{
	const bool isRangeAttack = pLoopUnit->IsCanAttackRanged();
	const int moveRange = pLoopUnit->getMovesPlots();

	// get nearby plots
	CvPlot* unitPlot = pLoopUnit->plot();
	const int randSeed = GC.getGame().getGameTurn() * 49 + unitPlot->getX() * 174 + unitPlot->getY() * 335;
	vector<CvPlot*> plots = unitPlot->GetAdjacentPlotsRadiusRange(1, 4);
	shuffleVector(&plots, randSeed); // randomize moves, be unpredictable!

	// try move towards enemies
	bool didMove = false;
	if (!didMove)
	{
		for (int i = 0; !didMove && i < (int)plots.size(); ++i)
		{
			CvPlot* plot = plots[i];
			const bool shouldMoveTowards = shouldMoveToward(pLoopUnit, plot);
			if (shouldMoveTowards)
			{
				unitPlot = pLoopUnit->plot();
				doMove(pLoopUnit, isRangeAttack, plot->getX(), plot->getY());
				didMove = unitPlot != pLoopUnit->plot();
			}
		}
	}
	
	// otherwise try moving towards a random plot
	if (!didMove)
	{
		for (int i = 0; !didMove && i < (int)plots.size(); ++i)
		{
			CvPlot* plot = plots[i];
			const int distance = unitPlot->distanceTo(plot);
			if (distance <= moveRange)
			{
				unitPlot = pLoopUnit->plot();

				// we can't move to the plot directly often because the unit prevents us from moving
				// so try moving to an adjacent plot
				vector<CvPlot*> adjacentPlots = unitPlot->GetAdjacentPlots();
				shuffleVector(&adjacentPlots, randSeed + 75); // randomize moves, be unpredictable!
				for (int j = 0; !didMove && j < (int)adjacentPlots.size(); ++j)
				{
					CvPlot* adjacentPlot = adjacentPlots[j];
					doMove(pLoopUnit, isRangeAttack, adjacentPlot->getX(), adjacentPlot->getY());
					didMove = unitPlot != pLoopUnit->plot();
				}
			}
		}
	}

	pLoopUnit->finishMoves();
	pLoopUnit->SetTurnProcessed(true);
}
void CvPlayerAI::DoTurnMoves_Barbarian()
{
	bool shouldRetreat = false;
	int iLoop = 0;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		stringstream s;
		s << "DoTurnMoves_Barbarian " << pLoopUnit->getOwner() << " " << pLoopUnit->GetID();
		GC.debugState(s); // CvPlayerAI::DoTurnMoves_Barbarian

		// prioritize attacking
		pLoopUnit->ClearMissionQueue(); // forget anything else we were doing
		tryDoAttacks(pLoopUnit);
		tryMove(pLoopUnit);
	}
}
