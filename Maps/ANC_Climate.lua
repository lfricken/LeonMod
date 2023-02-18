

include("ANC_Utils");

function isLat(lat, latMin)
	return lat < latMin or lat > (1 - latMin);
end

function ANC_Climate(this)
	print("ANC_Climate Begin");

	local maxX, maxY = this.maxX, this.maxY;

	local polar = 0.085;
	local tundra = 0.13;
	local temperate = 0.30;
	local tropical = 0.46;
	--local desert = 0.44;

	for i, plot in Plots() do
		local xy = GetXy(i, maxX);
		local xyScale = GetXyScaled(xy, maxX, maxY);
		if this.plotTypes[i] ~= PlotTypes.PLOT_OCEAN and not this.plotIsLocked[i] then
			if Map.Rand(1000, "RandDir") < 380 then
				this.plotTypes[i] = PlotTypes.PLOT_HILLS;
			end

			this.plotTerrain[i] = TerrainTypes.TERRAIN_PLAINS;

			local lat = xyScale[2];
			if isLat(lat, polar) then
				this.plotTerrain[i] = TerrainTypes.TERRAIN_SNOW;


			elseif isLat(lat, tundra) then
				this.plotTerrain[i] = TerrainTypes.TERRAIN_TUNDRA;


			elseif isLat(lat, temperate) then
				if Map.Rand(1000, "RandDir") < 230 then
					this.plotFeature[i] = FeatureTypes.FEATURE_FOREST
				end
				this.plotTerrain[i] = TerrainTypes.TERRAIN_GRASS;


			elseif isLat(lat, tropical) then
				this.plotTerrain[i] = TerrainTypes.TERRAIN_GRASS;
				if Map.Rand(1000, "RandDir") < 300 then
					this.plotFeature[i] = FeatureTypes.FEATURE_JUNGLE
				elseif Map.Rand(1000, "RandDir") < 20 then
					this.plotFeature[i] = FeatureTypes.FEATURE_MARSH
				end


			else
				this.plotTerrain[i] = TerrainTypes.TERRAIN_DESERT;
			end

		else

		end	
	end

	print("ANC_Climate End");
end
