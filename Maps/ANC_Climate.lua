

include("ANC_Utils");

function probLat(lat, latMin)
	local moreLat = math.max( 0, (1 / latMin) * (math.max(lat, 1 - lat) - ( 1 - latMin) ) );
	return moreLat;
end

function ANC_Climate(this)
	print("ANC_Climate Begin");

	local maxX, maxY = this.maxX, this.maxY;

	--local desert = 0.44;


	local isWater = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_OCEAN; end
	local isNotWater = function(idx) return this.plotTypes[idx] ~= PlotTypes.PLOT_OCEAN; end
	local isArableLand = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_LAND or this.plotTypes[idx] == PlotTypes.PLOT_HILLS; end

	-- CLIMATE and HILLS
	for i, plot in ANC_Plots() do
		local xy = GetXy(i, maxX);
		local xyScale = GetXyScaled(xy, maxX, maxY);
		local lat = xyScale[2];
		if not this.plotIsLocked[i] then
			if isArableLand(i) or this.plotTypes[i] == PlotTypes.PLOT_MOUNTAIN then
				if this.plotTypes[i] ~= PlotTypes.PLOT_MOUNTAIN and Map.Rand(1000, "hills") < 380 then
					this.plotTypes[i] = PlotTypes.PLOT_HILLS;
				end

				if isLat(lat, this.polarLat) then
					this.plotTerrain[i] = TerrainTypes.TERRAIN_SNOW;
				elseif isLat(lat, this.tundraLat) then
					this.plotTerrain[i] = TerrainTypes.TERRAIN_TUNDRA;
				elseif isLat(lat, this.temperateLat) then
					this.plotTerrain[i] = TerrainTypes.TERRAIN_GRASS;
				elseif isLat(lat, this.tropicalLat) then
					this.plotTerrain[i] = TerrainTypes.TERRAIN_PLAINS;
				else
					this.plotTerrain[i] = TerrainTypes.TERRAIN_DESERT;
				end


			elseif isWater(i) then -- is water
				--this.plotTerrain[i] = TerrainTypes.TERRAIN_OCEAN;
				local isLandAdjacent = 0 < ANC_countAdjacents(this, this.plotTypes, xy, PlotTypes.PLOT_OCEAN, false);
				if (isLandAdjacent) then this.plotTerrain[i] = TerrainTypes.TERRAIN_COAST;
				else this.plotTerrain[i] = TerrainTypes.TERRAIN_OCEAN; end


			else -- mountain
				--DO NOT USE this.plotTerrain[i] = TerrainTypes.TERRAIN_MOUNTAIN; -- NEVER use terrain_mountain, it causes glitches
			end	
		end	
	end

	-- randomize climate borders
	for i=1,3 do
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_DESERT, nil, ANC_grow(1.0), isArableLand);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_SNOW, nil, ANC_grow(0.5), isArableLand);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_TUNDRA, nil, ANC_grow(0.5), isArableLand);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_PLAINS, nil, ANC_grow(0.5), isArableLand);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_GRASS, nil, ANC_grow(0.5), isArableLand);
	end

	-- grow coast
	for i=1,4 do
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_COAST, nil, ANC_grow(1.0, 100), isWater);
	end

	-- forests, jungle, marsh, oasis, ice
	for i, plot in ANC_Plots() do
		local xy = GetXy(i, maxX);
		local xyScale = GetXyScaled(xy, maxX, maxY);
		local lat = xyScale[2];
		if not this.plotIsLocked[i] then
			if isNotWater(i) then

				local lat = xyScale[2];
				if this.plotTerrain[i] == TerrainTypes.TERRAIN_SNOW then


				elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_TUNDRA then
					if Map.Rand(1000, "tundra forest") < 180 then
						this.plotFeature[i] = FeatureTypes.FEATURE_FOREST;
					end

				elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_GRASS then
					if Map.Rand(1000, "forest") < 180 then
						this.plotFeature[i] = FeatureTypes.FEATURE_FOREST;
					end


				elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_PLAINS then
					if Map.Rand(1000, "jungle") < 250 then
						this.plotFeature[i] = FeatureTypes.FEATURE_JUNGLE;
					elseif Map.Rand(1000, "marsh") < 30 then
						this.plotTypes[i] = PlotTypes.PLOT_LAND;
						this.plotFeature[i] = FeatureTypes.FEATURE_MARSH;
					end


				elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_DESERT then
					if Map.Rand(1000, "oasis") < 50 then
						this.plotTypes[i] = PlotTypes.PLOT_LAND;
						this.plotFeature[i] = FeatureTypes.FEATURE_OASIS;
					end

				end

			else
				if (isLat(lat, this.tundraLat)) then
					local isLandAdjacent = 0 < ANC_countAdjacents(this, this.plotTypes, xy, PlotTypes.PLOT_OCEAN, false);
					local maxProb = 500 * probLat(lat, this.tundraLat);
					if (not isLandAdjacent) then
						if Map.Rand(1000, "ice") < maxProb then
							this.plotFeature[i] = FeatureTypes.FEATURE_ICE;
						end
					end
				end

			end	
		end
	end

	-- throw some grassland/plains together
	for i, plot in ANC_Plots() do
		local switch = Map.Rand(1000, "flip") < 200;
		if not this.plotIsLocked[i] and switch then
			if this.plotTerrain[i] == TerrainTypes.TERRAIN_PLAINS  then
				this.plotTerrain[i] = TerrainTypes.TERRAIN_GRASS;
			elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_GRASS then
				this.plotTerrain[i] = TerrainTypes.TERRAIN_PLAINS;
			end
		end
	end

	print("ANC_Climate End");
end

function ANC_FloodPlains(this)
	for i, plot in ANC_Plots() do
		-- flat desert with no other feature
		local isNoFeature = this.plotFeature[i] == FeatureTypes.NO_FEATURE;
		local hasRes = this.plotResourceNum[i] > 0;
		local isDesert = this.plotTerrain[i] == TerrainTypes.TERRAIN_DESERT;
		if plot:IsRiver() and not hasRes and isNoFeature and isDesert and this.plotTypes[i] == PlotTypes.PLOT_LAND then
			this.plotFeature[i] = FeatureTypes.FEATURE_FLOOD_PLAINS;
		end
	end
end
