

include("ANC_Utils");

function isLat(lat, latMin)
	return lat < latMin or lat > (1 - latMin);
end

function ANC_Climate(this)
	print("ANC_Climate Begin");

	local maxX, maxY = this.maxX, this.maxY;

	--local desert = 0.44;


	local isWater = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_OCEAN; end
	local isNotWater = function(idx) return this.plotTypes[idx] ~= PlotTypes.PLOT_OCEAN; end

	-- CLIMATE and HILLS
	for i, plot in ANC_Plots() do
		local xy = GetXy(i, maxX);
		local xyScale = GetXyScaled(xy, maxX, maxY);
		if not this.plotIsLocked[i] then
			if isNotWater(i) then
				if Map.Rand(1000, "RandDir") < 380 then
					this.plotTypes[i] = PlotTypes.PLOT_HILLS;
				end

				local lat = xyScale[2];
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

			else -- is water
				--this.plotTerrain[i] = TerrainTypes.TERRAIN_OCEAN;
				local isLandAdjacent = 6 > countAdjacent(this, this.plotTypes, xy, PlotTypes.PLOT_OCEAN);
				if (isLandAdjacent) then this.plotTerrain[i] = TerrainTypes.TERRAIN_COAST;
				else this.plotTerrain[i] = TerrainTypes.TERRAIN_OCEAN; end
			end	
		end	
	end

	for i=1,3 do
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_DESERT, nil, ANC_grow(1.0), isWater);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_SNOW, nil, ANC_grow(0.5), isWater);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_TUNDRA, nil, ANC_grow(0.5), isWater);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_PLAINS, nil, ANC_grow(0.5), isWater);
		Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_GRASS, nil, ANC_grow(0.5), isWater);
		--Mutate(this.plotTerrain, this, nil, TerrainTypes.TERRAIN_COAST, nil, ANC_grow(1.0, 0), isNotWater);
	end

	for i, plot in ANC_Plots() do
		local xy = GetXy(i, maxX);
		local xyScale = GetXyScaled(xy, maxX, maxY);
		if not isWater(i) and not this.plotIsLocked[i] then

			local lat = xyScale[2];
			if this.plotTerrain[i] == TerrainTypes.TERRAIN_SNOW then


			elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_TUNDRA then


			elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_GRASS then
				if Map.Rand(1000, "RandDir") < 230 then
					this.plotFeature[i] = FeatureTypes.FEATURE_FOREST
				end


			elseif this.plotTerrain[i] == TerrainTypes.TERRAIN_PLAINS then
				if Map.Rand(1000, "RandDir") < 300 then
					this.plotFeature[i] = FeatureTypes.FEATURE_JUNGLE
				elseif Map.Rand(1000, "RandDir") < 30 then
					this.plotFeature[i] = FeatureTypes.FEATURE_MARSH
				end


			else

			end

		else

		end	
	end




	-- MOUNTAINS

	print("ANC_Climate End");
end
