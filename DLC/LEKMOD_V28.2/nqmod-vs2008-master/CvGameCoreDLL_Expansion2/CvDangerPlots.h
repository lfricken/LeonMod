/*	-------------------------------------------------------------------------------------------------------
	� 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#pragma once

#ifndef CIV5_DANGER_PLOTS_H
#define CIV5_DANGER_PLOTS_H

#include "CvDiplomacyAIEnums.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvDangerPlots
//!  \brief		Used to calculate the relative danger of a given plot for a player
//
//!  Key Attributes:
//!  - Replaces the AI_getPlotDanger function in CvPlayerAI
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvDangerPlots
{
public:
	CvDangerPlots(void);
	~CvDangerPlots(void);

	void Init(PlayerTypes ePlayer, bool bAllocate);
	void Uninit();
	void Reset();

	void UpdateDanger(bool bPretendWarWithAllCivs = false, bool bIgnoreVisibility = false);
	void AddDanger(int iPlotX, int iPlotY, int iValue, bool bWithinOneMove);
	int GetDanger(const CvPlot& pPlot) const;
	bool IsUnderImmediateThreat(const CvPlot& pPlot) const;
	int GetCityDanger(CvCity* pCity);  // sums the plots around the city to determine it's danger value

	int ModifyDangerByRelationship(PlayerTypes ePlayer, CvPlot* pPlot, int iDanger);

	bool ShouldIgnorePlayer(PlayerTypes ePlayer) const;
	bool ShouldIgnoreUnit(const CvUnit* pUnit, bool bIgnoreVisibility = false) const;
	bool ShouldIgnoreCity(const CvCity* pCity, bool bIgnoreVisibility = false) const;
	bool ShouldIgnoreCitadel(const CvPlot* pCitadelPlot, bool bIgnoreVisibility = false) const;
	void AssignUnitDangerValue(CvUnit* pUnit, CvPlot* pPlot);
	void AssignCityDangerValue(CvCity* pCity, CvPlot* pPlot);

	void SetDirty();
	bool IsDirty() const
	{
		return m_bDirty;
	}

	void Read(FDataStream& kStream);
	void Write(FDataStream& kStream) const;

protected:

	bool IsDangerByRelationshipZero(PlayerTypes ePlayer, CvPlot* pPlot);

	int GetDangerValueOfCitadel() const;

	PlayerTypes m_ePlayer;
#ifndef AUI_DANGER_PLOTS_FIX_USE_ARRAY_NOT_FFASTVECTOR
	bool m_bArrayAllocated;
#endif
	bool m_bDirty;
	int m_majorWarModT100;
	int m_majorHostileModT100;
	int m_majorDeceptiveModT100;
	int m_majorGuardedModT100;
	int m_majorAfraidModT100;
	int m_majorFriendlyModT100;
	int m_majorNeutralModT100;
	int m_minorNeutralrModT100;
	int m_minorFriendlyModT100;
	int m_minorBullyModT100;
	int m_minorConquestModT100;

#ifdef AUI_DANGER_PLOTS_FIX_USE_ARRAY_NOT_FFASTVECTOR
	uint* m_DangerPlots;
#else
	FFastVector<uint, true, c_eCiv5GameplayDLL, 0> m_DangerPlots;
#endif
};

#endif //CIV5_PROJECT_CLASSES_H
