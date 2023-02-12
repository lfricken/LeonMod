------------------------------------------------------------------------------
--	FILE:	 Lekmapv2.2.lua (Modified Pangaea_Plus.lua)
--	AUTHOR:  Original Bob Thomas, Changes HellBlazer, lek10, EnormousApplePie, Cirra, Meota
--	PURPOSE: Global map script - Simulates a Pan-Earth Supercontinent, with
--		   numerous tectonic island chains.
------------------------------------------------------------------------------
--	Copyright (c) 2011 Firaxis Games, Inc. All rights reserved.
------------------------------------------------------------------------------

include("ANC_Core");

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
				Name = "TXT_KEY_MAP_OPTION_WORLD_AGE", -- 1
				Values = {
					"TXT_KEY_MAP_OPTION_THREE_BILLION_YEARS",
					"TXT_KEY_MAP_OPTION_FOUR_BILLION_YEARS",
					"TXT_KEY_MAP_OPTION_FIVE_BILLION_YEARS",
					"No Mountains",
					"TXT_KEY_MAP_OPTION_RANDOM",
				},
				DefaultValue = 2,
				SortPriority = -200,
			},

			{
				Name = "TXT_KEY_MAP_OPTION_TEMPERATURE",	-- 2 add temperature defaults to random
				Values = {
					"TXT_KEY_MAP_OPTION_COOL",
					"TXT_KEY_MAP_OPTION_TEMPERATE",
					"TXT_KEY_MAP_OPTION_HOT",
					"TXT_KEY_MAP_OPTION_RANDOM",
				},
				DefaultValue = 2,
				SortPriority = -180,
			},

			{
				Name = "TXT_KEY_MAP_OPTION_RAINFALL",	-- 3 add rainfall defaults to random
				Values = {
					"TXT_KEY_MAP_OPTION_ARID",
					"TXT_KEY_MAP_OPTION_NORMAL",
					"TXT_KEY_MAP_OPTION_WET",
					"TXT_KEY_MAP_OPTION_RANDOM",
				},
				DefaultValue = 2,
				SortPriority = -170,
			},

			{
				Name = "Sea Level",--  (4)
				Values = {
					"50", -- 1
					"52",
					"54",
					"56",
					"58", --5
					"60",
					"62",
					"64",
					"66 (Low)",
					"68", --10
					"70",
					"72",
					"74",
					"76 (Medium)", --14
					"78", --15
					"80",
					"82",
					"84", --30
					"86 (High)",
					"88",
					"90",
					"92", --30
				},

				DefaultValue = 13,
				SortPriority = -160,
			},

			{
				Name = "Start Quality",	-- 5 add resources defaults to random
				Values = {
					"Legendary Start - Strat Balance",
					"Legendary - Strat Balance + Uranium",
					"TXT_KEY_MAP_OPTION_STRATEGIC_BALANCE",
					"Strategic Balance With Coal",
					"Strategic Balance With Aluminum",
					"Strategic Balance With Coal & Aluminum",
					"TXT_KEY_MAP_OPTION_RANDOM",
				},
				DefaultValue = 2,
				SortPriority = -95,
			},

			{
				Name = "Start Distance",	-- 6 add resources defaults to random
				Values = {
					"Close",
					"Normal",
					"Far - Warning: May sometimes crash during map generation",
				},
				DefaultValue = 2,
				SortPriority = -94,
			},

			{
				Name = "Natural Wonders", -- 7 number of natural wonders to spawn
				Values = {
					"0",
					"1",
					"2",
					"3",
					"4",
					"5",
					"6",
					"7",
					"8",
					"9",
					"10",
					"11",
					"12",
					"Random",
					"Default",
				},
				DefaultValue = 15,
				SortPriority = -93,
			},

			{
				Name = "Grass Moisture",	-- add setting for grassland mositure (8)
				Values = {
					"Wet",
					"Normal",
					"Dry",
				},

				DefaultValue = 2,
				SortPriority = -92,
			},

			{
				Name = "Rivers",	-- add setting for rivers (9)
				Values = {
					"Sparse",
					"Average",
					"Plentiful",
				},

				DefaultValue = 2,
				SortPriority = -91,
			},

			{
				Name = "Tundra",	-- add setting for tundra (10)
				Values = {
					"Sparse",
					"Average",
					"Plentiful",
				},

				DefaultValue = 1,
				SortPriority = -90,
			},

			{
				Name = "Land Size X",	-- add setting for land type (11) +28
				Values = {
					"30",
					"32",
					"34",
					"36",
					"38",
					"40 (Duel)",
					"42",
					"44",
					"46",
					"48", --10
					"50",
					"52",
					"54",
					"56 (Tiny)",
					"58", --15
					"60",
					"62",
					"64",
					"66 (Small)",
					"68", --20
					"70",
					"72",
					"74",
					"76",
					"78", --25
					"80 (Standard)",
					"82",
					"84",
					"86",
					"88", --30
					"90",
					"92",
					"94",
					"96",
					"98",
					"100",
					"102",
					"104 (Large)",
					"106",
					"108",
					"110",
					"112",
					"114",
					"116",
					"118",
					"120",
					"122",
					"124",
					"126",
					"128 (Huge)",
				},

				DefaultValue = 23,
				SortPriority = -89,
			},

			{
				Name = "Land Size Y",	-- add setting for land type (12) +18
				Values = {
					"20",
					"22",
					"24 (Duel)",
					"26",
					"28",
					"30",
					"32",
					"34",
					"36 (Tiny)",
					"38", --10
					"40",
					"42 (Small)",
					"44",
					"46",
					"48", --15
					"50",
					"52 (Standard)",
					"54",
					"56",
					"58", --20
					"60",
					"62",
					"64 (Large)",
					"66",
					"68",
					"70",
					"72",
					"74",
					"76", --30
					"78",
					"80 (Huge)",
				},

				DefaultValue = 20,
				SortPriority = -88,
			},

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

			{
				Name = "Balanced Regionals",	-- add setting for removing OP luxes from regional pool (14)
				Values = {
					"Yes",
					"No",
				},

				DefaultValue = 1,
				SortPriority = -85,
			},

			{
				Name = "Continent Type",	-- add setting for removing OP luxes from regional pool (15)
				Values = {
					"Pangea",
					"Continents",
					"Small Continents",
					"Fractal",
					"Fractal Super",
				},

				DefaultValue = 2,
				SortPriority = -75,
			},

			{
				Name = "Less Polar Land",	-- add setting for removing OP luxes from regional pool (16)
				Values = {
					"Yes",
					"No",
				},

				DefaultValue = 1,
				SortPriority = -70,
			},

			{
				Name = "Islands Per 1000",	-- odds of generating an island (17)
				Values = {
					"0",
					"2",
					"4",
					"6",
					"8",
					"10",
					"12",
					"14",
					"16",
					"18",
					"20",
					"22",
					"24",
					"26",
					"28", --15
					"30", --16
					"32",
					"34",
					"36",
					"38",
					"40",
					"42",
					"44",
					"46",
					"48",
					"50",
				},

				DefaultValue = 16,
				SortPriority = -65,
			},

			{
				Name = "Island Max Size",	-- (18)
				Values = {
					"1",
					"3",
					"5",
					"7",
					"9",
					"11",
					"13",
					"15",
					"17",
					"19",
					"21", --11
					"23",
					"25",
					"27",
					"29",
				},

				DefaultValue = 11,
				SortPriority = -60,
			},

			{
				Name = "Sea Level Modifier",	-- (19)
				Values = {
					"-1",
					" 0",
					"+1",
				},

				DefaultValue = 2,
				SortPriority = -155,
			},

			{
				Name = "Size Odds Exponent",--  (20)
				Values = {
					"(0.30^Size)%",
					"(0.40^Size)%",
					"(0.50^Size)%",
					"(0.75^Size)%", -- 4
					"(0.82^Size)%", -- 5
					"(0.86^Size)%",
					"(0.88^Size)%", -- 7
					"(0.90^Size)%",
					"(0.91^Size)%",
					"(0.92^Size)%",
					"(0.93^Size)%",
					"(0.99^Size)%",
				},

				DefaultValue = 4,
				SortPriority = -55,
			},
		},
	};
end

------------------------------------------------------------------------------
function GetSizeExponent()
	local choice = Map.GetCustomOption(20);
	local vals = {
		0.30,
		0.40,
		0.50,
		0.75,
		0.82,
		0.86,
		0.88,
		0.90,
		0.91,
		0.92,
		0.93,
		0.99
	};

	return vals[choice];
end

------------------------------------------------------------------------------
function GetMapInitData(worldSize)
	
	local LandSizeX = 28 + (Map.GetCustomOption(11) * 2);
	local LandSizeY = 18 + (Map.GetCustomOption(12) * 2);

	local worldsizes = {};

	worldsizes = {

		[GameInfo.Worlds.WORLDSIZE_DUEL.ID] = {LandSizeX, LandSizeY}, -- 720
		[GameInfo.Worlds.WORLDSIZE_TINY.ID] = {LandSizeX, LandSizeY}, -- 1664
		[GameInfo.Worlds.WORLDSIZE_SMALL.ID] = {LandSizeX, LandSizeY}, -- 2480
		[GameInfo.Worlds.WORLDSIZE_STANDARD.ID] = {LandSizeX, LandSizeY}, -- 3900
		[GameInfo.Worlds.WORLDSIZE_LARGE.ID] = {LandSizeX, LandSizeY}, -- 6076
		[GameInfo.Worlds.WORLDSIZE_HUGE.ID] = {LandSizeX, LandSizeY} -- 9424
		}
		
	local grid_size = worldsizes[worldSize];
	--
	local world = GameInfo.Worlds[worldSize];
	if (world ~= nil) then
		return {
			Width = grid_size[1],
			Height = grid_size[2],
			WrapX = true,
		}; 
	end

end

------------------------------------------------------------------------------
function GetSeaLevel()
	return Map.GetCustomOption(4)*2 + 48 + (Map.GetCustomOption(19)-2);
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


