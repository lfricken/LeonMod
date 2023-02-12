

include("HBMapmakerUtilities");
include("ANC_Utils");


------------------------------------------------------------------------------
-- Determines where players spawn
------------------------------------------------------------------------------
function ANC_SpaceStartPlots(this)
	print("ANC_SpaceStartPlots Begin");

	local additionalYBounds = 10; -- avoid spawning within this distance of the edge
	local maxX, maxY = Map.GetGridSize();
	print("Dims: " .. maxX .. "," .. maxY);
	--this.plotTypes = table.fill(PlotTypes.PLOT_OCEAN, maxX * maxY);

	-- Determine number of civilizations and city states present in this game.
	local iNumMajorCivs, iNumCityStates, player_ID_list, bTeamGame, teams_with_major_civs, number_civs_per_team = GetPlayerAndTeamInfo()
	local totalStarting = iNumMajorCivs + iNumCityStates;



	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));

	local spawnXy = getMajorCivSpawnPoints(iNumMajorCivs, maxX, maxY);
	if (this.cfg.shufflePositions) then
		spawnXy = CopyAndShuffle(spawnXy);
	end

	-- major civs
	for i,pid in pairs(player_ID_list) do
		local player = Players[pid];
		--local x,y = math.floor(maxX * haltonPointsX[i]), math.floor(additionalYBounds + (maxY - 2 * additionalYBounds) * haltonPointsY[i]);
		local x,y = spawnXy[i][1], spawnXy[i][2];
		print("Start Major: " .. x .. ", " .. y);
		local start_plot = Map.GetPlot(x, y);
		player:SetStartingPlot(start_plot);
		local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
		for k,index in pairs(indexes) do
			this.plotTypes[index] = PlotTypes.PLOT_HILLS;
			this.plotTerrain[index] = TerrainTypes.TERRAIN_PLAINS;
		end
	end

	-- minor civs
	for i = GameDefines.MAX_MAJOR_CIVS, GameDefines.MAX_CIV_PLAYERS - 1 do
		local haltonPoint = 5 + (i - GameDefines.MAX_MAJOR_CIVS); -- skip over first 4 halton points
		local player = Players[i];
		if player:IsEverAlive() then
			local x,y = math.floor(maxX * haltonPointsX[haltonPoint]), math.floor(additionalYBounds + (maxY - 2 * additionalYBounds) * haltonPointsY[haltonPoint]);
			print("Start Minor: " .. x .. ", " .. y);

			local start_plot = Map.GetPlot(x, y);
			player:SetStartingPlot(start_plot);
			local indexes = GetIndexesAround(x, y, maxX, maxY, 0, 1);
			for k,index in pairs(indexes) do
				this.plotTypes[index] = PlotTypes.PLOT_HILLS;
				this.plotTerrain[index] = TerrainTypes.TERRAIN_GRASS;
			end
		end
	end

	--ANC_SetPlotTypes(this.plotTypes);
	print("ANC_SpaceStartPlots End");
end






------------------------------------------------------------------------------
------------------------------------------------------------------------------
function getMajorCivSpawnPoints(numMajorCivs, maxX, maxY)
	local spawnFloat = {}; -- floating points
	if numMajorCivs == 1 then
		spawnFloat,maxVariation = getFor1();
	end
	if numMajorCivs == 2 then
		spawnFloat,maxVariation = getFor2();
	end
	if numMajorCivs == 3 then
		spawnFloat,maxVariation = getFor3();
	end
	if numMajorCivs == 4 then
		spawnFloat,maxVariation = getFor4();
	end
	if numMajorCivs == 5 then
		spawnFloat,maxVariation = getFor5();
	end
	if numMajorCivs == 6 then
		spawnFloat,maxVariation = getFor6();
	end
	if numMajorCivs == 7 then
		spawnFloat,maxVariation = getFor7();
	end
	if numMajorCivs == 8 then
		spawnFloat,maxVariation = getFor8();
	end
	if numMajorCivs == 9 then
		spawnFloat,maxVariation = getFor9();
	end
	if numMajorCivs == 10 then
		spawnFloat,maxVariation = getFor10();
	end
	if numMajorCivs == 11 then
		spawnFloat,maxVariation = getFor12();
	end
	if numMajorCivs == 12 then
		spawnFloat,maxVariation = getFor12();
	end
	if numMajorCivs > 12 and numMajorCivs <= 16 then
		spawnFloat,maxVariation = getFor16();
	end

	-- scale up to world coordinates
	local spawnXy = {};
	for k,xy in pairs(spawnFloat) do
		local coordinate = {math.floor(maxX * xy[1]), math.floor(maxY * xy[2])};
		table.insert(spawnXy, coordinate);
	end
	return spawnXy;
end
------------------------------------------------------------------------------
-- 1,1 is far lower right corner, 0,0 is other far left top corner
-- remember that X wraps, which means X distances should be considered accross that boundary
------------------------------------------------------------------------------
function getFor1()
	local f = {};
	table.insert(f, {1/2, 1/2}); -- dead center
	local maxVariation = {2/5, 2/5};
	return f,maxVariation;
end
 -- REMEMBER! x wraps, these are equal distance accross X dimension, positive and negative
function getFor2()
	local f = {};
	table.insert(f, {1/4, 1/3}); -- opposite corners
	table.insert(f, {3/4, 2/3});
	local maxVariation = {2/5, 1/4};
	return f,maxVariation;
end
function getFor3()
	local f = {};
	table.insert(f, {1/6, 1/2}); -- 3 accross the middle
	table.insert(f, {3/6, 1/2});
	table.insert(f, {5/6, 1/2});
	local maxVariation = {1/7, 1/3};
	return f,maxVariation;
end
function getFor4()
	local f = {};
	table.insert(f, {1/8, 1/4}); -- four corners
	table.insert(f, {3/8, 3/4});
	table.insert(f, {5/8, 1/4});
	table.insert(f, {7/8, 3/4});
	local maxVariation = {1/9, 1/5};
	return f,maxVariation;
end
function getFor5()
	local f = {};
	table.insert(f, {2/10, 1/4}); -- 4 around
	table.insert(f, {2/10, 3/4});
	table.insert(f, {8/10, 1/4});
	table.insert(f, {8/10, 3/4});

	table.insert(f, {5/10, 2/4}); -- one in middle
	local maxVariation = {2/11, 1/5};
	return f,maxVariation;
end
function getFor6()
	local f = {};
	table.insert(f, {1/6, 1/4}); -- 3 top
	table.insert(f, {3/6, 1/4});
	table.insert(f, {5/6, 1/4});
	
	table.insert(f, {1/6, 3/4}); -- 3 bottom
	table.insert(f, {3/6, 3/4});
	table.insert(f, {5/6, 3/4});
	local maxVariation = {1/7, 1/5};
	return f,maxVariation;
end
function getFor7()
	local f = {};
	table.insert(f, {1/6, 1/4}); -- 3 top (squished Y)
	table.insert(f, {3/6, 1/4});
	table.insert(f, {5/6, 1/4});
	
	local top = 3/5;
	table.insert(f, {1/8, top}); -- 4 bottom (squished X)
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/6};
	return f,maxVariation;
end
function getFor8()
	local f = {};
	local bottom = 1/4;
	table.insert(f, {1/8, bottom}); -- 4 bottom
	table.insert(f, {3/8, bottom});
	table.insert(f, {5/8, bottom});
	table.insert(f, {7/8, bottom});
	
	local top = 3/4;
	table.insert(f, {1/8, top}); -- 4 top
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/5};
	return f,maxVariation;
end
function getFor9()
	local f = {};
	local bottom = 1/6;
	table.insert(f, {1/6, bottom}); -- 3 bottom
	table.insert(f, {3/6, bottom});
	table.insert(f, {5/6, bottom});

	local middle = 3/6;
	table.insert(f, {1/6, middle}); -- 3 middle
	table.insert(f, {3/6, middle});
	table.insert(f, {5/6, middle});
	
	local top = 5/6;
	table.insert(f, {1/6, top}); -- 3 top
	table.insert(f, {3/6, top});
	table.insert(f, {5/6, top});
	local maxVariation = {1/7, 1/7};
	return f,maxVariation;
end
function getFor10()
	local f = {};

	table.insert(f, {2/20, 1/5}); -- 4 around
	table.insert(f, {2/20, 4/5});
	table.insert(f, {8/20, 1/5});
	table.insert(f, {8/20, 4/5});

	table.insert(f, {5/20, 2/4}); -- one in middle


	table.insert(f, {12/20, 1/5}); -- 4 around
	table.insert(f, {12/20, 4/5});
	table.insert(f, {18/20, 1/5});
	table.insert(f, {18/20, 4/5});

	table.insert(f, {15/20, 2/4}); -- one in middle

	local maxVariation = {2/22, 1/5};
	return f,maxVariation;
end
function getFor12()
	local f = {};

	local bottom = 1/6;
	table.insert(f, {1/8, bottom}); -- 4 bottom
	table.insert(f, {3/8, bottom});
	table.insert(f, {5/8, bottom});
	table.insert(f, {7/8, bottom});
	
	local middle = 5/6;
	table.insert(f, {0/8, middle}); -- 4 middle
	table.insert(f, {2/8, middle});
	table.insert(f, {4/8, middle});
	table.insert(f, {6/8, middle});
	local maxVariation = {1/9, 1/5};

	local top = 5/6;
	table.insert(f, {1/8, top}); -- 4 top
	table.insert(f, {3/8, top});
	table.insert(f, {5/8, top});
	table.insert(f, {7/8, top});
	local maxVariation = {1/9, 1/7};
	return f,maxVariation;
end
function getFor16()
	local f = {};
	local div = 8;
	local offset = 1/8;
	local yPos;
	yPos = 1/8;
	table.insert(f, {1/div, yPos}); -- 4 bottom
	table.insert(f, {3/div, yPos});
	table.insert(f, {5/div, yPos});
	table.insert(f, {7/div, yPos});

	yPos = 3/8;
	table.insert(f, {1/div + offset, yPos});
	table.insert(f, {3/div + offset, yPos});
	table.insert(f, {5/div + offset, yPos});
	table.insert(f, {7/div + offset, yPos});

	yPos = 5/8;
	table.insert(f, {1/div, yPos});
	table.insert(f, {3/div, yPos});
	table.insert(f, {5/div, yPos});
	table.insert(f, {7/div, yPos});

	yPos = 7/8;
	table.insert(f, {1/div + offset, yPos});
	table.insert(f, {3/div + offset, yPos});
	table.insert(f, {5/div + offset, yPos});
	table.insert(f, {7/div + offset, yPos});
	
	local maxVariation = {1/(div + 1), 1/7};
	return f,maxVariation;
end




