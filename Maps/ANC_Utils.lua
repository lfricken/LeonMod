
include("HBMapmakerUtilities");



------------------------------------------------------------------------------
-- Writes the plot TYPES to C++.
------------------------------------------------------------------------------
function ANC_SetPlotTypes(plotTypes)
	-- NOTE: Plots() is indexed from 0, the way the plots are indexed in C++
	-- However, Lua tables are indexed from 1, and all incoming plot tables should be indexed this way.
	-- So we add 1 to the Plots() index to find the matching plot data in plotTypes.
	for i, plot in Plots() do
		plot:SetPlotType(plotTypes[i + 1], false, false);
	end
end

local _plots = {}; --memoize table of plots
function Plots(sort)
	local _indices = {};
	for i = 1, Map.GetNumPlots(), 1 do
		_indices[i] = i - 1;
	end	
	
	if(sort) then
		sort(_indices);
	end
	
	local cur = 0;
	local it = function()
		cur = cur + 1;
		local index = _indices[cur];
		local plot;
		
		if(index) then
			plot = _plots[index] or Map.GetPlotByIndex(index);
			_plots[index] = plot;
		end
		return index, plot;
	end
	
	return it;
end

function addToSet(set, key)
	set[key] = true;
end

function removeFromSet(set, key)
	set[key] = nil;
end

function setContains(set, key)
	return set[key] ~= nil;
end
function table.contains(table, element)
	for _, value in pairs(table) do
		if value == element then
			return true;
		end
	end
	return false;
end
function removeTableElement(t, element)
	local i = 1;
	while i <= #t do
		if t[i] == element then
			table.remove(t, i);
		else
			i = i + 1;
		end
	end
	return false;
end
function concatTable(t1,t2)
    for i=1, #t2 do
        t1[#t1+1] = t2[i];
    end
    return t1;
end
function wouldConnectLandmasses(set, key)
	-- TODO
end

function length(T)
	local count = 0
	for _ in pairs(T) do count = count + 1 end
	return count
end
------------------------------------------------------------------------------
-- returns numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds
------------------------------------------------------------------------------
function ANC_GetPlayerAndTeamInfo()
	-- This function obtains Civ count, CS count, correct player IDs, and basic Team information.
	local numMajorCivs, numMinors = 0, 0;
	local majorIds, minorIds = {}, {};
	local majorTeamIds = {};

	for i = 0, GameDefines.MAX_MAJOR_CIVS - 1 do
		local player = Players[i];
		if player:IsEverAlive() then
			numMajorCivs = numMajorCivs + 1;
			table.insert(majorIds, i);
		end
	end
	for i = GameDefines.MAX_MAJOR_CIVS, GameDefines.MAX_CIV_PLAYERS - 1 do
		local player = Players[i];
		if player:IsEverAlive() then
			numMinors = numMinors + 1;
			table.insert(minorIds, i);
		end
	end

	local bTeamGame = false;
	local iNumTeams = Game.CountCivTeamsEverAlive()
	local iNumTeamsOfCivs = iNumTeams - numMinors;
	if iNumTeamsOfCivs < numMajorCivs then
		bTeamGame = true;
	end

	local numCivsPerTeam = table.fill(0, GameDefines.MAX_CIV_PLAYERS)
	numCivsPerTeam[0] = 0;
	for i = 0, GameDefines.MAX_MAJOR_CIVS - 1 do
		local player = Players[i];
		if player:IsEverAlive() then
			local teamID = player:GetTeam()
			numCivsPerTeam[teamID] = numCivsPerTeam[teamID] + 1;
			local bCheckTeamList = TestMembership(majorTeamIds, teamID)
			if bCheckTeamList == false then
				table.insert(majorTeamIds, teamID)
			end
		end
	end
	
	return numMajorCivs, majorIds, numMinors, minorIds, bTeamGame, numCivsPerTeam, majorTeamIds
end
------------------------------------------------------------------------------
-- Returns a copy that has been randomly shuffled
------------------------------------------------------------------------------
function CopyAndShuffle(incoming_table)
	-- Designed to operate on tables with no gaps. Does not affect original table.
	local len = table.maxn(incoming_table);
	local copy = {};
	local shuffledVersion = {};
	-- Make copy of table.
	for loop = 1, len do
		copy[loop] = incoming_table[loop];
	end
	-- One at a time, choose a random index from Copy to insert in to final table, then remove it from the copy.
	local left_to_do = table.maxn(copy);
	for loop = 1, len do
		local random_index = 0;
		random_index = 1 + Map.Rand(left_to_do, "CopyAndShuffle - Lua");
		random_index = 1 + Map.Rand(left_to_do, "CopyAndShuffle - Lua");
		random_index = 1 + Map.Rand(left_to_do, "CopyAndShuffle - Lua");
		random_index = 1 + Map.Rand(left_to_do, "CopyAndShuffle - Lua");
		table.insert(shuffledVersion, copy[random_index]);
		table.remove(copy, random_index);
		left_to_do = left_to_do - 1;
	end
	return shuffledVersion;
end
------------------------------------------------------------------------------
-- Outputs the maps land/water in a lua readable format.
------------------------------------------------------------------------------
function printMapToLua(plotTypes, maxX, maxY)
	-- debug print map result
	for y = 0, maxY - 1 do
		local str = "";
		if (y % 2 == 0) then str = " "; end -- odd r
		for x = 0, maxX - 1 do
			local i = GetI(x, maxY - y - 1, maxX);
			if plotTypes[i] ~= PlotTypes.PLOT_OCEAN then
				str = str .. "OO";
			else
				str = str .. "  ";
			end
		end
		print(str);
	end
end
------------------------------------------------------------------------------
-- true if the given tile is a single land tile in the ocean
------------------------------------------------------------------------------
function IsSingleIsland(plotTypes, x, y, maxX, maxY)
	local i = GetI(x,y,maxX);
	if (plotTypes[i] == PlotTypes.PLOT_OCEAN) then -- not land
		return false;
	end

	local points = GetIndexesAround(x,y,maxX,maxY,1);
	for k,idx in pairs(points) do
		if (plotTypes[idx] ~= PlotTypes.PLOT_OCEAN) then
			return false; -- adjacent tile was land, so skip
		end
	end

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

	local mountainChance = 18; -- gets reduced if we've made one

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

------------------------------------------------------------------------------
-- converts an x,y coordinate into an linear index
------------------------------------------------------------------------------
function GetI(x,y,maxX)
	return y * maxX + x + 1;
end

local NumDirections = 6;
local firstRingYIsEvenTABLE = {{0, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
local firstRingYIsOddTABLE = {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, 0}, {0, 1}};
function RandDir()
	local maxExclusive = NumDirections;
	return Map.Rand(maxExclusive, "RandDir");
end
function AddDir(dir, deltaDir)
	return (dir + deltaDir) % NumDirections;
end
------------------------------------------------------------------------------
-- Given an x,y and direction, returns the {x,y} of the next tile
------------------------------------------------------------------------------
function dir(x, y, dir0Idx)
	dir0Idx = dir0Idx % NumDirections; -- only 6 directions!
	local plot_adjustments;
	if y / 2 > math.floor(y / 2) then
		plot_adjustments = firstRingYIsOddTABLE[dir0Idx + 1]; -- +1 because lua tables are 1 indexed
	else
		plot_adjustments = firstRingYIsEvenTABLE[dir0Idx + 1];
	end
	local nextX = x + plot_adjustments[1];
	local nextY = y + plot_adjustments[2];
	return {nextX, nextY}
end
------------------------------------------------------------------------------
-- Skips xy and gives a line of points in the given direction
------------------------------------------------------------------------------
function GoLine(xy, dir, maxX, maxY, radius)
	local indexes = {};
	for span=1,radius do
		xy = GoDir(xy, dir);
		safeInsertIdx(indexes, xy, maxX, maxY);
	end
	return indexes;
end
------------------------------------------------------------------------------
-- Given an {xy} and direction, returns the {xy} of the next tile
------------------------------------------------------------------------------
function GoDir(xy, dir0Idx)
	dir0Idx = dir0Idx % NumDirections; -- only 6 directions!
	local plot_adjustments;
	if xy[2] / 2 > math.floor(xy[2] / 2) then
		plot_adjustments = firstRingYIsOddTABLE[dir0Idx + 1]; -- +1 because lua tables are 1 indexed
	else
		plot_adjustments = firstRingYIsEvenTABLE[dir0Idx + 1];
	end
	local nextX = xy[1] + plot_adjustments[1];
	local nextY = xy[2] + plot_adjustments[2];
	return {nextX, nextY}
end
------------------------------------------------------------------------------
-- Creates a rhombus with the wide angle point at the start.
------------------------------------------------------------------------------
function GetCone2(xStart, yStart, maxX, maxY, dirLeft, radius)
	local current = {xStart, yStart};
	local currentSpanXy = {xStart, yStart};
	local indexes = {};
	local dirSpan = AddDir(dirLeft, -2);
	for left=1,radius do -- go left dir

		for span=1,radius do -- go right dir
			safeInsertIdx(indexes, currentSpanXy, maxX, maxY);
			currentSpanXy = GoDir(currentSpanXy, dirSpan);
		end

		current = GoDir(current, dirLeft);
		currentSpanXy = current;
	end

	return indexes;
end
------------------------------------------------------------------------------
-- Does not insert into the table if the xy is out of bounds, corrects x coordinate, changes to index
------------------------------------------------------------------------------
function safeInsertIdx(values, xy, maxX, maxY)
	if (xy[2] >= 0 and xy[2] < maxY) then -- verify y bounds
		local x = xy[1] % maxX; -- fix x bounds
		local y = xy[2];
		table.insert(values, GetI(x, y, maxX));
	end
end
------------------------------------------------------------------------------
-- HEAVILY TESTED. radius 1 is 6 tiles surrounding x,y, radius 2 is the 12 tiles outside that
------------------------------------------------------------------------------
function GetXyAround(xStart, yStart, radius)
	local xys = {};
	local pos = {xStart, yStart};
	if (radius <= 0) then -- the start plot only is radius 0
		table.insert(xys, pos);
		return xys;
	end

	local edgeLen = radius;
	local perimeterLen = NumDirections * radius;
	if (perimeterLen == 0) then perimeterLen = 1; end


	-- move to edge of ring
	local direction = 1
	for i = 1, radius do
		pos = dir(pos[1], pos[2], direction);
	end
	direction = direction + 1; -- 1/6rd turn, but we will start the loop at an edge!
	-- loop around the ring
	for i = 0, (perimeterLen - 1) do -- start at 0 so we start at an edge turn
		table.insert(xys, pos);

		-- need to change direction, as we have reached end of this edge
		if (i % edgeLen == 0) then
			direction = direction + 1; -- 1/6 turn
		end
		pos = dir(pos[1], pos[2], direction);
	end

	return xys;
end
------------------------------------------------------------------------------
-- converts the output of getxyaround to indexes, does not include values outside maxXY bounds, auto wraps maxX coordinates
------------------------------------------------------------------------------
function GetIndexesAround(xCenter, yCenter, maxX, maxY, radiusMin, radiusMaxOptional)
	if radiusMaxOptional == nil then radiusMaxOptional = radiusMin; end -- default value
	local indexes = {};
	for r = radiusMin, radiusMaxOptional do -- each radius
		local xys = GetXyAround(xCenter, yCenter, r); -- get all points
		for k, xy in pairs(xys) do
			safeInsertIdx(indexes, xy, maxX, maxY);
		end
	end
	return indexes;
end
------------------------------------------------------------------------------
-- randomly shuffles output of GetIndexesAround
------------------------------------------------------------------------------
function GetIndexesAroundRand(x,y,maxX,maxY, radiusMin, radiusMaxOptional)
	return GetShuffledCopyOfTable(GetIndexesAround(x,y,maxX,maxY, radiusMin, radiusMaxOptional));
end


-- https://en.wikipedia.org/wiki/Halton_sequence
-- generate 600 halton points:
-- self.haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
-- self.haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));
-- local x,y = math.floor(iW * self.haltonPointsX[i]), math.floor(iH * self.haltonPointsY[i]);
function halton(base, numToGen, rngAddition)
	points = {};
	local numerator, divisor = 0, 1;
	local minToGen = 100 + rngAddition;

	for i = 0,minToGen+numToGen do
		local range = divisor - numerator;
		if range == 1 then -- find new divisor because we are the end of this range
			numerator = 1;
			divisor = divisor * base;
		else
			local shift = divisor / base;
			while (range <= shift) do
				shift = shift / base;
			end
			numerator = ((base + 1) * shift) - range;
		end
		if (i > minToGen) then
			table.insert(points, numerator / divisor);
		end
	end
	return points;
end
------------------------------------------------------------------------------
-- given radius=2 will return
-- 0 0
-- 0 1
-- 1 1
-- 1 0
------------------------------------------------------------------------------
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
------------------------------------------------------------------------------
-- Randomly switches the fromType to the toType with various rules
------------------------------------------------------------------------------
function Mutate(plotTypes, maxX, maxY, 
	idxsToMutate, minAdj, fromType,
	 toType, mutateChance1000, conjoinChance1000)

	local toMutate = {}; -- avoid growing an island multiple times in the same direction
	for x = 0, maxX - 1 do
		for y = 0, maxY - 1 do

			local plotIdx = GetI(x,y,maxX);
			if table.contains(idxsToMutate, plotIdx) then -- only mutate target indexes
				if plotTypes[plotIdx] == fromType then -- if from type
					local countAdjacentLand = 0;
					local countSwitches = 0;
					local wasLastLand = false;
					local points = GetIndexesAround(x,y,maxX,maxY,1);
					for k,index in pairs(points) do
						if table.contains(toMutate, index) then -- invalid because adjacent tile will change
							countAdjacentLand = 0;
							break;
						end
						local bIsFromType = plotTypes[index] ~= fromType; -- if adjacent land
						if bIsFromType then countAdjacentLand = countAdjacentLand + 1; end

						-- skip the first check since we don't know anything yet
						if (k ~= 1 and (bIsFromType ~= wasLastLand)) then 
							countSwitches = countSwitches + 1; 
						end
						wasLastLand = bIsFromType;
					end
					if (countAdjacentLand >= minAdj) then
						local bWouldConnectUnconnectedLand = (countSwitches >= 3);
						local canConnect = true;
						if (bWouldConnectUnconnectedLand) then
							canConnect = Map.Rand(1000,"PlotConjoin") < conjoinChance1000;
						end

						local doMutate = canConnect and (Map.Rand(1000, "Mutate") < mutateChance1000);
						if doMutate then
							table.insert(toMutate, plotIdx);
						end
					end
				end
			end
		end
	end

	for k,mutateIdx in pairs(toMutate) do
		plotTypes[mutateIdx] = toType; -- make this one the target type
	end
end
------------------------------------------------------------------------------
-- Randomly decides how big an island should be in land tiles
------------------------------------------------------------------------------
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
	else					  -- is odd number
		return (1+offset)/-2
	end
end
------------------------------------------------------------------------------
-- randomly generates a plot type weighted by (l)and, (h)ills, (m)ountain, (o)cean
------------------------------------------------------------------------------
function RandomPlot(l,h,m,o)
	local rand = Map.Rand(l+h+m+o, "Random Plot");
	if rand < l then				-- first part of probability distribution
		return PlotTypes.PLOT_LAND
	elseif rand < l+h then		  -- second part
		return PlotTypes.PLOT_HILLS
	elseif rand < l+h+m then
		return PlotTypes.PLOT_MOUNTAIN
	else
		return PlotTypes.PLOT_OCEAN
	end
end

