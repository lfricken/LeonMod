

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId, isMinor)

	-- do terrain
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
	for k,index in pairs(indexes) do
		this.plotTypes[index] = PlotTypes.PLOT_HILLS;
		this.plotTerrain[index] = TerrainTypes.TERRAIN_GRASS;
	end

	-- spawn lock
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	local indexes = GetIndexesAround(x, y, maxX, maxY, 0, this.cfg.spawnRangeMin);
	for k,index in pairs(indexes) do
		this.plotIsSpawnLocked[index] = true;
	end
end























