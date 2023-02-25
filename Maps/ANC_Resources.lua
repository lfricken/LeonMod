
include("ANC_Utils");

local waterLuxFraction = 1.0;
local waterBonusFraction = 0.7;
local waterStratFraction = 1.0;

local resVariation = 2;
local base = 4;
function calcRes(self)
	local id = self[1];
	if id == 0 then
		return base + Map.Rand(resVariation + 1, "iron");
	elseif id == 1 then
		return base + Map.Rand(resVariation + 1, "horses");
	elseif id == 2 then
		return base + Map.Rand(resVariation + 1, "coal");
	elseif id == 3 then
		return base + Map.Rand(resVariation + 1, "oil");
	elseif id == 4 then
		return base + Map.Rand(resVariation + 1, "aluminum");
	elseif id == 5 then
		return base + Map.Rand(resVariation + 1, "uranium");
	else
		return base + Map.Rand(resVariation + 1, "UNKNOWN RESOURCE");
	end
end
------------------------------------------------------------------------------
-- Populates the world with resources and features.
------------------------------------------------------------------------------
function ANC_DoSetupGoodies(this)
	recordResourceInfo(this);
	createResourceClumps(this);
end
------------------------------------------------------------------------------
-- populates the world with stuff that could be considered bonuses, such as:
-- Bonus/Strategic Resources, Luxuries, Atolls, Lakes, Oasis, 
-- Requires ANC_DoResourcesAndFeatures to have been called
------------------------------------------------------------------------------
function ANC_DoPopulateWorldWithGoodies(this)
	local isArableWater = function(idx) return this.plotTerrain[idx] == TerrainTypes.TERRAIN_COAST; end
	local isNotWater = function(idx) return this.plotTypes[idx] ~= PlotTypes.PLOT_OCEAN; end
	local isArableLand = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_LAND or this.plotTypes[idx] == PlotTypes.PLOT_HILLS; end

	-- halton for minor civs
	local haltonPointsX = halton(2, 2048, 0);
	local haltonPointsY = halton(3, 2048, 0);
	local scaleX = function(x) return math.floor(x * 180); end
	local scaleY = function(y) return math.floor(y * 180); end

	-- luxuries
	local count = 0;
	for i=1,#haltonPointsX do
		local xy = {scaleX(haltonPointsX[i]), scaleY(haltonPointsY[i])};
		local inBounds = xy[1] >= 0 and xy[1] < this.maxX and xy[2] >= 0 and xy[2] < this.maxY;
		if inBounds then
			local plotIdx = GetI(xy[1], xy[2], this.maxX);
			local hasFeature = this.plotFeature[plotIdx] ~= FeatureTypes.NO_FEATURE;
			local hasRes = this.plotResourceNum[plotIdx] > 0;
			local hasAdjacentLuxes = ANC_countAdjacents(this, this.plotHasLux, xy, true) > 0;
			local isNotLocked = not this.plotIsLocked[plotIdx];
			if isNotLocked and not hasRes and not hasFeature and not hasAdjacentLuxes then
				if isArableLand(plotIdx) then
					local idx = 1 + (i % #this.luxPolar);
					--print("resource" .. idx);
					local layer = 1;
					if Map.Rand(100,"Res layer 2?") < 50 then layer = 2; end
					local resourceInfo = findClosestClumpedResource(this, layer, xy); -- this.luxPolar[idx];
					this:applyData(plotIdx, resourceInfo);
					count = count + 1;
					this.plotHasLux[plotIdx] = true;

				elseif isArableWater(plotIdx) then
					if (Map.Rand(1000,"Skip Water Lux") < (1000 * waterLuxFraction)) then
						local idx = 1 + Map.Rand(#this.luxWater,"Rand Water Lux"); -- 1 + ((i % 17) % #this.luxWater);
						--print("resource2" .. idx);
						local resourceInfo = this.luxWater[idx];
						this:applyData(plotIdx, resourceInfo);
						count = count + 1;
						this.plotHasLux[plotIdx] = true;
					end
				end
			end
		end
	end
	print("Total Luxes: " .. count);


	local randOffsetX = Map.Rand(4, "res offset1");
	local randOffsetY = Map.Rand(4, "res offset2");
	local haltonPointsX = halton(2, 2048, 0);
	local haltonPointsY = halton(3, 2048, 0);
	local scaleX = function(x) return math.floor(x * 140) - randOffsetX; end
	local scaleY = function(y) return math.floor(y * 140) - randOffsetY; end
	-- bonuses
	local count = 0;
	for i=1,#haltonPointsX do
		local xy = {scaleY(haltonPointsY[i]), scaleX(haltonPointsX[i])};
		local inBounds = xy[1] >= 0 and xy[1] < this.maxX and xy[2] >= 0 and xy[2] < this.maxY;
		if inBounds then
			local plotIdx = GetI(xy[1], xy[2], this.maxX);
			local hasFeature = this.plotFeature[plotIdx] ~= FeatureTypes.NO_FEATURE;
			local hasRes = this.plotResourceNum[plotIdx] > 0;
			--local hasAdjacentLuxes = ANC_countAdjacents(this, this.plotHasLux, xy, true) > 0;
			local isNotLocked = not this.plotIsLocked[plotIdx];
			if isNotLocked and not hasRes and not hasFeature then
				local xyScaled = GetXyScaled(xy, this.maxX, this.maxY);
				if isArableLand(plotIdx) then
					local bonuses = this.bonusTropical;
					if isLat(xyScaled[2], 0.25) then bonuses = this.bonusPolar; end
					local idx = 1 + Map.Rand(#bonuses,"Rand Land Bonus");
					local resourceInfo = bonuses[idx];
					this:applyData(plotIdx, resourceInfo);
					count = count + 1;

				elseif isArableWater(plotIdx) then
					if (Map.Rand(1000,"Skip Water Bonus") < (1000 * waterBonusFraction)) then
						--print("water bonus");
						local idx = 1 + Map.Rand(#this.bonusWater,"Rand Water Bonus");
						local resourceInfo = this.bonusWater[idx];
						this:applyData(plotIdx, resourceInfo);
						count = count + 1;
					end
				end
			end
		end
	end
	print("Total Bonus: " .. count);


	local randOffsetX = Map.Rand(4, "res offset3");
	local randOffsetY = Map.Rand(4, "res offset4");
	local haltonPointsX = halton(17, 4096, Map.Rand(60, "RandStratHaltonStartX"));
	local haltonPointsY = halton(19, 4096, Map.Rand(60, "RandStratHaltonStartY"));
	local scaleX = function(x) return math.floor(x * 170) - randOffsetX; end
	local scaleY = function(y) return math.floor(y * 170) - randOffsetY; end
	local supportsWater = function(resInfo) return this.oil_ID[1] == resInfo[1]; end
	-- bonuses
	local count = 0;
	for resIdx=1,#this.strats do
		local resourceInfo = this.strats[resIdx];
		local scale = 1.0 / (resourceInfo[6] or 1.0);

		for i=1,#haltonPointsX do
			if ((resIdx + i) % #this.strats) == 0 then
				local xy = {scale * scaleY(haltonPointsY[i]), scale * scaleX(haltonPointsX[i])};
				local inBounds = xy[1] >= 0 and xy[1] < this.maxX and xy[2] >= 0 and xy[2] < this.maxY;
				if inBounds then
					local plotIdx = GetI(xy[1], xy[2], this.maxX);
					local hasFeature = this.plotFeature[plotIdx] ~= FeatureTypes.NO_FEATURE;
					local hasRes = this.plotResourceNum[plotIdx] > 0;
					local isNotLocked = not this.plotIsLocked[plotIdx];
					if isNotLocked and not hasRes and not hasFeature then
						local xyScaled = GetXyScaled(xy, this.maxX, this.maxY);
						if isArableLand(plotIdx) then
							this:applyData(plotIdx, resourceInfo);
							count = count + 1;

						elseif isArableWater(plotIdx) and supportsWater(resourceInfo) then
							if (Map.Rand(1000,"Skip Water Strat") < (1000 * waterStratFraction)) then
								this:applyData(plotIdx, resourceInfo);
								count = count + 1;
							end
						end
					end
				end
			end
		end
	end
	print("Total Strat: " .. count);

end
------------------------------------------------------------------------------
-- Generates all Resource Clumps. See createResourceClump and findClosestClumpedResource
------------------------------------------------------------------------------
function createResourceClumps(this)
	local mapXScaling = math.cos(math.rad(30)); -- hexagon packing means the X shift for each point is at a 30 degree angle
	--print("COSINE" .. mapXScaling);
	--print("Original Lux Diameter: " .. this.cfg.luxClumpDiameter);

	-- redo the clump sizes so they fit more evenly
	local xShiftDistance = this.cfg.luxClumpDiameter * mapXScaling;

	-- how many clumps can we fit on the x dimension?
	local numClumpsX = math.floor(0.5 + (this.maxX / xShiftDistance)); -- 60 / 11 > 5.45 > 5
	local eachXShift = this.maxX / numClumpsX;

	this.cfg.luxClumpDiameter = eachXShift / mapXScaling; -- 60 / 5 > 12 -- divide my map scaling since this was scaled by it initially
	--print("New Lux Diameter: " .. this.cfg.luxClumpDiameter);

	-- how many clumps can we fit on the y dimension?
	local ensureTopOfMapHasClumpAlongEdge = 2; -- ensures the very top of the map gets a clump, just like the very bottom
	local numClumpsY = math.floor(this.maxY / this.cfg.luxClumpDiameter);
	local eachYShift = (this.maxY - ensureTopOfMapHasClumpAlongEdge) / numClumpsY;

	-- we want to make sure to TRY to generate enough clumps so we extend past the map a bit
	-- to avoid a situation where the edge of the map is missing a clump 1 would probably be enough, but why not 2?
	local numClumpsSafety = 2;
	-- resIdx ensures that we don't accidentally use the same resource right next to ourselves
	local resIdx = {Map.Rand(30,"Rand Lux Start Tropical"), Map.Rand(40,"Rand Lux Start Polar")};
	for layer=1,2 do
		for x=1,numClumpsX+numClumpsSafety do
			local isEvenX = x % 2 == 0; -- every other column should be shifted up
			for y=1,numClumpsY+numClumpsSafety do
				local realX = (x * eachXShift) - (eachXShift / 2);
				local realY = (y * eachYShift) - (eachYShift / 2);

				-- the second layer should be offset so the centers are at the triple junction of 3 hexes
				if layer == 2 then realX = realX - (eachXShift / 2) * mapXScaling; realY = realY - eachYShift / 2; end
				if isEvenX then realY = realY - (eachYShift / 2); end -- shift down so they follow a centered hex pattern
				tryCreateResourceClump(resIdx, this, layer, realX, realY);
			end
		end
	end
end
------------------------------------------------------------------------------
-- Does not create a clump if the clump would be out of bounds.
-- Creates a resource clump entry, which is just an XY coordinate and a resource.
-- Used later by tiles to determine which kind of resource it should have 
------------------------------------------------------------------------------
function tryCreateResourceClump(resIdx, this, layer, realX, realY)
	-- check bounds
	realX = math.floor(realX);
	realY = math.floor(realY);
	if realX < 0 or realX >= this.maxX or realY < 0 or realY >= this.maxY then 
		--print("skipped: " .. realX .. "," .. realY); 
		return;
	end

	local resource = 1;
	local xyScaled = GetXyScaled({realX, realY}, this.maxX, this.maxY);
	if (isLat(xyScaled[2], 0.25)) then -- polar?
		local num = #this.luxPolar;
		--local randIdx = 1 + Map.Rand(num,"Rand Polar Lux");
		local resourceIndex = (resIdx[1] % num) + 1;
		resource = this.luxPolar[resourceIndex];
		resIdx[1] = resIdx[1] + 1;
	else
		local num = #this.luxTropical;
		--local randIdx = 1 + Map.Rand(num,"Rand Tropical Lux");
		local resourceIndex = (resIdx[2] % num) + 1;
		resource = this.luxTropical[resourceIndex];
		resIdx[2] = resIdx[2] + 1;
	end

	-- debugging
	--if layer == 1 then
	--	this:SetXY(realX, realY, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_GRASS);
	--else
	--	this:SetXY(realX, realY, PlotTypes.PLOT_MOUNTAIN, TerrainTypes.TERRAIN_MOUNTAIN);
	--end
	--print("Clump: " .. realX .. "," .. realY);
	local clump = {
		x = realX,
		y = realY,
		clumpRes = resource,
		distFrom = function(self, x1,y1, shift, maxX,maxY)
			local x = ((self.x + shift) % maxX) - ((x1 + shift) % maxX);
			local y = self.y - y1;
			return math.sqrt(x*x + y*y);
		end,
	};

	if (this.resourceClumps[layer] == nil) then this.resourceClumps[layer] = {}; end -- populate layer with empty array
	local layerClumps = this.resourceClumps[layer];
	table.insert(layerClumps, clump);
end
------------------------------------------------------------------------------
-- given an XY and a layer, gives the resource value for this layer
------------------------------------------------------------------------------
function findClosestClumpedResource(this, layer, xy)
	local extraSafety = 2; -- we want to be sure we shift all the way past the x wrap of the map
	local shiftDistance = this.cfg.luxClumpDiameter;
	local layerClumps = this.resourceClumps[layer];
	local closestDist = 999999;
	local closestClump = {};

	-- try shifting to handle edge of map X wrap
	-- the idea is, we'll shift the x coordinates back and forth over the wrap edge so we can correctly calculate the
	-- distance as if it was adjacent. we don't need to do a shift of 0 because... why would we?
	for shift = -1, 1, 2 do
		for k,clump in pairs(layerClumps) do
			local dist = clump:distFrom(xy[1],xy[2], (extraSafety * shift * shiftDistance), this.maxX, this.maxY);
			if (dist < closestDist) then
				closestDist = dist;
				closestClump = clump;
			end
		end
	end
	--print("closest: " .. closestDist);
	return closestClump.clumpRes;
end
------------------------------------------------------------------------------
-- populate all resource info so we can reference it later
------------------------------------------------------------------------------
function recordResourceInfo(this)

	this.getRandLux = function(self)
		local sum = #self.luxPolar + #self.luxTropical + #self.luxWater;
		local rand = 1 + Map.Rand(sum, "rand lux");
		local data;
		if (rand <= #self.luxPolar) then
			data = self.luxPolar[rand];
		elseif (rand <= #self.luxPolar + #self.luxTropical) then
			data = self.luxTropical[rand - #self.luxPolar];
		else
			data = self.luxWater[rand - #self.luxPolar - #self.luxTropical];
		end
		return data;
	end
	this.getRandBonus = function(self)
		local sum = #self.bonusPolar + #self.bonusTropical + #self.bonusWater;
		local rand = 1 + Map.Rand(sum, "rand lux");
		local data;
		if (rand <= #self.bonusPolar) then
			data = self.bonusPolar[rand];
		elseif (rand <= #self.bonusPolar + #self.bonusTropical) then
			data = self.bonusTropical[rand - #self.bonusPolar];
		else
			data = self.bonusWater[rand - #self.bonusPolar - #self.bonusTropical];
		end
		return data;
	end
	this.applyData = function(self, plotIdx, val)
		self.plotIsLocked[plotIdx] = true;
		if val[2] ~= nil then self.plotTypes[plotIdx] = val[2]; end
		if val[3] ~= nil then self.plotTerrain[plotIdx] = val[3]; end
		if val[4] ~= nil then self.plotFeature[plotIdx] = val[4]; end

		if val[1] ~= nil then
			self.plotResource[plotIdx] = val[1];
			local calcNumRes = val[5];
			if calcNumRes ~= nil then
				self.plotResourceNum[plotIdx] = calcNumRes(val);
			else
				self.plotResourceNum[plotIdx] = 1;
			end
		else
			print("applyData had nil val: " .. val[1]);
		end
	end


	this.strats = {};

	this.bonusPolar = {};
	this.bonusTropical = {};

	this.bonusWater = {};

	this.luxPolar = {};
	this.luxTropical = {};

	this.luxWater = {};


	-- atoll types
	table.insert(this.bonusWater, {-1, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, 17});
	table.insert(this.bonusWater, {-1, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, 26});
	table.insert(this.bonusWater, {-1, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, 27});
	table.insert(this.bonusWater, {-1, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, 28});
	table.insert(this.bonusWater, {-1, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, 29});


	for resource_data in GameInfo.Resources() do

		local rid = resource_data.ID;
		local rName = resource_data.Type;
		--print("Loaded Resource: " .. rName .. " : " .. rid);


		-- Set up Strategic IDs
		if rName == "RESOURCE_IRON" then
			this.iron_ID = {rid, PlotTypes.PLOT_HILLS, nil, nil, calcRes};
			table.insert(this.strats, this.iron_ID);
		elseif rName == "RESOURCE_HORSE" then
			this.horse_ID = {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE, calcRes};
			table.insert(this.strats, this.horse_ID);

		elseif rName == "RESOURCE_COAL" then
			this.coal_ID = {rid, PlotTypes.PLOT_HILLS, nil, nil, calcRes};
			table.insert(this.strats, this.coal_ID);
		elseif rName == "RESOURCE_OIL" then
			this.oil_ID = {rid, PlotTypes.PLOT_LAND, nil, nil, calcRes};
			table.insert(this.strats, this.oil_ID);

		elseif rName == "RESOURCE_ALUMINUM" then
			this.aluminum_ID = {rid, PlotTypes.PLOT_HILLS, nil, nil, calcRes};
			table.insert(this.strats, this.aluminum_ID);
		elseif rName == "RESOURCE_URANIUM" then
			this.uranium_ID = {rid, PlotTypes.PLOT_HILLS, nil, nil, calcRes, 1.0};
			table.insert(this.strats, this.uranium_ID);


		-- Set up Bonus IDs
		elseif rName == "RESOURCE_STONE" then
			this.stone_ID = rid;
			table.insert(this.bonusPolar, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_HARDWOOD" then	-- MOD.HungryForFood: New
			this.hardwood_ID = rid;
			table.insert(this.bonusPolar, {rid, nil, nil, FeatureTypes.FEATURE_FOREST});
		elseif rName == "RESOURCE_DEER" then
			this.deer_ID = rid;
			table.insert(this.bonusPolar, {rid, nil, nil, FeatureTypes.FEATURE_FOREST});
		elseif rName == "RESOURCE_BISON" then
			this.bison_ID = rid;
			table.insert(this.bonusPolar, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});

		elseif rName == "RESOURCE_FISH" then
			this.fish_ID = rid;
			table.insert(this.bonusWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, FeatureTypes.NO_FEATURE});
			table.insert(this.bonusWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, FeatureTypes.NO_FEATURE});
			table.insert(this.bonusWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, FeatureTypes.NO_FEATURE});
			-- double amount of fish

		elseif rName == "RESOURCE_SHEEP" then
			this.sheep_ID = rid;
			table.insert(this.bonusTropical, {rid, PlotTypes.PLOT_HILLS, TerrainTypes.TERRAIN_GRASS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_WHEAT" then
			this.wheat_ID = rid;
			table.insert(this.bonusTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_COW" then -- cattle
			this.cow_ID = rid;
			table.insert(this.bonusTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_MAIZE" then	-- MOD.HungryForFood: New
			this.maize_ID = rid;
			table.insert(this.bonusTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_BANANA" then
			this.banana_ID = rid;
			table.insert(this.bonusTropical, {rid, nil, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.FEATURE_JUNGLE});



		elseif rName == "RESOURCE_WHALE" then
			this.whale_ID = rid;
			table.insert(this.luxWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, nil});
		elseif rName == "RESOURCE_CRAB" then
			this.crab_ID = rid;
			table.insert(this.luxWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, nil});
		elseif rName == "RESOURCE_CORAL" then
			this.coral_ID = rid;
			table.insert(this.luxWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, nil});
		elseif rName == "RESOURCE_PEARLS" then
			this.pearls_ID = rid;
			table.insert(this.luxWater, {rid, PlotTypes.PLOT_OCEAN, TerrainTypes.TERRAIN_COAST, nil});

		-- Set up Luxury IDs
		elseif rName == "RESOURCE_FUR" then
			this.fur_ID = rid;
			table.insert(this.luxPolar, {rid, nil, nil, FeatureTypes.FEATURE_FOREST});
		elseif rName == "RESOURCE_LAPIS" then	-- MOD.Barathor: New
			this.lapis_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_GOLD" then
			this.gold_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_SILVER" then
			this.silver_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_GEMS" then
			this.gems_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_AMBER" then	-- MOD.Barathor: New
			this.amber_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, FeatureTypes.FEATURE_FOREST});
		elseif rName == "RESOURCE_JADE" then		-- MOD.Barathor: New
			this.jade_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, FeatureTypes.FEATURE_FOREST});
		elseif rName == "RESOURCE_COPPER" then
			this.copper_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_SALT" then
			this.salt_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_HILLS, nil, nil});
		elseif rName == "RESOURCE_MARBLE" then
			this.marble_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_OBSIDIAN" then	-- MOD.HungryForFood: New
			this.obsidian_ID = rid;
			table.insert(this.luxPolar, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});


		elseif rName == "RESOURCE_IVORY" then
			this.ivory_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_SILK" then
			this.silk_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_DYE" then
			this.dye_ID = rid;
			table.insert(this.luxTropical, {rid, nil, nil, nil});
		elseif rName == "RESOURCE_SPICES" then
			this.spices_ID = rid;
			table.insert(this.luxTropical, {rid, nil, nil, nil});
		elseif rName == "RESOURCE_SUGAR" then
			this.sugar_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, nil});
		elseif rName == "RESOURCE_COTTON" then
			this.cotton_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, nil});
		elseif rName == "RESOURCE_WINE" then
			this.wine_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, nil});
		elseif rName == "RESOURCE_INCENSE" then
			this.incense_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_CITRUS" then
			this.citrus_ID = rid;
			table.insert(this.luxTropical, {rid, nil, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.FEATURE_JUNGLE});
		elseif rName == "RESOURCE_TRUFFLES" then
			this.truffles_ID = rid;
			table.insert(this.luxTropical, {rid, nil, TerrainTypes.TERRAIN_PLAINS, nil});
		elseif rName == "RESOURCE_COCOA" then
			this.cocoa_ID = rid;
			table.insert(this.luxTropical, {rid, nil, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.FEATURE_JUNGLE});
		elseif rName == "RESOURCE_COFFEE" then	-- MOD.Barathor: New
			this.coffee_ID = rid;
			table.insert(this.luxTropical, {rid, nil, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_TEA" then		-- MOD.Barathor: New
			this.tea_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, TerrainTypes.TERRAIN_PLAINS, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_TOBACCO" then	-- MOD.Barathor: New
			this.tobacco_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_OLIVE" then	-- MOD.Barathor: New
			this.olives_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, nil, nil});
		elseif rName == "RESOURCE_PERFUME" then	-- MOD.Barathor: New
			this.perfume_ID = rid;
			table.insert(this.luxTropical, {rid, PlotTypes.PLOT_LAND, nil, FeatureTypes.NO_FEATURE});
		elseif rName == "RESOURCE_COCONUT" then	-- MOD.HungryForFood: New
			this.coconut_ID = rid;
			table.insert(this.luxTropical, {rid, nil, nil, FeatureTypes.FEATURE_JUNGLE});
		elseif rName == "RESOURCE_RUBBER" then	-- MOD.HungryForFood: New
			this.rubber_ID = rid;
			table.insert(this.luxTropical, {rid, nil, nil, FeatureTypes.FEATURE_JUNGLE});




		--[[ NOT USED
		elseif rName == "RESOURCE_PLATINUM" then	-- MOD.HungryForFood: New
			this.platinum_ID = rid;
		elseif rName == "RESOURCE_POPPY" then	-- MOD.HungryForFood: New
			this.poppy_ID = rid;
		elseif rName == "RESOURCE_TIN" then		-- MOD.HungryForFood: New
			this.tin_ID = rid;
		elseif rName == "RESOURCE_LAVENDER" then	-- MOD.HungryForFood: New
			this.lavender_ID = rid;

		-- Even More Resources for Vox Populi (bonus)
		elseif rName == "RESOURCE_LEAD" then		-- MOD.HungryForFood: New
			this.lead_ID = rid;
		elseif rName == "RESOURCE_PINEAPPLE" then	-- MOD.HungryForFood: New
			this.pineapple_ID = rid;
		elseif rName == "RESOURCE_POTATO" then	-- MOD.HungryForFood: New
			this.potato_ID = rid;
		elseif rName == "RESOURCE_RICE" then	-- MOD.HungryForFood: New
			this.rice_ID = rid;
		elseif rName == "RESOURCE_SULFUR" then	-- MOD.HungryForFood: New
			this.sulfur_ID = rid;
		elseif rName == "RESOURCE_TITANIUM" then	-- MOD.HungryForFood: New
			this.titanium_ID = rid;]]
		end
	end
end

