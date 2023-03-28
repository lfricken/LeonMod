/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#pragma once

#ifndef CIV5_TRADE_CLASSES_H
#define CIV5_TRADE_CLASSES_H

// The map layer the trade units reside on
#define TRADE_UNIT_MAP_LAYER	1

struct TradeConnectionPlot
{
	int m_iX;
	int m_iY;
};

#define PROJECTED_MAX_TRADE_LENGTH 40
#ifdef AUI_TRADE_FIX_FSTATICVECTOR_CONTENTS_ARE_POD
typedef FStaticVector<TradeConnectionPlot, PROJECTED_MAX_TRADE_LENGTH, true, c_eCiv5GameplayDLL> TradeConnectionPlotList;
#else
typedef FStaticVector<TradeConnectionPlot, PROJECTED_MAX_TRADE_LENGTH, false, c_eCiv5GameplayDLL> TradeConnectionPlotList;
#endif

struct TradeConnection
{
	int m_iID;
	int m_iOriginX;
	int m_iOriginY;
	int m_iDestX;
	int m_iDestY;
	PlayerTypes m_eOriginOwner;
	PlayerTypes m_eDestOwner;
	DomainTypes m_eDomain;
	TradeConnectionType m_eConnectionType;
#ifdef AUI_WARNING_FIXES
	uint m_iTradeUnitLocationIndex;
#else
	int m_iTradeUnitLocationIndex;
#endif
	bool m_bTradeUnitMovingForward;
	TradeConnectionPlotList m_aPlotList;
	// expected to be 100 per tile normally, lower for roads, higher for something bad
	int m_routeCost;
	// this is deprecated/dead
	bool m_hasEverBeenCalculated;
	int m_pirateFactor;
	int m_rangeFactor;
	int m_unitID;

	int m_iCircuitsCompleted;
	int m_iCircuitsToComplete;
	int m_iTurnRouteComplete;
	int m_aiOriginYields[NUM_YIELD_TYPES];
	int m_aiDestYields[NUM_YIELD_TYPES];

	bool isPathStillValid() const;
#ifdef AUI_EXPLICIT_DESTRUCTION
	~TradeConnection()
	{
		m_aPlotList.clear();
	}
#endif
	// true if this route has any yield greater than 0
	bool HasAnyYield() const;
	int GetNumEnemyUnitsOnRoute() const;
	// returns 100 if this should yield full, or 50 if half etc., due to range penalty
	static int RouteRangeFractionT100(int routeCost, int rangeInTiles, int maxFactorT100);
	CvPlot* GetRoutePlot(int idx) const;
	// determine type of route
	TradeRouteType GetRouteType() const;
private:
	// if this route requires 10, and the range is 5, this would return 200 signifying
	// that this route requires 200% range
	static int RouteRangeFactorT100(int routeCost, int rangeInTiles);
};

#define PROJECTED_MAX_TRADE_CONNECTIONS_PER_CIV 30
#define LIKELY_NUM_OF_PLAYERS 16
#define PROJECTED_MAX_TRADE_CONNECTIONS (PROJECTED_MAX_TRADE_CONNECTIONS_PER_CIV * LIKELY_NUM_OF_PLAYERS)

typedef FStaticVector<TradeConnection, PROJECTED_MAX_TRADE_CONNECTIONS, false, c_eCiv5GameplayDLL > TradeConnectionList;

class CvGameTrade
{
public:
	CvGameTrade(void);
	~CvGameTrade(void);

	void Init (void);
	void Uninit (void);
	void Reset (void);

#ifdef AUI_YIELDS_APPLIED_AFTER_TURN_NOT_BEFORE
	void DoTurn(PlayerTypes eForPlayer = NO_PLAYER);
#else
	void DoTurn (void);
#endif

	bool CanCreateTradeRoute(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType, bool bIgnoreExisting, bool bCheckPath = true) const;
	bool CanCreateTradeRoute(PlayerTypes eOriginPlayer, PlayerTypes eDestPlayer, DomainTypes eDomainRestriction) const;
	bool IsDuplicate(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType) const;

	
	bool TryCreateTradeRoute(DomainTypes eDomain, const CvCity* pOriginCity, const CvCity* pDestCity, TradeConnectionType type, TradeConnection* con, bool skipCheck = false) const;
	bool CreateTradeRoute(CvCity* pOriginCity, CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType, int& iRouteID);
	bool CalcRouteInfo(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes domain, int* numTurns = NULL, 
		int* pCircuits = NULL, CvAStarNode** pPathfinderNodeOut = NULL) const;

	bool IsValidTradeRoutePath(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes eDomain) const;
	CvPlot* GetPlotAdjacentToWater(const CvPlot* pTarget, const CvPlot* pOrigin) const;

	bool IsDestinationExclusive(const TradeConnection& kTradeConnection) const;
	bool IsConnectionInternational(const TradeConnection& kTradeConnection) const;

	bool IsCityConnectedToPlayer(const CvCity* pCity, PlayerTypes eOtherPlayer, bool bOnlyOwnedByCityOwner) const;
	bool IsPlayerConnectedToPlayer(PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer) const;
	int CountNumPlayerConnectionsToPlayer(PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer) const;

	bool IsCityConnectedToCity(const CvCity* pFirstCity, const CvCity* pSecondCity) const;
	bool IsCityConnectedFromCityToCity(const CvCity* pOriginCity, const CvCity* pDestCity) const;

	int GetNumTimesOriginCity(const CvCity* pCity, bool bOnlyInternational) const;
	int GetNumTimesDestinationCity(const CvCity* pCity, bool bOnlyInternational) const;

	void CopyPathIntoTradeConnection(const CvAStarNode* pNode, TradeConnection* pTradeConnection) const;

	int GetDomainModifierTimes100(DomainTypes eDomain) const;

	int GetEmptyTradeRouteIndex() const;
	bool IsTradeRouteIndexEmpty(int iIndex) const;
	bool EmptyTradeRoute (int iIndex);
#ifdef AUI_ITERATORIZE
	TradeConnectionList::iterator GetEmptyTradeRouteIter();
	TradeConnectionList::const_iterator GetEmptyTradeRouteIter() const;
	bool IsTradeRouteIndexEmpty(const TradeConnectionList::iterator it) const;
	bool IsTradeRouteIndexEmpty(const TradeConnectionList::const_iterator it) const;
#endif

	void ClearAllCityTradeRoutes (CvPlot* pPlot); // called when a city is captured or traded
	void ClearAllCivTradeRoutes (PlayerTypes ePlayer); // called from world congress code
	void ClearAllCityStateTradeRoutes (void); // called from world congress code
#ifdef NQM_TEAM_TRADE_ROUTES_CANCELLED_NOT_DESTROYED_FOR_WAR_DEFENDER_ON_DOW
	void CancelTradeBetweenTeams(TeamTypes eTeam1, TeamTypes eTeam2, bool bReturnUnits = false);
	void InvalidateTradeBetweenTeams(TeamTypes eOriginTeam, TeamTypes eDestinationTeam);
#else
	void CancelTradeBetweenTeams (TeamTypes eTeam1, TeamTypes eTeam2);
#endif

	void DoAutoWarPlundering(TeamTypes eTeam1, TeamTypes eTeam2); // when war is declared, both sides plunder each others trade routes for cash!


	int GetNumTradeRoutesInPlot(const CvPlot* pPlot) const;

	int GetIndexFromID(int iID) const;
	PlayerTypes GetOwnerFromID(int iID) const;
	PlayerTypes GetDestFromID(int iID) const;
	DomainTypes GetDomainFromID(int iID) const;

	int GetIndexFromUnitID(int iUnitID, PlayerTypes eOwner) const;
	bool IsUnitIDUsed(int iUnitID) const;

	static CvCity* GetOriginCity(const TradeConnection& kTradeConnection);
	static CvCity* GetDestCity(const TradeConnection& kTradeConnection);

#ifdef AUI_YIELDS_APPLIED_AFTER_TURN_NOT_BEFORE
	void ResetTechDifference(PlayerTypes eForPlayer = NO_PLAYER);
	void BuildTechDifference(PlayerTypes eForPlayer = NO_PLAYER);
#else
	void ResetTechDifference ();
	void BuildTechDifference ();
#endif

	int GetTechDifference(PlayerTypes ePlayer, PlayerTypes ePlayer2) const;


	void CreateVis (int iIndex); // Create the trade unit vis unit
#ifdef AUI_WARNING_FIXES
	CvUnit* GetVis(uint iIndex) const;
#else
	CvUnit* GetVis(int iIndex);
#endif
	// trade unit movement
	bool MoveUnit (int iIndex); // move a trade unit along its path for all its movement points
	bool StepUnit (int iIndex); // move a trade unit a single step along its path (called by MoveUnit)
#ifdef AUI_ITERATORIZE
	bool MoveUnit(TradeConnectionList::iterator it); // move a trade unit along its path for all its movement points
	bool StepUnit(TradeConnectionList::iterator it); // move a trade unit a single step along its path (called by MoveUnit)
#endif

	void DisplayTemporaryPopupTradeRoute(int iPlotX, int iPlotY, TradeConnectionType type, DomainTypes eDomain);
	void HideTemporaryPopupTradeRoute(int iPlotX, int iPlotY, TradeConnectionType type);

	CvString GetLogFileName() const;
	void LogTradeMsg(CvString& strMsg);

	TradeConnectionList m_aTradeConnections;
	int m_iNextID; // used to assign IDs to trade routes to avoid confusion when some are disrupted in multiplayer

	int m_aaiTechDifference[MAX_MAJOR_CIVS][MAX_MAJOR_CIVS];

	struct {
		int iPlotX, iPlotY;
		TradeConnectionType type;
	} m_CurrentTemporaryPopupRoute;
};

FDataStream& operator>>(FDataStream&, CvGameTrade&);
FDataStream& operator<<(FDataStream&, const CvGameTrade&);

struct TradeConnectionWasPlundered
{
	TradeConnection m_kTradeConnection;
	int m_iTurnPlundered;
};

#ifdef AUI_TRADE_FIX_POSSIBLE_DEALLOCATION_CRASH
typedef FStaticVector<TradeConnectionWasPlundered, PROJECTED_MAX_TRADE_CONNECTIONS_PER_CIV, false, c_eCiv5GameplayDLL> TradeConnectionWasPlunderedList;
#else
typedef FStaticVector<TradeConnectionWasPlundered, 10, false, c_eCiv5GameplayDLL > TradeConnectionWasPlunderedList;
#endif

class CvPlayerTrade
{
public:
	CvPlayerTrade(void);
	~CvPlayerTrade(void);
	void Init(CvPlayer* pPlayer);
	void Uninit(void);
	void Reset(void);

	// Functions invoked each player turn
	void DoTurn(void);
	void MoveUnits(void);
	int GetTradeConnectionValueExtra(const TradeConnection& kTradeConnection, const YieldTypes eYieldType, const bool bIsOwner, std::stringstream* tooltip) const;

	int GetTradeConnectionBaseValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;
	int GetTradeConnectionGPTValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer, bool bOriginCity) const;
	int GetTradeConnectionResourceValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;
	int GetTradeConnectionYourBuildingValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;
	int GetTradeConnectionTheirBuildingValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;
	int GetTradeConnectionExclusiveValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield) const;
	int GetTradeConnectionPolicyValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield) const;
	int GetTradeConnectionOtherTraitValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;
	int GetTradeConnectionDomainValueModifierTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield) const;
	int GetTradeConnectionRiverValueModifierTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, bool bAsOriginPlayer) const;

	string GetTradeRouteTooltip(const TradeConnection& kTradeConnection) const;
	int CalcTradeConnectionValueTotalForPlayerTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, std::stringstream* tooltip) const;
	int CalcTradeConnectionValueTimes100(const TradeConnection& kTradeConnection, YieldTypes eYield, 
		bool bAsOriginPlayer, std::stringstream* tooltip = NULL, bool isTurnUpdate = false) const;

	// set yields into the TradeConnection
	void UpdateYieldsFor(TradeConnection* con, bool isTurnUpdate, bool forceCorrectYieldCalculation) const;
	// updates the all the values for the trade routes that go to and from this player
	void UpdateTradeConnectionValues(bool isTurnUpdate = false);

	int GetTradeValuesAtCityTimes100(const CvCity* const pCity, YieldTypes eYield) const;
	// number of trade routes that start here
	int CalcNumTradeRoutesOriginatingFromExcept(const CvCity* const pCity, const TradeConnection& except) const;

	int GetAllTradeValueTimes100(YieldTypes eYield) const;
	int GetAllTradeValueFromPlayerTimes100(YieldTypes eYield, PlayerTypes ePlayer) const;

	bool IsConnectedToPlayer(PlayerTypes eOtherPlayer) const;

	bool CanCreateTradeRoute(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType, bool bIgnoreExisting, bool bCheckPath = true) const;
	bool CanCreateTradeRoute(PlayerTypes eOtherPlayer, DomainTypes eDomain) const;
	bool CanCreateTradeRoute(DomainTypes eDomain) const;


	bool CreateTradeRoute(CvCity* pOriginCity, CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType);

	// additional max range a route can go (but suffer a penalty)
	int GetRangeFactorT100() const;
	TradeConnection* GetTradeConnection(const CvCity* pOriginCity, const CvCity* pDestCity) const;
	int GetNumberOfCityStateTradeRoutes() const;

	bool IsPreviousTradeRoute(const CvCity* pOriginCity, const CvCity* pDestCity, DomainTypes eDomain, TradeConnectionType eConnectionType) const;

	int GetNumPotentialConnections(const CvCity* pFromCity, DomainTypes eDomain) const;

	std::vector<int> GetTradeUnitsAtPlot(const CvPlot* pPlot, bool bFailAtFirstFound, bool bExcludingMe, bool bOnlyWar) const;
	std::vector<int> GetTradePlotsAtPlot(const CvPlot* pPlot, bool bFailAtFirstFound, bool bExcludingMe, bool bOnlyWar) const;

	std::vector<int> GetOpposingTradeUnitsAtPlot(const CvPlot* pPlot, bool bFailAtFirstFound) const; // returns the ID of trade connections with units at that plot
	bool ContainsOpposingPlayerTradeUnit(const CvPlot* pPlot) const;

	std::vector<int> GetEnemyTradeUnitsAtPlot(const CvPlot* pPlot, bool bFailAtFirstFound) const; // returns the ID of trade connections with units at that plot
	bool ContainsEnemyTradeUnit(const CvPlot* pPlot) const;
	std::vector<int> GetEnemyTradePlotsAtPlot(const CvPlot* pPlot, bool bFailAtFirstFound) const; // returns the ID of trade connections that go through that plot
	bool ContainsEnemyTradePlot(const CvPlot* pPlot) const;

	bool PlunderTradeRoute(int iTradeConnectionID);
	// trade routes with any foriegn city (minor or major)
	int GetNumForeignTradeRoutes(PlayerTypes ePlayer);

	int GetTradeRouteRange(DomainTypes eDomain, const CvCity* pOriginCity) const;
	int GetTradeRouteSpeed(DomainTypes eDomain) const;

	uint GetNumTradeRoutesPossible() const;
	int GetNumTradeRoutesUsed(bool bContinueTraining) const;
	int GetNumTradeRoutesRemaining(bool bContinueTraining) const;

	int GetNumDifferentTradingPartners() const;

	void UpdateTradeConnectionWasPlundered();
#ifdef AUI_WARNING_FIXES
	void AddTradeConnectionWasPlundered(const TradeConnection& kTradeConnection);
#else
	void AddTradeConnectionWasPlundered(const TradeConnection kTradeConnection);
#endif

	bool CheckTradeConnectionWasPlundered(const TradeConnection& kTradeConnection) const;


	static UnitTypes GetTradeUnit (DomainTypes eDomain);

	std::vector<CvString> GetPlotToolTips(CvPlot* pPlot) const;
	std::vector<CvString> GetPlotMouseoverToolTips(CvPlot* pPlot) const;


	TradeConnectionList m_aRecentlyExpiredConnections;
	TradeConnectionWasPlunderedList m_aTradeConnectionWasPlundered;

	CvPlayer* m_pPlayer;
};

FDataStream& operator>>(FDataStream&, CvPlayerTrade&);
FDataStream& operator<<(FDataStream&, const CvPlayerTrade&);

class CvTradeAI
{
public:
	CvTradeAI(void);
	~CvTradeAI(void);
	void Init(CvPlayer* pPlayer);
	void Uninit(void);
	void Reset(void);

	void DoTurn(void);

	void GetAvailableTR(TradeConnectionList& aTradeConnectionList);
	void PrioritizeTradeRoutes(TradeConnectionList& aTradeConnectionList);


	int	ScoreInternationalTR(const TradeConnection& kTradeConnection) const;
	int ScoreFoodTR(const TradeConnection& kTradeConnection, const CvCity* pSmallestCity) const;
	int ScoreProductionTR(const TradeConnection& kTradeConnection, std::vector<const CvCity*>& aTargetCityList) const;

	bool ChooseTradeUnitTargetPlot(CvUnit* pUnit, int& iOriginPlotIndex, int& iDestPlotIndex, TradeConnectionType& eTradeConnectionType, bool& bDisband, const TradeConnectionList& aTradeConnections) const;


	int m_iRemovableValue;

	CvPlayer* m_pPlayer;
};

FDataStream& operator>>(FDataStream&, CvTradeAI&);
FDataStream& operator<<(FDataStream&, const CvTradeAI&);

#endif //CIV5_TRADE_CLASSES_H
