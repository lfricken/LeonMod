#include "CvGameCoreDLLPCH.h"
#include "Lua/CvLuaSupport.h"
#include "Lua/CvLuaCity.h"
#include "Lua/CvLuaGame.h"
#include "Lua/CvLuaPlot.h"
#include "Lua/CvLuaUnit.h"
#include "Lua/CvLuaLeague.h"

#include "CvGame.h"
#include "CvGameCoreUtils.h"
#include "CvInternalGameCoreUtils.h"
#include "CvGameTextMgr.h"
#include "CvReplayMessage.h"


//------------------------------------------------------------------------------
int CvLuaGame::lGetAdditionalHelpBuilding(lua_State* L)
{
	string info = "";

	// 0 building, 1 unit, 2 project, 3 process
	const int type = (BuildingTypes)lua_tointeger(L, 1);
	const PlayerTypes ePlayer = (PlayerTypes)lua_tointeger(L, 2);
	const CvPlayer& rPlayer = GET_PLAYER(ePlayer);
	const int id = lua_tointeger(L, 3);
	stringstream s;

	if (type == 0)
	{
		const BuildingTypes eBuilding = (BuildingTypes)id;
		const CvBuildingEntry* thisBuildingEntry = GC.getBuildingInfo(eBuilding);
		if (thisBuildingEntry != NULL)
		{
			const CvBuildingClassInfo& kBuildingClassInfo = thisBuildingEntry->GetBuildingClassInfo();
			// warn about world wonder costs
			if (isWorldWonderClass(kBuildingClassInfo))
			{
				const int costIncreasePerWonder = GC.getWONDER_COST_INCREASE();
				s << "[NEWLINE][NEWLINE]Every {WORLD_WONDER} will cost an additional [COLOR_NEGATIVE_TEXT]+";
				s << costIncreasePerWonder;
				s << "% [ENDCOLOR][ICON_PRODUCTION] for each World Wonder already in the City.";
			}
		}
	}
	else if (type == 3)
	{
		const ProcessTypes eProcess = (ProcessTypes)id;

		if (eProcess == 7) // PROCESS_MAKE_TRADEROUTES
		{
			const int have = rPlayer.GetCompetitionHammersT100(HAMMERCOMPETITION_MAKE_TRADE_ROUTES) / 100;
			int numBonusRoutesHave;
			int iProgress;
			rPlayer.GetTradeRouteProjectInfo(&numBonusRoutesHave, &iProgress);
			const int need = rPlayer.GetTradeRouteCost(numBonusRoutesHave + 1);
			const int routesPer = 1;
			s << " Completing this project will provide +" << routesPer << " {TRADE_ROUTE} in your civilization permanently.";
			s << "[NEWLINE][NEWLINE]This project has yielded +" << numBonusRoutesHave * routesPer << " {TRADE_ROUTE}s so far and ";
			s << "you have " << iProgress << " of " << need << " [ICON_PRODUCTION] Production needed for your next one. ";
			s << "The cost will increase by " << rPlayer.GetTradeRouteCostIncrease() << " [ICON_PRODUCTION] each time.";
		}
		else if (eProcess == 8) // HAMMERCOMPETITION_NATIONAL_GAMES
		{
			const int have = rPlayer.GetCompetitionHammersT100(HAMMERCOMPETITION_NATIONAL_GAMES) / 100;
			int iNumCompleted;
			int iProgress;
			rPlayer.GetNationalGamesProjectInfo(&iNumCompleted, &iProgress);
			const int need = rPlayer.GetNationalGamesCost(iNumCompleted + 1);
			const int happinessPer = rPlayer.GetNationalGamesHappinessPerProject();
			s << " Completing this project will yield +" << happinessPer << " {HAPPINESS} in your civilization permanently.";
			s << "[NEWLINE][NEWLINE]This project has yielded +" << iNumCompleted * happinessPer << " {HAPPINESS} so far and ";
			s << "you have " << iProgress << " of " << need << " [ICON_PRODUCTION] Production needed to successfully host the next one. ";
			s << "The cost will increase by " << rPlayer.GetNationalGamesCostIncrease() << " [ICON_PRODUCTION] each time.";
		}
	}


	info += GetLocalizedText(s.str().c_str());
	lua_pushstring(L, info.c_str());
	return 1;
}
