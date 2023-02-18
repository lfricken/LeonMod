
-- creates islands and grows/shrinks land

include("ANC_Utils");


function fill(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end
	local doMutate = false;

	if (numAdjacent == 1) then return false;
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 500);
	elseif (numAdjacent >= 5) then return (Map.Rand(1000, "Mutate") < 800); end
	return false;
end
function tendril(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end
	local doMutate = false;

	if (numAdjacent == 1) then return (Map.Rand(1000, "Mutate") < 800);
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 500);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent >= 5) then return false; end
	return false;
end
function expand(bWouldConnect,numAdjacent)
	if (bWouldConnect) then return false; end
	local doMutate = false;

	if (numAdjacent == 1) then return (Map.Rand(1000, "Mutate") < 100);
	elseif (numAdjacent == 2) then return (Map.Rand(1000, "Mutate") < 200);
	elseif (numAdjacent == 3) then return (Map.Rand(1000, "Mutate") < 300);
	elseif (numAdjacent == 4) then return (Map.Rand(1000, "Mutate") < 400);
	elseif (numAdjacent >= 5) then return (Map.Rand(1000, "Mutate") < 500); end
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

	-- randomly throw some islands
	for i, plot in Plots() do
		if (this.plotTypes[i] == PlotTypes.PLOT_OCEAN and (Map.Rand(1000, "RandIsland") < 5)) then
			this.plotTypes[i] = PlotTypes.PLOT_LAND;
		end
	end

	-- grow islands
	for i=1,8 do
		growLand(this);
		fillLand(this);
		fillLand(this);
	end
	for i=1,3 do
		fillLand(this);
	end

	-- fix alley ways to look better
	-- (basically they all become 1 wide, this fixes that)
	for i=1,4 do
		expandWater(this);
	end
end
