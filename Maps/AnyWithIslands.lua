------------------------------------------------------------------------------
--	FILE:	 Lekmapv2.2.lua (Modified Pangaea_Plus.lua)
--	AUTHOR:  Original Bob Thomas, Changes HellBlazer, lek10, EnormousApplePie, Cirra, Meota
--	PURPOSE: Global map script - Simulates a Pan-Earth Supercontinent, with
--           numerous tectonic island chains.
------------------------------------------------------------------------------
--	Copyright (c) 2011 Firaxis Games, Inc. All rights reserved.
------------------------------------------------------------------------------

include("HBMapGenerator");
include("FractalWorld");
include("LeonFeet");
include("HBTerrainGenerator");
include("IslandMaker");
include("MultilayeredFractal");

------------------------------------------------------------------------------
function GetMapScriptInfo()
	local world_age, temperature, rainfall, sea_level, resources = GetCoreMapOptions()
	return {
		Name = "Any with Islands",
		Description = "Generates a standard continents map with sporadic islands. Also clears waterways near the poles.",
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

------------------------------------------------------------------------------
function GeneratePlotTypes()
	--print("Generating Plot Types (Lua Small Continents) ...");

	--Map.RandSeed(9999); -- helps see changes by guaranteeing the same map gets generated

	local inverse_continent_sizes = Map.GetCustomOption(15); -- 1 means pangea, 2 means continents, 3, means small cont, etc.
	local age = Map.GetCustomOption(1);
	local sea = GetSeaLevel();
	local maxX = Map.GetCustomOption(11)*2+28; -- get map x size
	local maxY = Map.GetCustomOption(12)*2+18; -- get map y size
	local islandPoleMinSize = 4;
	local islandSizeMin = 8;
	local islandSizeMax = Map.GetCustomOption(18)*2-1;
	local islandChance = Map.GetCustomOption(17)*2; -- chance in 1000 that an island will start generating (Standard size does 4000 checks)
	local polesIslandChance = islandChance * 2; -- chance in 1000 that an island will start generating in polar region
	local poleClearDist = 8; -- clear all land at this range
	local polesAddDist =  3; -- add small islands up to this range 
	local geometricReduction = GetSizeExponent();

	-- Fetch Sea Level and World Age user selections.
	if age == 4 then
		age = 1 + Map.Rand(3, "Random World Age - Lua");
	end

	local fractal_world = FractalWorld.Create();
	fractal_world:InitFractal{ continent_grain = inverse_continent_sizes};

	local args = {
		sea_level = 1,
		world_age = age,
		sea_level_low = sea,
		sea_level_normal = 75,
		sea_level_high = 85,
		extra_mountains = 5, -- at 0, very few mountains, at 40, ~15% of all land is mountains
		adjust_plates = 1.65, -- overlapping plates form mountains 0 forms giant mountain regions
		-- 1.5 pushes them apart a lot
		tectonic_islands = false -- should we form islands where plates overlap?
		}

	-- generate using primary continent algorithm
	local plotTypes = fractal_world:GeneratePlotTypes(args);
	--local x = fractal_world:Plots();


	-- redo mountains
	for x = 0, maxX - 1 do
		for y = 0, maxY - 1 do
			local i = GetI(x,y,maxX);
			if plotTypes[i] ~= PlotTypes.PLOT_OCEAN then
				if plotTypes[i] == PlotTypes.PLOT_MOUNTAIN then -- remove existing mountain
					plotTypes[i] = PlotTypes.PLOT_HILLS;
				end
				if Map.Rand(1000, "Mountain Chance") < 65 then -- mountain chance
					plotTypes[i] = PlotTypes.PLOT_MOUNTAIN;
				end
			end
		end
	end


	-- add random islands
	for x = 0, maxX - 1 do
		for y = 0, maxY - 1 do
			local i = GetI(x,y,maxX);
			local islandSize = GenIslandSize(islandSizeMin,islandSizeMax,geometricReduction);
			if plotTypes[i] == PlotTypes.PLOT_OCEAN then
				-- local shouldGen = (x==0 and (y==30 or y==15 or y==25 or y==35 or y==45));
				local shouldGen = Map.Rand(1000, "Island Chance") < islandChance and (not IsNearLand(plotTypes,x,y,maxX,islandSize));
				if shouldGen then
					RandomIsland(plotTypes,x,y,maxX,islandSize,80)
				end
			end
		end
	end


	-- remove near poles
	-- re add small stuff near poles 
	if Map.GetCustomOption(16)==1 then
		for x = 0, maxX - 1 do
			for y = 0, maxY - 1 do
				local i = y * maxX + x + 1;
				if y < poleClearDist or y > maxY-poleClearDist then
					plotTypes[i] = PlotTypes.PLOT_OCEAN; -- clear land
				end
				if y == poleClearDist or y == maxY-poleClearDist then
					if Map.Rand(1000, "Pole Clear Chance") < 500 then
						plotTypes[i] = PlotTypes.PLOT_OCEAN; -- make it semi random
						-- so no one concludes intelligent design
					end
				end
			end
		end
	end


	-- add pole islands
	for x = 0, maxX - 1 do
		for y = 0, maxY - 1 do
			if y <= polesAddDist or y >= maxY-polesAddDist then
				if Map.Rand(1000, "Pole Island Chance") < polesIslandChance then
					RandomIsland(plotTypes,x,y,maxX,3+Map.Rand(8, "Pole Size"),80);
				end
			end
		end
	end

	local numGrowths = 0;


	-- grow land
	numGrowths = 2;
	for iter = 1, numGrowths do
		print ("Growing Land");
		local toMakeLandIdx = {};
		for x = 0, maxX - 1 do
			for y = 0, maxY - 1 do

				local iCenter = GetI(x,y,maxX);
				if plotTypes[iCenter] == PlotTypes.PLOT_OCEAN then -- if water

					local radius = 1;
					local countAdjacentLand = 0;
					local points = GetIndexesAround(x,y,maxX,maxY,radius);
					for k,index in pairs(points) do
						if plotTypes[index] ~= PlotTypes.PLOT_OCEAN then -- if adjacent land
							countAdjacentLand = countAdjacentLand + 1;
						end
						if setContains(toMakeLandIdx, index) then -- invalid because adjacent tile will change
							countAdjacentLand = 0;
							break;
						end
					end

					local makeLand = false;
					if countAdjacentLand == 1 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 400); end
					--if iter == 1 and countAdjacentLand == 2 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 100); end
					--if countAdjacentLand == 5 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 900); end

					if makeLand then
						addToSet(toMakeLandIdx, iCenter);
						--print("made land");
					end
				end
			end
		end

		for makeLandIdx,v in pairs(toMakeLandIdx) do
			plotTypes[makeLandIdx] = RandomPlot(10,40,10,0); -- make this one land too
		end
	end


	-- fill land
	numGrowths = 3;
	for _ = 1, numGrowths do
		print ("Filling Land");
		for x = 0, maxX - 1 do
			for y = 0, maxY - 1 do

				local iCenter = GetI(x,y,maxX);
				if plotTypes[iCenter] == PlotTypes.PLOT_OCEAN then -- if water

					local radius = 1;
					local countSwitches = 0;
					local wasLastLand = false;
					local bHasAnyAdjacentLand = false;
					local points = GetIndexesOrderedRing(x,y,maxX,maxY,radius);
					local countAdjacentLand = 0;
					for k,index in pairs(points) do
						local bIsLand = (plotTypes[index] ~= PlotTypes.PLOT_OCEAN); -- if adjacent land
						if bIsLand then countAdjacentLand = countAdjacentLand + 1; end

						bHasAnyAdjacentLand = bHasAnyAdjacentLand or bIsLand;
						-- skip the first check since we don't know anything yet
						if (k ~= 1 and (bIsLand ~= wasLastLand)) then countSwitches = countSwitches + 1; end
						wasLastLand = bIsLand;
					end

					local bWouldConnectUnconnectedLand = (countSwitches >= 3);
					local bMakeLand = false;
					if (not bWouldConnectUnconnectedLand) then
						chance = 0;
						if (countAdjacentLand == 5) then chance = 1000; end
						if (countAdjacentLand == 4) then chance = 800; end
						if (countAdjacentLand == 3) then chance = 400; end

						bMakeLand = (Map.Rand(1000, "Grow Land Chance") < chance); 
					end

					if bMakeLand then -- make changes immediately to avoid running over eachother
						--table.insert(toMakeLandIdx, iCenter);
						plotTypes[iCenter] = RandomPlot(20,30,10,0); -- make this one land too
						--print ("filled_");
						--print("made land");
					end
				end
			end
		end

		--for k,makeLandIdx in pairs(toMakeLandIdx) do
			--plotTypes[makeLandIdx] = RandomPlot(10,40,10,0); -- make this one land too
		--end
	end


	-- grow land
	numGrowths = 2;
	for iter = 1, numGrowths do
		print ("Growing Land");
		local toMakeLandIdx = {};
		for x = 0, maxX - 1 do
			for y = 0, maxY - 1 do

				local iCenter = GetI(x,y,maxX);
				if plotTypes[iCenter] == PlotTypes.PLOT_OCEAN then -- if water

					local radius = 1;
					local countAdjacentLand = 0;
					local points = GetIndexesAround(x,y,maxX,maxY,radius);
					for k,index in pairs(points) do
						if plotTypes[index] ~= PlotTypes.PLOT_OCEAN then -- if adjacent land
							countAdjacentLand = countAdjacentLand + 1;
						end
						if setContains(toMakeLandIdx, index) then -- invalid because adjacent tile will change
							countAdjacentLand = 0;
							break;
						end
					end

					local makeLand = false;
					if countAdjacentLand == 1 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 100); end
					--if iter == 1 and countAdjacentLand == 2 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 100); end
					--if countAdjacentLand == 5 then makeLand = (Map.Rand(1000, "Grow Land Chance") < 900); end

					if makeLand then
						addToSet(toMakeLandIdx, iCenter);
						--print("made land");
					end
				end
			end
		end

		for makeLandIdx,v in pairs(toMakeLandIdx) do
			plotTypes[makeLandIdx] = RandomPlot(10,40,10,0); -- make this one land too
		end
	end


	-- remove single islands
	for x = 0, maxX - 1 do
		for y = 0, maxY - 1 do
			local i = GetI(x,y,maxX);
			local islandSize = GenIslandSize(islandSizeMin,islandSizeMax,geometricReduction);
			if plotTypes[i] ~= PlotTypes.PLOT_OCEAN then
				if (IsSingleIsland(plotTypes,x,y,maxX)) then
					RandomIsland(plotTypes,x,y,maxX,islandPoleMinSize+Map.Rand(4, "Pole Size")+math.floor(islandSizeMax/3), 0);
				end
			end
		end
	end


	SetPlotTypes(plotTypes);

	local args = {expansion_diceroll_table = {10, 4, 4}};
	GenerateCoasts(args);
end

function addToSet(set, key)
    set[key] = true
end

function removeFromSet(set, key)
    set[key] = nil
end

function setContains(set, key)
    return set[key] ~= nil
end


function IsSingleIsland(plotTypes,x,y,maxX,numLandTiles)

	local points = GetIndexesOrderedRing(x,y,maxX,maxY,1);

	for k,idx in pairs(points) do
		if plotTypes[idx] ~= PlotTypes.PLOT_OCEAN then
			return false; -- adjacent tile was land, so skip
		end
	end

	print ("Single Island found at" .. x .. "," .. y);
	return true; -- we checked them all
end

------------------------------------------------------------------------------
-- True if there is any land in this area
------------------------------------------------------------------------------
function IsNearLand(plotTypes,x,y,maxX,numLandTiles)

	local start = GetI(x,y,maxX);

	local radius = 3 + math.floor(math.sqrt(numLandTiles) + 0.5);
	local points = GetGridPoints(radius);

	for i=1,radius*radius do
		local p = points[i];
    	local xOffA,yOffA = p[1]-1, p[2]-1;
		local index = GetI(x+xOffA,y+yOffA,maxX);

		-- don't replace an existing non ocean tile
		if plotTypes[index] ~= PlotTypes.PLOT_OCEAN then
			return true;
		end
	end
	return false;
end

------------------------------------------------------------------------------
-- creates a random island starting with x,y and going around that point 
-- bouncing positive and negative until numLandTiles is reached
-- maxX needs to be the width of the map
-- plotTypes needs to be the linear array of tile types
------------------------------------------------------------------------------
function RandomIsland(plotTypes,x,y,maxX,numLandTiles,oceanChance)
	--print("Gen island of size: " .. numLandTiles);
	local remaining = numLandTiles;
	local start = GetI(x,y,maxX);


	local radius = 1 + math.floor(math.sqrt(numLandTiles) + 0.5); -- get extra points just in case
	local points = GetGridPoints(radius);

	local mountainChance = 17; -- gets reduced if we've made one

	for i=1,radius*radius do
		p = points[i];
		local xOffA,yOffA = p[1], p[2];
		local index = GetI(x+xOffA,y+yOffA,maxX);

		-- don't replace an existing non ocean tile
		if plotTypes[index] == PlotTypes.PLOT_OCEAN then
			if i==1 then oceanChance = 0; end
			plotTypes[index] = RandomPlot(10,40,mountainChance,oceanChance);

			-- reduce odds of mountain if we just made one
			if plotTypes[index] == PlotTypes.PLOT_MOUNTAIN then
				mountainChance = math.floor(mountainChance / 2);
			end
		end
		-- reduce count if we added/already have a land tile here
		if plotTypes[index] ~= PlotTypes.PLOT_OCEAN then
			remaining = remaining - 1;
		end
		-- we are done making the island
		if remaining <= 0 then
			break;
		end
	end
	
	--print("Remaining: ".. remaining);
	--return;
	--[-[
	-- we still need to place tiles!
	if (remaining > 0) then
		for i=1,radius*radius do -- recheck the points
	    	p = points[i];
	    	local xOffA, yOffA = p[1], p[2];
			local index = GetI(x+xOffA,y+yOffA,maxX);

			-- replace an existing ocean tile
			if plotTypes[index] == PlotTypes.PLOT_OCEAN then
				plotTypes[index] = RandomPlot(30,40,0,0);
				remaining = remaining - 1;
			end

			if (not (remaining > 0)) then
				break;
			end
		end
	end
	--]-]

end

-- starts with Northeast and goes clockwise
function GetIndexesOrderedRing(x,y,maxX,maxY)
	local pts = GetIndexesAround(x,y,maxX,maxY,1); 

	local reorder = {};
	table.insert(reorder, pts[1]);
	table.insert(reorder, pts[4]);
	table.insert(reorder, pts[7]);
	-- skip middle
	table.insert(reorder, pts[6]);
	table.insert(reorder, pts[5]);
	table.insert(reorder, pts[2]);
	return reorder;
end



-- radius 1 is 7 tiles
function GetIndexesAround(x,y,maxX,maxY,radius)
	local xy = {};
	local totalSideLength = radius * 2 + 1;

	local ds = GetGridPoints(totalSideLength);

	local oddR = 0; -- currently odd-r,  www.redblobgames.com/grids/hexagons/#coordinates
	local evenR = 1;

	-- works by using total y coordinate to skip 
	for i = 1, totalSideLength * totalSideLength do
		local px = ds[i][1]; -- for totalSideLength=5 px would be in range [0,4]
		local dx, dy = ds[i][1] - radius, ds[i][2] - radius;
		local nx, ny = x + dx, y + dy;

		
		local xSkipStart = 0;
		local xSkipEnd = 0;
		if y % 2 == oddR then
			xSkipStart = math.floor(math.abs(dy) / 2.0); -- ceiling and floor control the row shift
			xSkipEnd = totalSideLength - math.ceil(math.abs(dy) / 2.0);
		else
			xSkipStart = math.ceil(math.abs(dy) / 2.0); -- ceiling and floor control the row shift
			xSkipEnd = totalSideLength - math.floor(math.abs(dy) / 2.0);
		end

		if xSkipStart <= px and px < xSkipEnd then
			local idx = GetI(nx,ny,maxX);
			table.insert(xy, idx);
		end
	end
	return xy;
end

-- given radius=2 will return
-- 0 0
-- 0 1
-- 1 1
-- 1 0
function GetGridPoints(radius)
	local points = {};
	offX, offY = 0, 0;
	for layer=0,radius-1 do
	    offX = 0;
	    offY = layer;
	    
	    repeat 
	        table.insert(points, {offX, offY});
	        
	        if (offX==layer) then
	            offY = offY - 1;
	        else
	            offX = offX + 1;
	        end
	            
	    until (offX > layer or offY < 0)
	end
	return points;
end

function GenIslandSize(min,max,c)
	return GeometricRand(min, max, c);
end
-------------------------------------------------
-- https://www.wolframalpha.com/input/?i=y%3D0.8%5Ex+from+1+to+10
-------------------------------------------------
function GeometricRand(min,max,c)
	local odds = math.floor(c*1000);

	if (min >= max) then return max; end

	val = min;
	while val<max do
		-- return [0,x-1] -- so a 99% chance should be possible
		if Map.Rand(1000, "Geometric Random") >= odds then
			return val;
		end
		val = val + 1;
	end
	return max; -- never stopped, just return max
end
-------------------------------------------------
-- maps positive integers: 0, 1, 2, 3, 4 etc.
-- to alternating signed:  0,-1, 1,-2, 2 etc.
-------------------------------------------------
function Switch(offset)
	if (offset % 2 == 0) then -- is even number
		return offset/2;
	else                      -- is odd number
		return (1+offset)/-2
	end
end
------------------------------------------------------------------------------
-- randomly generates a plot type weighted by (l)and, (h)ills, (m)ountain, (o)cean
------------------------------------------------------------------------------
function RandomPlot(l,h,m,o)
	local rand = Map.Rand(l+h+m+o, "Random Plot");
	if rand < l then                -- first part of probability distribution
		return PlotTypes.PLOT_LAND
	elseif rand < l+h then          -- second part
		return PlotTypes.PLOT_HILLS
	elseif rand < l+h+m then
		return PlotTypes.PLOT_MOUNTAIN
	else
		return PlotTypes.PLOT_OCEAN
	end
end
------------------------------------------------------------------------------
-- converts an x,y coordinate into an linear index
------------------------------------------------------------------------------
function GetI(x,y,maxX)
	return y * maxX + x + 1;
end
------------------------------------------------------------------------------
function GenerateTerrain()

	local DesertPercent = 28;

	-- Get Temperature setting input by user.
	local temp = Map.GetCustomOption(2)
	if temp == 4 then
		temp = 1 + Map.Rand(3, "Random Temperature - Lua");
	end

	local grassMoist = Map.GetCustomOption(8);

	local args = {
			temperature = temp,
			iDesertPercent = DesertPercent,
			iGrassMoist = grassMoist,
			};

	local terraingen = TerrainGenerator.Create(args);

	terrainTypes = terraingen:GenerateTerrain();
	
	SetTerrainTypes(terrainTypes);


end

------------------------------------------------------------------------------
function AddFeatures()

	-- Get Rainfall setting input by user.
	local rain = Map.GetCustomOption(3)
	if rain == 4 then
		rain = 1 + Map.Rand(3, "Random Rainfall - Lua");
	end
	
	local args = {rainfall = rain}
	local featuregen = FeatureGenerator.Create(args);

	-- passing false removes mountains from coastlines
	featuregen:AddFeatures(false);
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
function StartPlotSystem()

	local RegionalMethod = 2;

	-- Get Resources setting input by user.
	local res = Map.GetCustomOption(13)
	local starts = Map.GetCustomOption(5)
	--if starts == 7 then
		--starts = 1 + Map.Rand(8, "Random Resources Option - Lua");
	--end

	-- Handle coastal spawns and start bias
	MixedBias = false;
	BalancedCoastal = false;
	OnlyCoastal = false;
	CoastLux = false;

	--print("Creating start plot database.");
	local start_plot_database = AssignStartingPlots.Create()
	
	--print("Dividing the map in to Regions.");
	-- Regional Division Method 1: Biggest Landmass
	local args = {
		method = RegionalMethod,
		start_locations = starts,
		resources = res,
		CoastLux = CoastLux,
		NoCoastInland = OnlyCoastal,
		BalancedCoastal = BalancedCoastal,
		MixedBias = MixedBias;
		};
	start_plot_database:GenerateRegions(args)

	--print("Choosing start locations for civilizations.");
	start_plot_database:ChooseLocations()
	
	--print("Normalizing start locations and assigning them to Players.");
	start_plot_database:BalanceAndAssign(args)

	--print("Placing Natural Wonders.");
	local wonders = Map.GetCustomOption(7)
	if wonders == 14 then
		wonders = Map.Rand(13, "Number of Wonders To Spawn - Lua");
	else
		wonders = wonders - 1;
	end

	--print("########## Wonders ##########");
	--print("Natural Wonders To Place: ", wonders);

	local wonderargs = {
		wonderamt = wonders,
	};
	start_plot_database:PlaceNaturalWonders(wonderargs);
	--print("Placing Resources and City States.");
	start_plot_database:PlaceResourcesAndCityStates()

	-- tell the AI that we should treat this as a naval expansion map
	Map.ChangeAIMapHint(4);

end
------------------------------------------------------------------------------