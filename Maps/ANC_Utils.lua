
include("HBMapmakerUtilities");

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
	set[key] = true
end

function removeFromSet(set, key)
	set[key] = nil
end

function setContains(set, key)
	return set[key] ~= nil
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

local firstRingYIsEvenTABLE = {{0, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
local firstRingYIsOddTABLE = {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, 0}, {0, 1}};
------------------------------------------------------------------------------
-- Given an x,y and direction, returns the {x,y} of the next tile
------------------------------------------------------------------------------
function dir(x, y, dir0Idx)
	dir0Idx = dir0Idx % 6; -- only 6 directions!
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
	local perimeterLen = 6 * radius;
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
	if radiusMaxOptional == nil then radiusMax = radiusMin; end -- default value
	local indexes = {};
	for r = radiusMin, radiusMaxOptional do -- each radius
		local xys = GetXyAround(xCenter, yCenter, r); -- get all points
		for k, xy in pairs(xys) do
			if (xy[2] >= 0 and xy[2] < maxY) then
				local x = xy[1] % maxX;
				local y = xy[2];
				table.insert(indexes, GetI(x, y, maxX));
			end
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

