
-- Core mapgen file that calls into the rest of the ANC code

include("ANC_LandAndSea");
include("ANC_SpaceStartPlots");
include("ANC_Climate");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Arguments for map generator
------------------------------------------------------------------------------
function ANC_CreateArgs()
	local ancArgs = {
		spawnVariation = 0, -- [0,1] how much CAN spawn points for Major Civs be out of expected location?
		spawnRangeMin = 5, -- the minimum tiles between civ spawn points
		shufflePositions = true, -- will player ID's always start in the same spots?

		luxDensity = 1, -- [0,3] multiplier for how dense are luxuries
		stratDensity = 1, -- [0,3] multiplier for how dense are strategic resources
		bonusDensity = 1, -- [0,3] multiplier for how dense are bonus resources
	};
	return ancArgs;
end
------------------------------------------------------------------------------
-- Initialize the map variables
------------------------------------------------------------------------------
function ANC_Constructor(ancArgs)
	local this = {
		plotTypes = {}, -- PlotTypes (PLOT_MOUNTAIN, PLOT_HILLS, PLOT_LAND, PLOT_OCEAN)
		plotTerrain = {}, -- TerrainTypes (TERRAIN_GRASS  _PLAINS _DESERT _TUNDRA _SNOW _COAST _OCEAN _MOUNTAIN _HILL)
		plotFeature = {}, -- FeatureTypes (FEATURE_ICE _JUNGLE _MARSH _OASIS _FLOOD_PLAINS _FOREST _FALLOUT _NATURAL_WONDER)

		plotResource = {}, -- ResourceTypes (Luxuries, Strategic, Bonus)		
		plotResourceNum = {}, -- ResourceTypes (Luxuries, Strategic, Bonus)
		plotIsSpawnLocked = {}, -- this plot should not be modified any more

		cfg = ancArgs,
	};
	ANC_SafeInitPlots(this);
	return this;
end
------------------------------------------------------------------------------
-- Entry point for creating this map type
------------------------------------------------------------------------------
function ANC_CreateMap(ancArgs)
	print("CreateMap Begin");
	this = ANC_Constructor(ancArgs);
	
	
	ANC_SpaceStartPlots(this); -- spaces out initial spawn points and places basic initial spawn island and spawn points
	--ANC_LandAndSea(this); -- creates more islands and grows existing ones -- mountains, land, hills, ocean
	
	ANC_Climate(this); -- desert, tundra, snow, forest, jungle, plains, grassland
	
	-- Each body of water, area of mountains, or area of hills+flatlands is independently grouped and tagged.
	Map.RecalculateAreas();
	
	-- River generation is affected by plot types, originating from highlands and preferring to traverse lowlands.
	--AddRivers();
	
	-- Lakes would interfere with rivers, causing them to stop and not reach the ocean, if placed any sooner.
	--AddLakes();
	
	-- Features depend on plot types, terrain types, rivers and lakes to help determine their placement.
	--AddFeatures();

	-- Feature Ice is impassable and therefore requires another area recalculation.
	Map.RecalculateAreas();

	-- Assign Starting Plots, Place Natural Wonders, and Distribute Resources.
	-- This system was designed and programmed for Civ5 by Bob Thomas.
	-- Starting plots are wholly dependent on all the previous elements being in place.
	-- Natural Wonders are dependent on civ starts being in place, to keep them far enough away.
	-- Resources are dependent on start locations, Natural Wonders, as well as plots, terrain, rivers, lakes and features.
	--
	-- This system relies on Area-based data and cannot tolerate an AreaID recalculation during its operations.
	-- Due to plot changes from Natural Wonders and possibly other source, another recalculation is done as the final action of the system.
	--StartPlotSystem();

	-- Goodies depend on not colliding with resources or Natural Wonders, or being placed too near to start plots.
	--AddGoodies(this);

	-- Continental artwork selection must wait until Areas are finalized, so it gets handled last.
	DetermineContinents();



	

	print("CreateMap End");
end
------------------------------------------------------------------------------
-- avoid crashes by initializing the map plots to SOMETHING
------------------------------------------------------------------------------
function ANC_SafeInitPlots(this)
	-- PLOT_HILL TERRAIN_GRASS

	local maxX, maxY = Map.GetGridSize();
	this.plotTypes = table.fill(PlotTypes.PLOT_OCEAN, maxX * maxY);
	this.plotTerrain = table.fill(TerrainTypes.TERRAIN_OCEAN, maxX * maxY); -- never use TERRAIN_HILL?
	this.plotFeature = table.fill(FeatureTypes.NO_FEATURE, maxX * maxY);

	this.plotResource = table.fill(-1, maxX * maxY);
	this.plotResourceNum = table.fill(0, maxX * maxY);
	this.plotIsSpawnLocked = table.fill(false, maxX * maxY);

	ANC_UpdatePlots(this);
	--[[for i, plot in Plots() do
		-- type
		plot:SetPlotType(this.plotTypes[i + 1], false, false);
		-- terrain
		plot:SetPlotType(PlotTypes.PLOT_LAND, false, true);
		plot:SetTerrainType(TerrainTypes.TERRAIN_GRASS, false, true);
		plot:SetFeatureType(FeatureTypes.NO_FEATURE, -1);
	end]]
end
function ANC_UpdatePlots(this)
	for i, plot in Plots() do
		plot:SetTerrainType(this.plotTerrain[i + 1], false, true);
		plot:SetPlotType(this.plotTypes[i + 1], false, true);
		plot:SetFeatureType(this.plotFeature[i + 1], -1);
		plot:SetResourceType(this.plotResource[i + 1], this.plotResourceNum[i + 1]);
	end
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
