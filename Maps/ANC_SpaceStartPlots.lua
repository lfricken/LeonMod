

include("HBMapmakerUtilities");
include("ANC_Utils");

function ANC_SpaceStartPlots(this)
	print("ANC_SpaceStartPlots Begin");

	local additionalYBounds = 10; -- avoid spawning within this distance of the edge
	local maxX, maxY = Map.GetGridSize();
	--this.plotTypes = table.fill(PlotTypes.PLOT_OCEAN, maxX * maxY);

	-- Determine number of civilizations and city states present in this game.
	local iNumCivs, iNumCityStates, player_ID_list, bTeamGame, teams_with_major_civs, number_civs_per_team = GetPlayerAndTeamInfo()
	local totalStarting = iNumCivs + iNumCityStates;



	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));

	-- major civs
	for i,pid in pairs(player_ID_list) do
		local player = Players[pid];
		local x,y = math.floor(maxX * haltonPointsX[i]), math.floor(additionalYBounds + (maxY - 2 * additionalYBounds) * haltonPointsY[i]);
		print("Start Major: " .. x .. ", " .. y);
		local start_plot = Map.GetPlot(x, y);
		player:SetStartingPlot(start_plot);
		local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
		for k,index in pairs(indexes) do
			--this.plotTypes[index] = PlotTypes.PLOT_HILLS;
		end
	end

	-- minor civs
	for i = GameDefines.MAX_MAJOR_CIVS, GameDefines.MAX_CIV_PLAYERS - 1 do
		local player = Players[i];
		if player:IsEverAlive() then
			local x,y = math.floor(maxX * haltonPointsX[i]), math.floor(additionalYBounds + (maxY - 2 * additionalYBounds) * haltonPointsY[i]);
			print("Start Minor: " .. x .. ", " .. y);

			local start_plot = Map.GetPlot(x, y);
			player:SetStartingPlot(start_plot);
			local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
			for k,index in pairs(indexes) do
				--this.plotTypes[index] = PlotTypes.PLOT_HILLS;
			end
		end
	end

	--ANC_SetPlotTypes(this.plotTypes);
	print("ANC_SpaceStartPlots End");
end





