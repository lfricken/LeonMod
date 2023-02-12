

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId, isMinor)

	local randDir = RandDir();
	-- do terrain
	--local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
	local indexes = GetCone2(x, y, maxX, maxY, randDir, 3);
	for k,index in pairs(indexes) do
		this.plotTypes[index] = PlotTypes.PLOT_HILLS;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_GRASS;
	end

	-- move a random direction, and carve out a cone to keep as water
	local adjacentWater = dir(x,y,randDir);


	local L1, L2, L3 = {}, {}, {};




	-- spawn lock
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, this.cfg.spawnRangeMin);
	for k,index in pairs(indexes) do
		this.plotIsSpawnLocked[index] = true;
	end
end




function ANC_AddSpawnThingy()


end


















