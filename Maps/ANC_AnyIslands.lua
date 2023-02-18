------------------------------------------------------------------------------
--	FILE:	 Lekmapv2.2.lua (Modified Pangaea_Plus.lua)
--	AUTHOR:  Original Bob Thomas, Changes HellBlazer, lek10, EnormousApplePie, Cirra, Meota
--	PURPOSE: Global map script - Simulates a Pan-Earth Supercontinent, with
--		   numerous tectonic island chains.
------------------------------------------------------------------------------
--	Copyright (c) 2011 Firaxis Games, Inc. All rights reserved.
------------------------------------------------------------------------------

include("ANC_Core");
include("ANC_SpaceStartPlots");

------------------------------------------------------------------------------
function GetMapScriptInfo()
	--local world_age, temperature, rainfall, sea_level, resources = GetCoreMapOptions()
	return {
		Name = "Any Islands",
		Description = "Customizable island generator. Plays best on a square map. ONLY supports up to 16 players.",
		IsAdvancedMap = false,
		IconIndex = 1,
		SortIndex = 2,
		SupportsMultiplayer = true,
		CustomOptions = {

			{
				Name = "TXT_KEY_MAP_OPTION_RESOURCES",	-- add setting for resources (13)
				Values = {
					"1 -- Nearly Nothing",
					"2",
					"3",
					"4",
					"5 -- Default",
					"6",
					"7",
					"8",
					"9",
					"10 -- Almost no normal tiles left",
				},

				DefaultValue = 5,
				SortPriority = -87,
			},
		},
	};
end

local g_tilesPerMajorCiv = 500;

------------------------------------------------------------------------------
function GetMapInitData(worldSize)
	-- calculate world size
	local numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds = ANC_GetPlayerAndTeamInfo();
	local _, _, ratio = ANC_getMajorCivSpawnPoints(numMajorCivs, 20, 20);
	print(numMajorCivs);
	--print("RATIO: " .. ratio[1] .. "," .. ratio[2]);
	local grid_size = ANC_calcMapSizeXy(numMajorCivs, numMinors, ratio, g_tilesPerMajorCiv);
	--print("NEWSIZE: " .. grid_size[1] .. "," .. grid_size[2]);


	--local world = GameInfo.Worlds[worldSize];
	if (grid_size ~= nil) then
		return {
			Width = grid_size[1],
			Height = grid_size[2],
			WrapX = true,
		};
	else
		print("WARNING in GetMapInitData. Invalid dimension data.")
		return {
			Width = 90,
			Height = 90,
			WrapX = true,
		};
	end
end

--Entry point defined in CvMapGenerator::pGenerateRandomMap
function GenerateMap()
	print("GenerateMap Begin");

	Map.RandSeed(9999); -- helps see changes by guaranteeing the same map gets generated

	args = ANC_CreateArgs();
	ANC_CreateMap(args);




	--printMapToLua(plotTypes, maxX, maxY);
	print("GenerateMap End");
end


