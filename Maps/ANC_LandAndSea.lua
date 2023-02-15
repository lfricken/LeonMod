
-- creates islands and grows/shrinks land

include("ANC_Utils");



function ANC_LandAndSea(this)


	for i=1,4 do -- 7 attempts (random number)
		local from, to = PlotTypes.PLOT_OCEAN, PlotTypes.PLOT_LAND;
		local chance = 800;
		--if (i%2==0) then local temp = from; from = to; to = temp; chance = 1000 - chance; end
		Mutate(this, from, to);
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
