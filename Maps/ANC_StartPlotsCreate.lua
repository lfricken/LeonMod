

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Populates the players spawn points with terrain and goodies, then locks the tiles
------------------------------------------------------------------------------
function ANC_DoSpawnFor(this, x, y, maxX, maxY, playerId, isMinor)

	local spawnHexRadius = 4;
	if (isMinor) then spawnHexRadius = 2; end -- we don't want minors taking up tons of space

	local randWaterDir = RandDir();

	-- setup spawn land vs water
	local spawnIndexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius);
	for k,index in pairs(spawnIndexes) do
		this.plotTypes[index] = PlotTypes.PLOT_LAND;
	end

	-- move a random direction, and carve out a cone to keep as water
	local adjacentWater = dir(x,y,randWaterDir);
	local waterCone = GetCone2(adjacentWater[1], adjacentWater[2], maxX, maxY, AddDir(randWaterDir, 1), spawnHexRadius);
	for k,index in pairs(waterCone) do
		this.plotTypes[index] = PlotTypes.PLOT_OCEAN;
	end
	-- select a line of water tiles that leads away from the spawn to guarantee these stay as water
	local waterSafeLineIdxs = GoLine({x, y}, randWaterDir, maxX, maxY, spawnHexRadius);

	-- mutate water adjacent terrain
	-- but avoid the line of water leading out of this hex area
	local toMutate = CopyAndShuffle(waterCone);
	for i,idx in pairs(waterSafeLineIdxs) do
		removeTableElement(toMutate, idx);
	end
	-- but include the adjacent terrain
	concatTable(toMutate, GoLine({x, y}, AddDir(randWaterDir, 1), maxX, maxY, spawnHexRadius));
	concatTable(toMutate, GoLine({x, y}, AddDir(randWaterDir, -1), maxX, maxY, spawnHexRadius));
	for i=1,7 do -- 7 attempts (random number)
		local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
		local chance = 200;
		if (i%2==0) then local temp = from; from = to; to = temp; chance = 1000 - chance; end
		Mutate(this, from, to, toMutate);
	end





	local indexes;

	-- lock nearby everything
	indexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius);
	for k,index in pairs(indexes) do
		this.plotIsLocked[index] = true;
	end

	-- lock nearby water, prevents land growth from putting the spawn point in a tiny inlet
	local waterLockSize = 4;
	local waterLock = adjacentWater;
	for i=1,4 do
		waterLock = dir(waterLock[1],waterLock[2],randWaterDir);
	end
	indexes = GetIndexesAround(waterLock[1],waterLock[2], maxX, maxY, 0, waterLockSize);
	for k,index in pairs(indexes) do
		this.plotIsLocked[index] = true;
	end


	-- finally, spawn lock tiles to avoid people spawning too close
	if (this.cfg.spawnRangeMin < 3) then print("WARNING: spawnRangeMin was dangerously low!"); end
	indexes = GetIndexesAround(x, y, maxX, maxY, 0, spawnHexRadius);
	for k,index in pairs(indexes) do
		this.plotIsWithinSpawnDist[index] = true;
	end



	-- add goodies to spawn tiles

	local L1, L2, L3 = {}, {}, {};




end















