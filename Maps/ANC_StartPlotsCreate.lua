

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId, isMinor)

	local spawnHexRadius = 4;
	if (isMinor) then spawnHexRadius = 2; end -- we don't want minors taking up tons of space

	local randDir = RandDir();
	-- do terrain
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius);
	for k,index in pairs(indexes) do
		this.plotTypes[index] = PlotTypes.PLOT_HILLS;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_GRASS;
	end

	-- move a random direction, and carve out a cone to keep as water
	local adjacentWater = dir(x,y,randDir);
	local waterCone = GetCone2(adjacentWater[1], adjacentWater[2], maxX, maxY, AddDir(randDir, 1), spawnHexRadius);
	for k,index in pairs(waterCone) do
		this.plotTypes[index] = PlotTypes.PLOT_OCEAN;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_OCEAN;
	end


	local L1, L2, L3 = {}, {}, {};




	-- spawn lock all nearby tiles
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, this.cfg.spawnRangeMin);
	for k,index in pairs(indexes) do
		this.plotIsSpawnLocked[index] = true;
	end
end




function ANC_AddSpawnThingy()


end


















