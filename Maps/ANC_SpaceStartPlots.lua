

include("HBMapmakerUtilities");
include("ANC_Utils");
include("ANC_StartPlotsCreate");


local g_polarSpawnAvoidance = 8;


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId)

	local waterLockSize = 3;
	local spawnHexRadius = 3;

	local randWaterDir = RandDir();

	-- setup spawn land vs water
	local spawnIndexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius + 2);
	for k,plotIdx in pairs(spawnIndexes) do
		this.plotTypes[plotIdx] = PlotTypes.PLOT_LAND;
	end

	-- move a random direction, and carve out a cone to keep as water
	local adjacentWater = dir(x,y,randWaterDir);
	local waterCone = GetCone2(adjacentWater[1], adjacentWater[2], maxX, maxY, AddDir(randWaterDir, 1), spawnHexRadius + 1);
	for k,plotIdx in pairs(waterCone) do
		this.plotTypes[plotIdx] = PlotTypes.PLOT_OCEAN;
	end
	-- select a line of water tiles that leads away from the spawn to guarantee these stay as water
	local waterSafeLineIdxs = GoLine({x, y}, randWaterDir, maxX, maxY, spawnHexRadius + 1);

	-- mutate water adjacent terrain
	-- but avoid the line of water leading out of this hex area
	local toMutate = CopyAndShuffle(waterCone);
	for i,idx in pairs(waterSafeLineIdxs) do
		removeTableElement(toMutate, idx);
	end
	-- but include the adjacent terrain
	concatTable(toMutate, GoLine({x, y}, AddDir(randWaterDir, 1), maxX, maxY, spawnHexRadius));
	concatTable(toMutate, GoLine({x, y}, AddDir(randWaterDir, -1), maxX, maxY, spawnHexRadius));
	for i=1,2 do -- JUST 2 attempts, with more, the spawn might not have enough water
		local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
		local chance = 200;
		if (i%2==0) then local temp = from; from = to; to = temp; chance = 1000 - chance; end
		Mutate(this.plotTypes, this, from, to, toMutate);
	end




	local indexes;

	-- set terrain types
	local lockSize = spawnHexRadius;
	indexes = GetIndexesAround(x, y, maxX, maxY, 0, lockSize);
	for k,plotIdx in pairs(indexes) do
		if this.plotTypes[plotIdx] == PlotTypes.PLOT_LAND then
			this.plotTerrain[plotIdx] = TerrainTypes.TERRAIN_GRASS;
		else
			this.plotTerrain[plotIdx] = TerrainTypes.TERRAIN_OCEAN;
		end
	end

	-- rand luxuries
	local lux1 = this:getRandLux();
	local lux2;
	for i=1,10 do
		lux2 = this:getRandLux();
		if (lux2[1] ~= lux1[1]) then break; end
	end

	-- things to add
	local toAdd1 = {};
	local toAdd2 = {};
	local toAdd3 = {};
	table.insert(toAdd1, lux2);
	table.insert(toAdd1, this:getRandBonus());
	table.insert(toAdd1, this.iron_ID);

	table.insert(toAdd2, lux1);

	table.insert(toAdd2, lux2);
	table.insert(toAdd2, this:getRandBonus());
	table.insert(toAdd2, this.horse_ID);
	table.insert(toAdd2, this.coal_ID);
	table.insert(toAdd2, this.oil_ID);

	table.insert(toAdd3, lux2);
	table.insert(toAdd3, this:getRandBonus());
	table.insert(toAdd3, this:getRandBonus());
	table.insert(toAdd3, this.aluminum_ID);
	table.insert(toAdd3, this.uranium_ID);


	local toAddAll = {};
	table.insert(toAddAll, {toAdd1, GetIndexesAroundRand(x, y, maxX, maxY, 1)});
	table.insert(toAddAll, {toAdd2, GetIndexesAroundRand(x, y, maxX, maxY, 2)});
	table.insert(toAddAll, {toAdd3, GetIndexesAroundRand(x, y, maxX, maxY, 3)}); -- up to cityWorkRadius

	
	for k0,addInfo in pairs(toAddAll) do
		local toAdd = addInfo[1];
		local indexes = addInfo[2];
		for k1,resInfo in pairs(toAdd) do
			local didPlace = false;
			for k2,plotIdx in pairs(indexes) do
				local isWaterRes = resInfo[2] == PlotTypes.PLOT_OCEAN;
				if not this.plotIsLocked[plotIdx] then
					if isWaterRes and this.plotTypes[plotIdx] == PlotTypes.PLOT_OCEAN then
						--print("lands " .. plotIdx .. " " .. resInfo[1] or "." .. " " .. resInfo[4] or ".")
						this:applyData(plotIdx, resInfo);
						didPlace = true;
						break;
					elseif not isWaterRes and this.plotTypes[plotIdx] ~= PlotTypes.PLOT_OCEAN then
						--print("water " .. plotIdx .. " " .. resInfo[1] or "." .. " " .. resInfo[4] or ".")
						this:applyData(plotIdx, resInfo);
						didPlace = true;
						break;
					end
				end
			end
			if not didPlace then
				-- bump this to a later ring
				local nextRing = k0 + 1;
				if (nextRing < 4) then
				table.insert(toAddAll[nextRing], resInfo);
				print("WARNING Spawn Placement Failed: " .. (resInfo[1] or ".") .. " " .. (resInfo[2] or ".") .. " " .. (resInfo[3] or ".") .. " " ..(resInfo[4] or "."));
				else
					print("WARNING FAILURE Spawn Placement: " .. (resInfo[1] or ".") .. " " .. (resInfo[2] or ".") .. " " .. (resInfo[3] or ".") .. " " ..(resInfo[4] or "."));
				end
			end
		end
	end

	local cityWorkRadius = 3;
	local numHills = 0;
	local numFlat = 0;
	indexes = GetIndexesAround(x, y, maxX, maxY, 0, cityWorkRadius);
	for k,plotIdx in pairs(indexes) do
		if this.plotTypes[plotIdx] == PlotTypes.PLOT_HILLS then
			numHills = numHills + 1;
		elseif this.plotTypes[plotIdx] == PlotTypes.PLOT_LAND then
			numFlat = numFlat + 1;
		end
	end

	local numHillsNeeded = math.floor((numFlat - numHills) / 3); -- divide by 3 means it will be close to 50 50, 2 would be perfect
	indexes = GetIndexesAroundRand(x, y, maxX, maxY, 0, cityWorkRadius);
	for k,plotIdx in pairs(indexes) do
		if not this.plotIsLocked[plotIdx] then
			if numHillsNeeded > 0 then
				this.plotTypes[plotIdx] = PlotTypes.PLOT_HILLS;
				numHillsNeeded = numHillsNeeded - 1;
			elseif numHillsNeeded < 0 then
				this.plotTypes[plotIdx] = PlotTypes.PLOT_LAND;
				numHillsNeeded = numHillsNeeded + 1;
			end -- or if 0, do nothing!
		end
	end



	-- lock nearby water, prevents land growth from putting the spawn point in a tiny inlet
	local waterLock = adjacentWater;
	for i=1,4 do
		waterLock = dir(waterLock[1], waterLock[2], randWaterDir);
	end
	indexes = GetIndexesAround(waterLock[1],waterLock[2], maxX, maxY, 0, waterLockSize);
	for k,plotIdx in pairs(indexes) do
		this.plotIsLocked[plotIdx] = true;
		-- it's important to set a valid terrain type or future sets get glitched
		if this.plotTypes[plotIdx] == PlotTypes.PLOT_OCEAN then
			this.plotTerrain[plotIdx] = TerrainTypes.TERRAIN_OCEAN;
		end
	end

	-- lock them all
	indexes = GetIndexesAround(x, y, maxX, maxY, 0, lockSize);
	for k,plotIdx in pairs(indexes) do
		this.plotIsLocked[plotIdx] = true;
	end


	flagAsSpawnPoint(this, {x, y});
end
------------------------------------------------------------------------------
-- Mark the nearby spawn points so we dont spawn too close
------------------------------------------------------------------------------
function flagAsSpawnPoint(this, xy)
	-- finally, spawn lock tiles to avoid people spawning too close
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	local indexes = GetIndexesAround(xy[1], xy[2], this.maxX, this.maxY, 0, this.cfg.spawnRangeMin);
	for k,plotIdx in pairs(indexes) do
		this.plotIsWithinSpawnDist[plotIdx] = true;
	end
end
------------------------------------------------------------------------------
-- Place minor civs
------------------------------------------------------------------------------
function ANC_DoMinorSpawns(this)
	local numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds = ANC_GetPlayerAndTeamInfo();
	-- halton for minor civs
	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));


	-- minor civs
	local yRange = this.maxY - 2 * g_polarSpawnAvoidance;
	local scaleY = function(y)
		return math.floor(y * yRange + g_polarSpawnAvoidance);
	end
	local scaleX = function(x)
		return math.floor(x * this.maxX);
	end
	for idx,pid in ipairs(minorIds) do
		local player = Players[pid];
		local x,y = scaleX(haltonPointsX[pid]), scaleY(haltonPointsY[pid]);
		local xy = findCsSpawnNear(this, x, y);
		local start_plot = Map.GetPlot(xy[1], xy[2]);
		player:SetStartingPlot(start_plot);
		flagAsSpawnPoint(this, xy);
	end

end
------------------------------------------------------------------------------
-- Determines where players spawn
------------------------------------------------------------------------------
function ANC_SetupStartPlots(this)
	print("ANC_SpaceStartPlots Begin");

	local maxX, maxY = Map.GetGridSize();

	-- Determine number of civilizations and city states present in this game.
	local numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds = ANC_GetPlayerAndTeamInfo();


	-- halton for minor civs
	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));

	-- predetermined for major civs
	local spawnXy, variation, _ = ANC_getMajorCivSpawnPoints(this.cfg, numMajorCivs, maxX, maxY);
	if (this.cfg.shufflePositions) then
		spawnXy = CopyAndShuffle(spawnXy);
	end


	-- temp assign starts so the game doesn't crash later
	-- if these aren't assigned, C++ crashes
	Map.RecalculateAreas();
	for idx,pid in ipairs(majorIds) do
		local temp_start_plot = Map.GetPlot(pid % 17, pid % 19);
		local player = Players[pid];
		player:SetStartingPlot(temp_start_plot);
	end
	for idx,pid in ipairs(minorIds) do
		local temp_start_plot = Map.GetPlot(pid % 17, pid % 19);
		local player = Players[pid];
		player:SetStartingPlot(temp_start_plot);
	end


	-- major civs
	for idx,pid in ipairs(majorIds) do
		local player = Players[pid];
		local x,y = spawnXy[idx][1], spawnXy[idx][2];
		--print("Start Major: " .. x .. ", " .. y);
		local start_plot = Map.GetPlot(x, y);
		player:SetStartingPlot(start_plot);

		ANC_DoSpawnFor(this, x, y, maxX, maxY, pid);
	end

	--ANC_SetPlotTypes(this.plotTypes);
	print("ANC_SpaceStartPlots End");
end
-------------------------------------------------
-- Finds a valid spawn location for 
-------------------------------------------------
function findCsSpawnNear(this, x, y)
	for r = 0, 40 do
		local points = GetIndexesAroundRand(x, y, this.maxX, this.maxY, r);
		for k,index in pairs(points) do
			local xy = GetXy(index, this.maxX);
			if isValidForCs(this, xy) then
				return xy;
			end
		end
	end
	print("ERROR NO CS SPAWN POINT FOUND");
	return nil;
end
-------------------------------------------------
-- True if this xy is a valid spawn location for a city state
-------------------------------------------------
function isValidForCs(this, xy)
	-- bounds
	if not (xy[1] > 1 and xy[1] < this.maxX-1 and xy[2] > 1 and xy[2] < this.maxY-1) then return false; end

	local plotIdx = GetI(xy[1], xy[2], this.maxX);
	local isLand = this.plotTypes[plotIdx] == PlotTypes.PLOT_LAND or this.plotTypes[plotIdx] == PlotTypes.PLOT_HILLS;
	local hasFeatures = this.plotFeature[plotIdx] ~= FeatureTypes.NO_FEATURE;
	-- require land
	if not isLand or hasFeatures then return false; end
	-- spawn distances
	if this.plotIsWithinSpawnDist[plotIdx] then return false; end


	local isSurroundedByLand = true;
	local adjacents = GetIndexesAround(xy[1], xy[2], this.maxX, this.maxY, 1);
	for k,adjIdx in pairs(adjacents) do
		if this.plotTypes[adjIdx] == PlotTypes.PLOT_OCEAN then
			isSurroundedByLand = false;
			break;
		end
	end
	-- require water adjacent
	if isSurroundedByLand then return false; end

	return true;
end
------------------------------------------------------------------------------
-- Determines the XY size of the map
------------------------------------------------------------------------------
function ANC_calcMapSizeXy(numMajorCivs,numMinorCivs,ratio,tilesPerMajorCiv)
	-- ratio 4, 2
	local targetNumTiles = numMajorCivs * tilesPerMajorCiv;
	local A = ratio[1] / ratio[2]; -- A = x / y ; 2
	--A * x * x = targetNum
	--x * x = targetNum / A
	local base = math.floor(math.sqrt(targetNumTiles / A)); -- y = sqrt(targetNum / A)
	local y = math.floor(0.5 + base);
	local x = math.floor(0.5 + base * A);

	return {x, y};
end
------------------------------------------------------------------------------
-- returns
-- xy coordinates of every major civ player
------------------------------------------------------------------------------
function ANC_getMajorCivSpawnPoints(cfg, numMajorCivs, maxX, maxY)
	local spawnFloat = {}; -- floating points
	if numMajorCivs == 1 then
		spawnFloat,maxVariation,shapeRatio = getFor1();
	end
	if numMajorCivs == 2 then
		spawnFloat,maxVariation,shapeRatio = getFor2();
	end
	if numMajorCivs == 3 then
		spawnFloat,maxVariation,shapeRatio = getFor3();
	end
	if numMajorCivs == 4 then
		spawnFloat,maxVariation,shapeRatio = getFor4();
	end
	if numMajorCivs == 5 then
		spawnFloat,maxVariation,shapeRatio = getFor5();
	end
	if numMajorCivs == 6 then
		spawnFloat,maxVariation,shapeRatio = getFor6();
	end
	if numMajorCivs == 7 then
		spawnFloat,maxVariation,shapeRatio = getFor7();
	end
	if numMajorCivs == 8 then
		spawnFloat,maxVariation,shapeRatio = getFor8();
	end
	if numMajorCivs == 9 then
		spawnFloat,maxVariation,shapeRatio = getFor9();
	end
	if numMajorCivs == 10 then
		spawnFloat,maxVariation,shapeRatio = getFor10();
	end
	if numMajorCivs == 11 then
		spawnFloat,maxVariation,shapeRatio = getFor12();
	end
	if numMajorCivs == 12 then
		spawnFloat,maxVariation,shapeRatio = getFor12();
	end
	if numMajorCivs > 12 and numMajorCivs <= 16 then
		spawnFloat,maxVariation,shapeRatio = getFor16();
	end

	-- scale up to world coordinates
	local spawnXy = {};
	for k,xy in ipairs(spawnFloat) do
		local xyVariation = {(Map.Rand(2000, "randSpawnX") / 1000) - 1, (Map.Rand(2000, "randSpawnY") / 1000) - 1};
		xy[1] = xy[1] + cfg.spawnVariation * maxVariation[1] * xyVariation[1];
		xy[2] = xy[2] + cfg.spawnVariation * maxVariation[2] * xyVariation[2];


		local coordinate = {math.floor(maxX * xy[1]), math.floor(maxY * xy[2])};
		table.insert(spawnXy, coordinate);
	end
	return spawnXy,maxVariation,shapeRatio;
end
------------------------------------------------------------------------------
-- 1,1 is far lower right corner, 0,0 is other far left top corner
-- remember that X wraps, which means X distances should be considered accross that boundary
------------------------------------------------------------------------------
function getFor1()
	local f = {};
	table.insert(f, {1/2, 1/2}); -- dead center
	local maxVariation = {2/5, 2/5};
	return f,maxVariation,{1,1};
end
 -- REMEMBER! x wraps, these are equal distance accross X dimension, positive and negative
function getFor2()
	local f = {};
	table.insert(f, {1/4, 1/3}); -- opposite corners
	table.insert(f, {3/4, 2/3});
	local maxVariation = {2/5, 1/4};
	return f,maxVariation,{1,1};
end
function getFor3()
	local f = {};
	table.insert(f, {1/6, 1/2}); -- 3 accross the middle
	table.insert(f, {3/6, 1/2});
	table.insert(f, {5/6, 1/2});
	local maxVariation = {1/7, 1/3};
	return f,maxVariation,{3,2};
end
function getFor4()
	local f = {};
	table.insert(f, {1/8, 1/4}); -- four corners
	table.insert(f, {3/8, 3/4});
	table.insert(f, {5/8, 1/4});
	table.insert(f, {7/8, 3/4});
	local maxVariation = {1/9, 1/5};
	return f,maxVariation,{1,1};
end
function getFor5()
	local f = {};
	table.insert(f, {2/10, 1/4}); -- 4 around
	table.insert(f, {2/10, 3/4});
	table.insert(f, {8/10, 1/4});
	table.insert(f, {8/10, 3/4});

	table.insert(f, {5/10, 2/4}); -- one in middle
	local maxVariation = {2/11, 1/5};
	return f,maxVariation,{4,4};
end
function getFor6()
	local f = {};
	table.insert(f, {1/6, 1/4}); -- 3 top
	table.insert(f, {3/6, 1/4});
	table.insert(f, {5/6, 1/4});
	
	table.insert(f, {1/6, 3/4}); -- 3 bottom
	table.insert(f, {3/6, 3/4});
	table.insert(f, {5/6, 3/4});
	local maxVariation = {1/7, 1/5};
	return f,maxVariation,{4,3};
end
function getFor7()
	local f = {};
	table.insert(f, {1/6, 1/4}); -- 3 top (squished Y)
	table.insert(f, {3/6, 1/4});
	table.insert(f, {5/6, 1/4});
	
	local top = 3/5;
	table.insert(f, {1/8, top}); -- 4 bottom (squished X)
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/6};
	return f,maxVariation,{1,1};
end
function getFor8()
	local f = {};
	local bottom = 1/4;
	table.insert(f, {1/8, bottom}); -- 4 bottom
	table.insert(f, {3/8, bottom});
	table.insert(f, {5/8, bottom});
	table.insert(f, {7/8, bottom});
	
	local top = 3/4;
	table.insert(f, {1/8, top}); -- 4 top
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/5};
	return f,maxVariation,{4,3};
end
function getFor9()
	local f = {};
	local bottom = 1/6;
	table.insert(f, {1/6, bottom}); -- 3 bottom
	table.insert(f, {3/6, bottom});
	table.insert(f, {5/6, bottom});

	local middle = 3/6;
	table.insert(f, {1/6, middle}); -- 3 middle
	table.insert(f, {3/6, middle});
	table.insert(f, {5/6, middle});
	
	local top = 5/6;
	table.insert(f, {1/6, top}); -- 3 top
	table.insert(f, {3/6, top});
	table.insert(f, {5/6, top});
	local maxVariation = {1/7, 1/7};
	return f,maxVariation,{1,1};
end
function getFor10()
	local f = {};

	table.insert(f, {2/20, 1/5}); -- 4 around
	table.insert(f, {2/20, 4/5});
	table.insert(f, {8/20, 1/5});
	table.insert(f, {8/20, 4/5});

	table.insert(f, {5/20, 2/4}); -- one in middle


	table.insert(f, {12/20, 1/5}); -- 4 around
	table.insert(f, {12/20, 4/5});
	table.insert(f, {18/20, 1/5});
	table.insert(f, {18/20, 4/5});

	table.insert(f, {15/20, 2/4}); -- one in middle

	local maxVariation = {2/22, 1/5};
	return f,maxVariation,{3,2};
end
function getFor12()
	local f = {};

	local bottom = 1/6;
	table.insert(f, {1/8, bottom}); -- 4 bottom
	table.insert(f, {3/8, bottom});
	table.insert(f, {5/8, bottom});
	table.insert(f, {7/8, bottom});
	
	local middle = 5/6;
	table.insert(f, {0/8, middle}); -- 4 middle
	table.insert(f, {2/8, middle});
	table.insert(f, {4/8, middle});
	table.insert(f, {6/8, middle});
	local maxVariation = {1/9, 1/5};

	local top = 5/6;
	table.insert(f, {1/8, top}); -- 4 top
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/7};
	return f,maxVariation,{4,3};
end
function getFor16()
	local f = {};
	local div = 8;
	local offset = 1/8;
	local yPos;
	yPos = 1/8;
	table.insert(f, {1/div, yPos}); -- 4 bottom
	table.insert(f, {3/div, yPos});
	table.insert(f, {5/div, yPos});
	table.insert(f, {7/div, yPos});

	yPos = 3/8;
	table.insert(f, {1/div + offset, yPos});
	table.insert(f, {3/div + offset, yPos});
	table.insert(f, {5/div + offset, yPos});
	table.insert(f, {7/div + offset, yPos});

	yPos = 5/8;
	table.insert(f, {1/div, yPos});
	table.insert(f, {3/div, yPos});
	table.insert(f, {5/div, yPos});
	table.insert(f, {7/div, yPos});

	yPos = 7/8;
	table.insert(f, {1/div + offset, yPos});
	table.insert(f, {3/div + offset, yPos});
	table.insert(f, {5/div + offset, yPos});
	table.insert(f, {7/div + offset, yPos});
	
	local maxVariation = {1/(div + 1), 1/7};
	return f,maxVariation,{16,16};
end



