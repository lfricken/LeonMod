-- copied from HBFeatureGeneratorRectangular.lua

include("HBMapmakerUtilities");

------------------------------------------------------------------------------
FeatureGenerator = {};
------------------------------------------------------------------------------
function FeatureGenerator.Create(args)
	print("leon feet");
	--[[ Civ4's truncated "Climate" setting has been abandoned. Civ5 has returned to 
	Civ3-style map options for World Age, Temperature, and Rainfall. Control over the 
	terrain has been removed from the XML.  - Bob Thomas, March 2010  ]]--
	--
	-- Sea Level and World Age map options affect only plot generation.
	-- Temperature map options affect only terrain generation.
	-- Rainfall map options affect only feature generation.
	--	
	local grassMoist = Map.GetCustomOption(8);

	local args = args or {};
	local rainfall = args.rainfall or 2; -- Default is Normal rainfall.
	local jungle_grain = args.jungle_grain or 5;
	local forest_grain = args.forest_grain or 6;
	local clump_grain = args.clump_grain or 10;
	local iJungleChange = args.iJungleChange or 20;
	local iForestChange = args.iForestChange or 7;
	local iClumpChange = args.iClumpChange or 5;
	local iJungleFactor = args.iJungleFactor or 7;
	local iAridFactor = args.iAridFactor or 6;
	local iWetFactor = args.iWetFactor or 2;
	local fMarshChange = args.fMarshChange or 1.5;
	local fOasisChange = args.fOasisChange or 1.5;
	local fracXExp = args.fracXExp or -1;
	local fracYExp = args.fracYExp or -1;
	
	-- Set feature traits.
	local iJunglePercent = args.iJunglePercent or 42;

	if grassMoist == 1 then
		iJunglePercent = iJunglePercent - 5;
	elseif grassMoist == 3 then
		iJunglePercent = iJunglePercent + 5;
	end

	local iForestPercent = args.iForestPercent or 18;
	local iClumpHeight = args.iClumpHeight or 75;
	local fMarshPercent = args.fMarshPercent or 8;
	local iOasisPercent = args.iOasisPercent or 25;

	-- if MapShape == 3 then
	-- 	iForestPercent = iForestPercent + 6;
	-- 	iJunglePercent = iJunglePercent + 3;
	-- 	fMarshPercent = fMarshPercent + 1;
	-- end

	-- Adjust foliage amounts according to user's Rainfall selection. (Which must be passed in by the map script.)
	if rainfall == 1 then -- Rainfall is sparse, climate is Arid.
		iJunglePercent = iJunglePercent - iJungleChange;
		iJungleFactor = iAridFactor;
		iForestPercent = iForestPercent - iForestChange;
		iClumpHeight = iClumpHeight - iClumpChange;
		fMarshPercent = fMarshPercent / fMarshChange;
		iOasisPercent = iOasisPercent / fOasisChange;
	elseif rainfall == 3 then -- Rainfall is abundant, climate is Wet.
		iJunglePercent = iJunglePercent + iJungleChange;
		iJungleFactor = iWetFactor;
		iForestPercent = iForestPercent + iForestChange;
		iClumpHeight = iClumpHeight + iClumpChange;
		fMarshPercent = fMarshPercent * fMarshChange;
		iOasisPercent = iOasisPercent * fOasisChange;
	else -- Rainfall is Normal.
	end

	--[[ Activate printout for debugging only.
	print("-"); print("--- Rainfall Readout ---");
	print("- Rainfall Setting:", rainfall);
	print("- Jungle Percentage:", iJunglePercent);
	print("- Loose Forest %:", iForestPercent);
	print("- Clump Forest %:", 100 - iClumpHeight);
	print("- Marsh Percentage:", fMarshPercent);
	print("- Oasis Percentage:", iOasisPercent);
	print("- - - - - - - - - - - - - - -");
	]]--

	local gridWidth, gridHeight = Map.GetGridSize();
	local world_info = GameInfo.Worlds[Map.GetWorldSize()];
	jungle_grain = jungle_grain + world_info.FeatureGrainChange;
	forest_grain = forest_grain + world_info.FeatureGrainChange;

	-- create instance data
	local instance = {
	
		-- methods
		__initFractals		= FeatureGenerator.__initFractals,
		__initFeatureTypes	= FeatureGenerator.__initFeatureTypes,
		AddFeatures			= FeatureGenerator.AddFeatures,
		GetLatitudeAtPlot	= FeatureGenerator.GetLatitudeAtPlot,
		AddFeaturesAtPlot	= FeatureGenerator.AddFeaturesAtPlot,
		AddOasisAtPlot		= FeatureGenerator.AddOasisAtPlot,
		AddIceAtPlot		= FeatureGenerator.AddIceAtPlot,
		AddMarshAtPlot		= FeatureGenerator.AddMarshAtPlot,
		AddJunglesAtPlot	= FeatureGenerator.AddJunglesAtPlot,
		AddForestsAtPlot	= FeatureGenerator.AddForestsAtPlot,
		AddAtolls			= FeatureGenerator.AddAtolls,
		AdjustTerrainTypes	= FeatureGenerator.AdjustTerrainTypes,
		
		-- members
		iGridW = gridWidth,
		iGridH = gridHeight,
		
		iJunglePercent = iJunglePercent,
		iJungleFactor = iJungleFactor,
		iForestPercent = iForestPercent,
		iClumpHeight = iClumpHeight,
		fMarshPercent = fMarshPercent,
		iOasisPercent = iOasisPercent,
	
		jungle_grain = jungle_grain,
		forest_grain = forest_grain,
		clump_grain = clump_grain,
		
		fractalFlags = Map.GetFractalFlags(),
		fracXExp = fracXExp,
		fracYExp = fracYExp,
	};

	-- initialize instance data
	instance:__initFractals()
	instance:__initFeatureTypes()
	
	return instance;
end
------------------------------------------------------------------------------
function FeatureGenerator:__initFractals()
	local width = self.iGridW;
	local height = self.iGridH;
	
	-- Create fractals
	self.jungles		= Fractal.Create(width, height, self.jungle_grain, self.fractalFlags, self.fracXExp, self.fracYExp);
	self.forests		= Fractal.Create(width, height, self.forest_grain, self.fractalFlags, self.fracXExp, self.fracYExp);
	self.forestclumps	= Fractal.Create(width, height, self.clump_grain, self.fractalFlags, self.fracXExp, self.fracYExp);
	self.marsh			= Fractal.Create(width, height, 4, self.fractalFlags, self.fracXExp, self.fracYExp);
	
	-- Get heights
	self.iJungleBottom	= self.jungles:GetHeight((100 - self.iJunglePercent)/2)
	self.iJungleTop		= self.jungles:GetHeight((100 + self.iJunglePercent)/2)
	self.iJungleRange	= (self.iJungleTop - self.iJungleBottom) * self.iJungleFactor;
	self.iForestLevel	= self.forests:GetHeight(100 - self.iForestPercent)
	self.iClumpLevel	= self.forestclumps:GetHeight(self.iClumpHeight)
	self.iMarshLevel	= self.marsh:GetHeight(100 - self.fMarshPercent)
end
------------------------------------------------------------------------------
function FeatureGenerator:__initFeatureTypes()

	self.featureFloodPlains = FeatureTypes.FEATURE_FLOOD_PLAINS;
	self.featureIce = FeatureTypes.FEATURE_ICE;
	self.featureJungle = FeatureTypes.FEATURE_JUNGLE;
	self.featureForest = FeatureTypes.FEATURE_FOREST;
	self.featureOasis = FeatureTypes.FEATURE_OASIS;
	self.featureMarsh = FeatureTypes.FEATURE_MARSH;
	
	self.terrainIce = TerrainTypes.TERRAIN_SNOW;
	self.terrainTundra = TerrainTypes.TERRAIN_TUNDRA;
	self.terrainPlains = TerrainTypes.TERRAIN_PLAINS;
end
------------------------------------------------------------------------------
function FeatureGenerator:AddFeatures(allow_mountains_on_coast)
	local flag = allow_mountains_on_coast or false;
	--[[ Removing mountains from coasts cannot be done during plot or terrain 
	generation, because the function that determines what is or isn't adjacent
	to a salt water ocean requires both plot and terrain data to operate. So
	even though this operation is, strictly speaking, a plot-type operation, I
	have added it here in the default FeatureGenerator so I can easily call on
	it for any script that needs it.  - Bob Thomas, March 2010  ]]--
	--
	if allow_mountains_on_coast == false then -- remove any mountains from coastal plots
		for x = 0, self.iGridW - 1 do
			for y = 0, self.iGridH - 1 do
				local plot = Map.GetPlot(x, y)
				if plot:GetPlotType() == PlotTypes.PLOT_MOUNTAIN and 60 > Map.Rand(100, "Remove Coastal Mountain") then
					if plot:IsCoastalLand() then
						plot:SetPlotType(PlotTypes.PLOT_HILLS, false, true); -- These flags are for recalc of areas and rebuild of graphics. Instead of recalc over and over, do recalc at end of loop.
					end
				end
			end
		end
		-- This function needs to recalculate areas after operating. However, so does 
		-- adding feature ice, so the recalc was removed from here and put in MapGenerator()
	end
	
	self:AddAtolls(); -- Adds Atolls to oceanic maps.
	
	-- Main loop, adds features to all plots as appropriate
	for y = 0, self.iGridH - 1, 1 do
		for x = 0, self.iGridW - 1, 1 do
			self:AddFeaturesAtPlot(x, y);
		end
	end
	
	self:AdjustTerrainTypes(); -- Sets terrain under jungles and softens arctic rivers
end
------------------------------------------------------------------------------
function FeatureGenerator:GetLatitudeAtPlot(iX, iY)
	-- Latitude affects only jungles and ice by default.
	-- However, you can make use of it in replacement methods for AddAtPlot if you wish.
	-- Returns a value in the range of 0.0 (tropical) to 1.0 (polar)
	return math.abs((self.iGridH/2) - iY)/(self.iGridH/2);
end
------------------------------------------------------------------------------
function FeatureGenerator:AddFeaturesAtPlot(iX, iY)
	-- adds any appropriate features at the plot (iX, iY) where (0,0) is in the SW
	local lat = self:GetLatitudeAtPlot(iX, iY);
	local plot = Map.GetPlot(iX, iY);

	if plot:CanHaveFeature(self.featureFloodPlains) then
		-- All desert plots along river are set to flood plains.
		plot:SetFeatureType(self.featureFloodPlains, -1)
	end
	
	if (plot:GetFeatureType() == FeatureTypes.NO_FEATURE) then
		self:AddOasisAtPlot(plot, iX, iY, lat);
	end

	if (plot:GetFeatureType() == FeatureTypes.NO_FEATURE) then
		self:AddIceAtPlot(plot, iX, iY, lat);
	end

	if (plot:GetFeatureType() == FeatureTypes.NO_FEATURE) then
		self:AddMarshAtPlot(plot, iX, iY, lat);
	end
		
	if (plot:GetFeatureType() == FeatureTypes.NO_FEATURE) then
		self:AddJunglesAtPlot(plot, iX, iY, lat);
	end
	
	if (plot:GetFeatureType() == FeatureTypes.NO_FEATURE) then
		self:AddForestsAtPlot(plot, iX, iY, lat);
	end
		
end
------------------------------------------------------------------------------
function FeatureGenerator:AddOasisAtPlot(plot, iX, iY, lat)
	if(plot:CanHaveFeature(self.featureOasis)) then
		if Map.Rand(100, "Add Oasis Lua") <= self.iOasisPercent then
			plot:SetFeatureType(self.featureOasis, -1);
		end
	end
end
------------------------------------------------------------------------------
function FeatureGenerator:AddIceAtPlot(plot, iX, iY, lat)
	if(false and plot:CanHaveFeature(self.featureIce)) then
		if Map.IsWrapX() and (iY == 0 or iY == self.iGridH - 1) then
			plot:SetFeatureType(self.featureIce, -1)

		else
			local rand = Map.Rand(700, "Add Ice Lua")/100.0;

			if(rand < 8 * (lat - 0.875)) then
				plot:SetFeatureType(self.featureIce, -1);
			elseif(rand < 4 * (lat - 0.75)) then
				plot:SetFeatureType(self.featureIce, -1);
			end
		end
	end
end
------------------------------------------------------------------------------
function FeatureGenerator:AddMarshAtPlot(plot, iX, iY, lat)
	local marsh_height = self.marsh:GetHeight(iX, iY)
	if(marsh_height >= self.iMarshLevel) then
		if(plot:CanHaveFeature(self.featureMarsh)) then
			plot:SetFeatureType(self.featureMarsh, -1)
		end
	end
end
------------------------------------------------------------------------------
function FeatureGenerator:AddJunglesAtPlot(plot, iX, iY, lat)
	local jungle_height = self.jungles:GetHeight(iX, iY);
	local climate_info = GameInfo.Climates[Map.GetClimate()];
	if jungle_height <= self.iJungleTop and jungle_height >= self.iJungleBottom + (self.iJungleRange * lat) then
		if(plot:CanHaveFeature(self.featureJungle)) then
			plot:SetFeatureType(self.featureJungle, -1);
		end
	end
end
------------------------------------------------------------------------------
function FeatureGenerator:AddForestsAtPlot(plot, iX, iY, lat)
	if (self.forests:GetHeight(iX, iY) >= self.iForestLevel) or (self.forestclumps:GetHeight(iX, iY) >= self.iClumpLevel) then
		if plot:CanHaveFeature(self.featureForest) then
			plot:SetFeatureType(self.featureForest, -1)
		end
	end
end
------------------------------------------------------------------------------
function FeatureGenerator:AdjustTerrainTypes()
	-- This function added April 2009 for Civ5, by Bob Thomas.
	-- Purpose of this function is to turn terrain under jungles
	-- into Plains, and to soften arctic terrain types at rivers.
	local width = self.iGridW - 1;
	local height = self.iGridH - 1;
	
	for y = 0, height do
		for x = 0, width do
			local plot = Map.GetPlot(x, y);
			
			if (plot:GetFeatureType() == self.featureJungle) then
				plot:SetTerrainType(self.terrainPlains, false, true)  -- These flags are for recalc of areas and rebuild of graphics. No need to recalc from any of these changes.		
			elseif (plot:IsRiver()) then
				local terrainType = plot:GetTerrainType();
				if (terrainType == self.terrainTundra) then
					plot:SetTerrainType(self.terrainPlains, false, true)
				elseif (terrainType == self.terrainIce) then
					plot:SetTerrainType(self.terrainTundra, false, true)					
				end
			end
		end
	end
end

function mod(a,b)
	return a - math.floor(a/b)*b;
end
------------------------------------------------------------------------------
function FeatureGenerator:AddAtolls()
	print("AddAtolls")
	local direction_types = {
		DirectionTypes.DIRECTION_NORTHEAST,
		DirectionTypes.DIRECTION_EAST,
		DirectionTypes.DIRECTION_SOUTHEAST,
		DirectionTypes.DIRECTION_SOUTHWEST,
		DirectionTypes.DIRECTION_WEST,
		DirectionTypes.DIRECTION_NORTHWEST
	};

	local xMod = 3;
	local yMod = 3;
	local oddsPerTile = 900;

	local iW, iH = Map.GetGridSize()
	local possibleAtolls = {};
	for thisFeature in GameInfo.Features() do
		if thisFeature.Type == "FEATURE_ATOLL" then table.insert(possibleAtolls, thisFeature.ID) end
		if thisFeature.Type == "FEATURE_ATOLL_GOLD" then table.insert(possibleAtolls, thisFeature.ID) end
		if thisFeature.Type == "FEATURE_ATOLL_PRODUCTION" then table.insert(possibleAtolls, thisFeature.ID) end
		if thisFeature.Type == "FEATURE_ATOLL_CULTURE" then table.insert(possibleAtolls, thisFeature.ID) end
		if thisFeature.Type == "FEATURE_ATOLL_SCIENCE" then table.insert(possibleAtolls, thisFeature.ID) end
	end

	for y = 10, iH - 11 do
		for x = 0, iW - 1 do
			repeat
				if mod(x,xMod) ~= 0 or mod(y,yMod) ~= 0 then do break end end

				local targetX = x + Map.Rand(xMod - 1, "");
				local targetY = y + Map.Rand(yMod - 1, "");
				local plot = Map.GetPlot(targetX, targetY);

				-- skip most plots
				if PlotTypes.PLOT_OCEAN ~= plot:GetPlotType() then do break end end			-- must be ocean
				if FeatureTypes.FEATURE_ICE == plot:GetFeatureType() then do break end end 	-- cannot be ice
				if TerrainTypes.TERRAIN_COAST ~= plot:GetTerrainType() then do break end end -- must be coast
				if plot:IsLake() then do break end end 										-- cannot be a lake
				if not plot:IsAdjacentToLand() then do break end end 						-- must be immediate coast
				if x > iW - 1 or y > iH - 11 then do break end end 

				if Map.Rand(1000, "Atoll Chance") < oddsPerTile then
					local randIdx = 1 + Map.Rand(table.getn(possibleAtolls), "atoll random");
					print("AddAtolls +1"..randIdx);
					plot:SetFeatureType(possibleAtolls[randIdx], -1);
				end
			until true
		end
	end
end
------------------------------------------------------------------------------
