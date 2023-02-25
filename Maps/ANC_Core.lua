
-- Core mapgen file that calls into the rest of the ANC code

include("ANC_LandAndSea");
include("ANC_SpaceStartPlots");
include("ANC_Climate");
include("ANC_Resources");
include("ANC_Utils");
include("ANC_Rivers");


ANC = {};
------------------------------------------------------------------------------
-- Arguments for map generator
------------------------------------------------------------------------------
function ANC_CreateArgs()
	local ancArgs = {
		spawnVariation = 0, -- [0,1] how much CAN spawn points for Major Civs be out of expected location?
		spawnRangeMin = 7, -- the minimum tiles between civ spawn points
		shufflePositions = true, -- will player ID's always start in the same spots?

		luxDensity = 1, -- [0,3] multiplier for how dense are luxuries
		stratDensity = 1, -- [0,3] multiplier for how dense are strategic resources
		bonusDensity = 1, -- [0,3] multiplier for how dense are bonus resources

		luxClumpDiameter = 19, -- how large of an area will a particular resource appear in (this is recalculated slightly based on map size)
		bonusClumpDiameter = 13, -- how large of an area will a particular resource appear in
	};
	return ancArgs;
end
------------------------------------------------------------------------------
-- Initialize the map variables
------------------------------------------------------------------------------
function ANC_Constructor(ancArgs)
	local maxX, maxY = Map.GetGridSize();
	local this = {
		Set = ANC.Set,


		resourceClumps = {}, -- given a lux meta index, returns a list of possible luxuries
		bonusClumps = {}, -- given a lux meta index, returns a list of possible bonuses

		plotToLuxMeta = {}, -- given a plot index, indexes into the luxMeta

		plotTypes = {}, -- PlotTypes (PLOT_MOUNTAIN, PLOT_HILLS, PLOT_LAND, PLOT_OCEAN)
		plotTerrain = {}, -- TerrainTypes (TERRAIN_GRASS  _PLAINS _DESERT _TUNDRA _SNOW _COAST _OCEAN _MOUNTAIN _HILL)
		plotFeature = {}, -- FeatureTypes (FEATURE_ICE _JUNGLE _MARSH _OASIS _FLOOD_PLAINS _FOREST _FALLOUT _NATURAL_WONDER)

		plotResource = {}, -- rNames (Luxuries, Strategic, Bonus)		
		plotResourceNum = {}, -- rNames (Luxuries, Strategic, Bonus)
		plotIsLocked = {};
		plotHasLux = {};
		plotIsWithinSpawnDist = {}, -- this plot should not be modified any more

		maxX = maxX,
		maxY = maxY,

		polarLat = 0.085;
		tundraLat = 0.13;
		temperateLat = 0.30;
		tropicalLat = 0.46;

		cfg = ancArgs,
	};
	ANC_SafeInitPlots(this);
	return this;
end
------------------------------------------------------------------------------
-- PlotTypes (PLOT_MOUNTAIN, PLOT_HILLS, PLOT_LAND, PLOT_OCEAN)
-- TerrainTypes (TERRAIN_GRASS  _PLAINS _DESERT _TUNDRA _SNOW _COAST _OCEAN _MOUNTAIN _HILL)
-- 
-- self.featureFloodPlains = FeatureTypes.FEATURE_FLOOD_PLAINS;
-- self.featureIce = FeatureTypes.FEATURE_ICE;
-- self.featureJungle = FeatureTypes.FEATURE_JUNGLE;
-- self.featureForest = FeatureTypes.FEATURE_FOREST;
-- self.featureOasis = FeatureTypes.FEATURE_OASIS;
-- self.featureMarsh = FeatureTypes.FEATURE_MARSH;
-- 
-- self.terrainIce = TerrainTypes.TERRAIN_SNOW;
-- self.terrainTundra = TerrainTypes.TERRAIN_TUNDRA;
-- self.terrainPlains = TerrainTypes.TERRAIN_PLAINS;
------------------------------------------------------------------------------
function ANC:Set(idx, plotType, terrain, feature, resource, numres)

	if (self.plotIsLocked[idx]) then print("WARNING ANC.Set PlotLocked: " .. debug.traceback()); return; end
	if (plotType == nil) then print("WARNING ANC.Set call missing type: " .. debug.traceback()); return; end
	if (terrain == nil) then print("WARNING ANC.Set missing terrain: " .. debug.traceback()); return; end

	feature = feature or FeatureTypes.NO_FEATURE;
	resource = resource or -1;
	numRes = numRes or 0;

	self.plotTypes[idx] = plotType;
	self.plotTerrain[idx] = terrain;
	self.plotFeature[idx] = feature;
	self.plotResource[idx] = resource;
	self.plotResourceNum[idx] = numres;
	self.plotIsLocked[idx] = true;
	-- a future call to ANC_UpdatePlots is expected
	--local plot = Map.GetPlotByIndex(idx);
	--plot:


end
------------------------------------------------------------------------------
-- See ANC:Set
------------------------------------------------------------------------------
function ANC:SetXY(x, y, plotType, terrain, feature, resource, numres)
	self:Set(GetI(x, y, self.maxX), plotType, terrain, feature, resource, numres)
end
------------------------------------------------------------------------------
-- Entry point for creating this map type
------------------------------------------------------------------------------
function ANC_CreateMap(ancArgs)
	print("CreateMap Begin");
	this = ANC_Constructor(ancArgs);
	
	ANC_DoSetupGoodies(this); -- first, because spawn point bonuses check this logic
	
	ANC_SetupStartPlots(this); -- creates all initial spawn locations and properties including terrain, resources

	ANC_LandAndSea(this); -- creates more islands and grows existing ones -- mountains, land, hills, ocean
	
	ANC_Climate(this); -- desert, tundra, snow, forest, jungle, plains, grassland, coast, rivers

	ANC_DoPopulateWorldWithGoodies(this); -- (EXCEPT FOR SPAWN POINTS) add luxuries, bonuses, strategics, features (ice, oasis, atolls)
	
	-- do minor spawn BEFORE adding lakes so they dont' spawn on the lakes
	ANC_DoMinorSpawns(this);

	-- do lakes AFTER putting resources on the map so they don't collide
	for i, plot in ANC_Plots() do
		local xy = GetXy(i, this.maxX);
		local points = GetIndexesAround(xy[1], xy[2], this.maxX, this.maxY, 1);
		local isSurroundedByLand = true;
		for k,adjIdx in pairs(points) do
			if this.plotTypes[adjIdx] == PlotTypes.PLOT_OCEAN then
				isSurroundedByLand = false;
				break;
			end
		end
		if not this.plotIsLocked[i] and isSurroundedByLand and (Map.Rand(1000, "lake") < 30) then
			this.plotTypes[i] = PlotTypes.PLOT_OCEAN;
			this.plotTerrain[i] = TerrainTypes.TERRAIN_COAST;
			this.plotFeature[i] = FeatureTypes.NO_FEATURE;
			this.plotResource[i] = -1;
			this.plotResourceNum[i] = 0;
		end
	end

	ANC_UpdatePlots(this);

	ANC_AddRivers(this);


	--ANC_UpdatePlots(this); -- flood plains need to check for rivers
	ANC_FloodPlains(this);


	ANC_UpdatePlots(this);
	DetermineContinents(); -- Continental artwork selection must wait until Areas are finalized, so it gets handled last.

	Map.DoPlaceNaturalWonders();



	-- IMPROVEMENT_GOODY_HUT goody hut ancient ruin
	local randOffsetX = Map.Rand(4, "res offset3");
	local randOffsetY = Map.Rand(4, "res offset4");
	local haltonPointsX = halton(23, 4096, Map.Rand(60, "RandGoodyHaltonStartX"));
	local haltonPointsY = halton(19, 4096, Map.Rand(60, "RandGoodyHaltonStartY"));
	local scaleX = function(x) return math.floor(x * 170) - randOffsetX; end
	local scaleY = function(y) return math.floor(y * 170) - randOffsetY; end
	local isArableWater = function(idx) return this.plotTerrain[idx] == TerrainTypes.TERRAIN_COAST; end
	local isNotWater = function(idx) return this.plotTypes[idx] ~= PlotTypes.PLOT_OCEAN; end
	local isArableLand = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_LAND or this.plotTypes[idx] == PlotTypes.PLOT_HILLS; end
	-- bonuses
	local count = 0;
	local goodyHutMult = 0.21;
	local goodyHutScalar = math.sqrt(goodyHutMult);

	for i=1,#haltonPointsX do
		local xy = {scaleY(haltonPointsY[i]) / goodyHutScalar, scaleX(haltonPointsX[i]) / goodyHutScalar};
		local inBounds = xy[1] >= 0 and xy[1] < this.maxX and xy[2] >= 0 and xy[2] < this.maxY;
		if inBounds then
			local plotIdx = GetI(xy[1], xy[2], this.maxX);
			local isNotLocked = not this.plotIsLocked[plotIdx];
			if isNotLocked then
				local plot = Map.GetPlot(xy[1], xy[2]);
				if not plot:IsImpassable() and isArableLand(plotIdx) then
					plot:SetImprovementType(2);
					count = count + 1;

				elseif not plot:IsImpassable() and plot:IsAdjacentToLand() then
					if (Map.Rand(1000,"Skip Water Strat") < (200)) then
						plot:SetImprovementType(2);
						count = count + 1;
					end
				end
			end
		end
	end
	print("Total Goody: " .. count);



	print("CreateMap End");
end
------------------------------------------------------------------------------
-- avoid crashes by initializing the map plots to SOMETHING
------------------------------------------------------------------------------
function ANC_SafeInitPlots(this)
	-- PLOT_HILL TERRAIN_GRASS

	local maxX, maxY = Map.GetGridSize();
	this.plotTypes = table.fill(PlotTypes.PLOT_OCEAN, maxX * maxY);
	this.plotTerrain = table.fill(TerrainTypes.TERRAIN_OCEAN, maxX * maxY); -- never use TERRAIN_HILL? never use TERRAIN_MOUNTAIN
	this.plotFeature = table.fill(FeatureTypes.NO_FEATURE, maxX * maxY);

	this.plotResource = table.fill(-1, maxX * maxY);
	this.plotResourceNum = table.fill(0, maxX * maxY);
	this.plotIsLocked = table.fill(false, maxX * maxY);
	this.plotIsWithinSpawnDist = table.fill(false, maxX * maxY);

	this.plotHasLux = table.fill(false, maxX * maxY);
	--this.plotHasBonus = table.fill(false, maxX * maxY);

	--ANC_UpdatePlots(this);
	--[[for i, plot in Plots() do
		-- type
		plot:SetPlotType(this.plotTypes[i + 1], false, false);
		-- terrain
		plot:SetPlotType(PlotTypes.PLOT_LAND, false, true);
		plot:SetTerrainType(TerrainTypes.TERRAIN_GRASS, false, true);
		plot:SetFeatureType(FeatureTypes.NO_FEATURE, -1);
	end]]
end
------------------------------------------------------------------------------
-- Sends ALL plot info to C++. Should only NEED to be called right at the end
------------------------------------------------------------------------------
function ANC_UpdatePlots(this)
	print("ANC_UpdatePlots");
	for i, plot in ANC_Plots() do
		plot:SetPlotType(this.plotTypes[i], false, true);
		plot:SetTerrainType(this.plotTerrain[i], false, true);
		plot:SetFeatureType(this.plotFeature[i], -1);
		plot:SetResourceType(this.plotResource[i], this.plotResourceNum[i]);
	end
	Map.RecalculateAreas();
end

function DetermineContinents()
	print("Determining continents for art purposes (MapGenerator.Lua)");
	-- Each plot has a continent art type. Mixing and matching these could look
	-- extremely bad, but there is nothing technical to prevent it. The worst 
	-- that will happen is that it can't find a blend and draws red checkerboards.
	
	-- Command for setting the art type for a plot is: <plot object>:SetContinentArtType(<art_set_number>)
	
	-- CONTINENTAL ART SETS
	-- 0) Ocean
	-- 1) America
	-- 2) Asia
	-- 3) Africa
	-- 4) Europe
	
	-- Here is an example that sets all land in the world to use the European art set.
	--[[
	for i, plot in Plots() do
		if plot:IsWater() then
			plot:SetContinentArtType(0);
		else
			plot:SetContinentArtType(4);
		end
	end
	]]--
	
	-- Default for this function operates in C++, but you can override by
	-- replacing this method with your own and not calling the default stamper.
	Map.DefaultContinentStamper();
end
