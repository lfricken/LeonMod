


include("ANC_LandAndSea");
include("ANC_SpaceStartPlots");
include("ANC_Climate");


function ANC_Constructor()
	local this = {
		plotTypes = {},
	};
	return this;
end
function ANC_CreateMap(ancArgs)
	print("CreateMap Begin");
	this = ANC_Constructor();
	ANC_SafeInitPlots(this);
	
	ANC_SpaceStartPlots(this);
	--ANC_LandAndSea();
	ANC_SetPlots(this.plotTypes);
	
	ANC_Climate(this);

	-- Terrain covers climate: grassland, plains, desert, tundra, snow.
	--GenerateTerrain();
	
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
	--AddGoodies();

	-- Continental artwork selection must wait until Areas are finalized, so it gets handled last.
	--DetermineContinents();



	

	print("CreateMap End");
end

function ANC_SetPlots(plotTypes)
	-- NOTE: Plots() is indexed from 0, the way the plots are indexed in C++
	-- However, Lua tables are indexed from 1, and all incoming plot tables should be indexed this way.
	-- So we add 1 to the Plots() index to find the matching plot data in plotTypes.
	for i, plot in Plots() do
		plot:SetPlotType(plotTypes[i + 1], false, false);
	end
end
-- avoid crashes by initializing the map plots to SOMETHING
function ANC_SafeInitPlots(this)


	local maxX, maxY = Map.GetGridSize();
	this.plotTypes = table.fill(PlotTypes.PLOT_LAND, maxX * maxY);


	for i, plot in Plots() do
		-- type
		plot:SetPlotType(this.plotTypes[i + 1], false, false);
		-- terrain
		if(plot:IsWater()) then
			if(plot:IsAdjacentToLand()) then
				plot:SetTerrainType(TerrainTypes.TERRAIN_COAST, false, false);
			else
				plot:SetTerrainType(TerrainTypes.TERRAIN_OCEAN, false, false);
			end
		else
			local grass = TerrainTypes.TERRAIN_GRASS;
			plot:SetTerrainType(grass, false, false);
		end
	end
end

