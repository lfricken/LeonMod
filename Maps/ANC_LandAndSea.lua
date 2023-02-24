
-- creates islands and grows/shrinks land

include("ANC_Utils");


function fill(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end

	if (numAdjacent == 1) then return false;
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 500);
	elseif (numAdjacent >= 5) then return (Map.Rand(1000, "Mutate") < 800); end
	return false;
end
function tendril(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end

	if (numAdjacent == 1) then return (Map.Rand(1000, "Mutate") < 800);
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 500);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent >= 5) then return false; end
	return false;
end
function tendrilMod(prob)
	return function(bWouldConnect,numAdjacent)
		if (bWouldConnect) then return false; end

		if (numAdjacent == 1) then return (Map.Rand(1000, "Mutate") < prob * 800);
		elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < prob * 500);
		elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < prob * 300);
		elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < prob * 100);
		elseif (numAdjacent >= 5) then return false; end
		return false;
	end
end
function expand(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end

	if (numAdjacent == 1) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 200);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 400);
	elseif (numAdjacent >= 5) then return (Map.Rand(1000, "Mutate") < 500); end
	return false;
end
function swiss(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end

	if (numAdjacent == 0) then return (Map.Rand(1000, "swiss") < 25); end
	return false;
end


function growLand(this)
	local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
	Mutate(this.plotTypes, this, from, to, nil, tendril);
end
function fillLand(this)
	local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
	Mutate(this.plotTypes, this, from, to, nil, fill);
end
function growWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this.plotTypes, this, from, to, nil, tendril);
end
function fillWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this.plotTypes, this, from, to, nil, fill);
end
function expandWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this.plotTypes, this, from, to, nil, expand);
end


function ANC_LandAndSea(this)

	local isArableLand = function(idx) return this.plotTypes[idx] == PlotTypes.PLOT_LAND or this.plotTypes[idx] == PlotTypes.PLOT_HILLS; end

	-- randomly throw some islands
	for i, plot in ANC_Plots() do
		if (not this.plotIsLocked[i] and this.plotTypes[i] == PlotTypes.PLOT_OCEAN and (Map.Rand(1000, "RandIsland") < 1)) then
			this.plotTypes[i] = PlotTypes.PLOT_LAND;
		end
	end

	-- grow islands
	for i=1,8 do
		growLand(this);
		fillLand(this);
		fillLand(this);
	end
	-- make the islands beefy er
	for i=1,3 do
		fillLand(this);
	end

	-- fix alley ways to look better
	-- (basically they all become 1 wide, this fixes that)
	for i=1,4 do
		expandWater(this);
	end

	-- lakes
	for i=1,1 do
		local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
		Mutate(this.plotTypes, this, from, to, nil, swiss, ANC_ignoreLat(this.tundraLat, this.maxX, this.maxY));
	end


	for i, plot in ANC_Plots() do
		if (not this.plotIsLocked[i] and isArableLand(i) and (Map.Rand(1000, "mountains") < 15)) then
			this.plotTypes[i] = PlotTypes.PLOT_MOUNTAIN;
		end
	end
	for i=1,2 do
		local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_MOUNTAIN;
		Mutate(this.plotTypes, this, from, to, nil, tendrilMod(0.2), isArableLand);
	end
end
