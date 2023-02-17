
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
	Mutate(this, from, to, nil, tendril);
end
function fillLand(this)
	local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
	Mutate(this, from, to, nil, fill);
end
function growWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this, from, to, nil, tendril);
end
function fillWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this, from, to, nil, fill);
end
function expandWater(this)
	local from, to = PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN;
	Mutate(this, from, to, nil, expand);
end


function ANC_LandAndSea(this)


	for i=1,8 do
		growLand(this);
		fillLand(this);
	end

	for i=1,2 do
		expandWater(this);
	end
	--[[
	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));
	for i=1,10 do
		local x,y = math.floor(maxX * haltonPointsX[i]), math.floor(maxY * haltonPointsY[i]);
		local indexes = GetIndexesAround(x, y, maxX, maxY, 2, 3);
		for k,index in pairs(indexes) do
			print("adding land");
			this.plotTypes[index] = PlotTypes.PLOT_HILLS;
		end
	end]]

	--ANC_SetPlotTypes(this.plotTypes);
end
