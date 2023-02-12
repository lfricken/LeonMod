

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId, isMinor)

	local spawnHexRadius = 4;
	if (isMinor) then spawnHexRadius = 2; end -- we don't want minors taking up tons of space

	local randWaterDir = RandDir();
	-- do terrain
	local spawnIndexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius);
	for k,index in pairs(spawnIndexes) do
		this.plotTypes[index] = PlotTypes.PLOT_LAND;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_GRASS;
	end

	-- move a random direction, and carve out a cone to keep as water
	local adjacentWater = dir(x,y,randWaterDir);
	local waterCone = GetCone2(adjacentWater[1], adjacentWater[2], maxX, maxY, AddDir(randWaterDir, 1), spawnHexRadius);
	for k,index in pairs(waterCone) do
		this.plotTypes[index] = PlotTypes.PLOT_OCEAN;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_OCEAN;
	end
	local waterLineIdxs = GoLine({x, y}, randWaterDir, maxX, maxY, spawnHexRadius);


	local toMutate = CopyAndShuffle(waterCone);
	for i,idx in pairs(waterLineIdxs) do
		table.removeElement(toMutate, idx);
	end
	table.concat(toMutate, GoLine({x, y}, AddDir(randWaterDir, 1), maxX, maxY, spawnHexRadius));
	table.concat(toMutate, GoLine({x, y}, AddDir(randWaterDir, -1), maxX, maxY, spawnHexRadius));

	for i=1,7 do
		local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
		if (i%2==0) then local temp = from; from = to; to = temp; end
		Mutate(this.plotTypes, maxX, maxY, 
			toMutate, 1, from,
			to, 500, 0);
	end

	local L1, L2, L3 = {}, {}, {};


	-- spawn lock all nearby tiles
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, this.cfg.spawnRangeMin);
	for k,index in pairs(indexes) do
		this.plotIsWithinSpawnDist[index] = true;
	end
end




function ANC_SetAll(this, index)
	this.plotIsLocked[index] = true;
end


















