

include("HBMapmakerUtilities");
include("ANC_Utils");
include("ANC_StartPlotsCreate");


------------------------------------------------------------------------------
-- Determines where players spawn
------------------------------------------------------------------------------
function ANC_SetupStartPlots(this)
	print("ANC_SpaceStartPlots Begin");

	local maxX, maxY = Map.GetGridSize();

	-- Determine number of civilizations and city states present in this game.
	local numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds = ANC_GetPlayerAndTeamInfo()


	-- halton for minor civs
	local haltonPointsX = halton(2, 600, 2 + Map.Rand(6,"Halton Rng"));
	local haltonPointsY = halton(3, 600, 2 + Map.Rand(6,"Halton Rng"));

	-- predetermined for major civs
	local spawnXy, _, _ = ANC_getMajorCivSpawnPoints(numMajorCivs, maxX, maxY);
	if (this.cfg.shufflePositions) then
		spawnXy = CopyAndShuffle(spawnXy);
	end


	-- temp assign starts so the game doesn't crash later
	-- if these aren't assigned, C++ crashes
	Map.RecalculateAreas();
	for idx,pid in ipairs(majorIds) do
		local temp_start_plot = Map.GetPlot(pid % 17, pid % 19);
		local player = Players[pid];
		player:SetStartingPlot(temp_start_plot);
	end
	for idx,pid in ipairs(minorIds) do
		local temp_start_plot = Map.GetPlot(pid % 17, pid % 19);
		local player = Players[pid];
		player:SetStartingPlot(temp_start_plot);
	end


	-- major civs
	for idx,pid in ipairs(majorIds) do
		local player = Players[pid];
		local x,y = spawnXy[idx][1], spawnXy[idx][2];
		--print("Start Major: " .. x .. ", " .. y);
		local start_plot = Map.GetPlot(x, y);
		player:SetStartingPlot(start_plot);

		ANC_DoSpawnFor(this, x, y, maxX, maxY, pid, false);
	end

	-- minor civs
	local haltonPoint = 0;
	local additionalYBounds = 1; -- avoid spawning within this distance of the edge
	for idx,pid in ipairs(minorIds) do
		local player = Players[pid];

		-- find valid start location
		local x,y;
		for attempts=1,20 do -- limit attempts to avoid an infinite loop
			haltonPoint = haltonPoint + 1;
			x,y = math.floor(maxX * haltonPointsX[haltonPoint]), math.floor(additionalYBounds + (maxY - 2 * additionalYBounds) * haltonPointsY[haltonPoint]);
			local plotIdx = GetI(x, y, maxX);
			-- found a valid plot?
			if (not this.plotIsWithinSpawnDist[plotIdx]) then break; end

			if (attempts > 18) then print("WARNING: Map Too Small. Minor civ cant find spawn location."); end
		end

		--print("Start Minor: " .. x .. ", " .. y);

		local start_plot = Map.GetPlot(x, y);
		player:SetStartingPlot(start_plot);

		ANC_DoSpawnFor(this, x, y, maxX, maxY, pid, true);
	end

	--ANC_SetPlotTypes(this.plotTypes);
	print("ANC_SpaceStartPlots End");
end
------------------------------------------------------------------------------
-- Determines the XY size of the map
------------------------------------------------------------------------------
function ANC_calcMapSizeXy(numMajorCivs,numMinorCivs,ratio,tilesPerMajorCiv)
	-- ratio 4, 2
	local targetNumTiles = numMajorCivs * tilesPerMajorCiv;
	local A = ratio[1] / ratio[2]; -- A = x / y ; 2
	--A * x * x = targetNum
	--x * x = targetNum / A
	local base = math.floor(math.sqrt(targetNumTiles / A)); -- y = sqrt(targetNum / A)
	local y = math.floor(0.5 + base);
	local x = math.floor(0.5 + base * A);

	return {x, y};
end
------------------------------------------------------------------------------
-- returns
-- xy coordinates of every major civ player
------------------------------------------------------------------------------
function ANC_getMajorCivSpawnPoints(numMajorCivs, maxX, maxY)
	local spawnFloat = {}; -- floating points
	if numMajorCivs == 1 then
		spawnFloat,maxVariation,shapeRatio = getFor1();
	end
	if numMajorCivs == 2 then
		spawnFloat,maxVariation,shapeRatio = getFor2();
	end
	if numMajorCivs == 3 then
		spawnFloat,maxVariation,shapeRatio = getFor3();
	end
	if numMajorCivs == 4 then
		spawnFloat,maxVariation,shapeRatio = getFor4();
	end
	if numMajorCivs == 5 then
		spawnFloat,maxVariation,shapeRatio = getFor5();
	end
	if numMajorCivs == 6 then
		spawnFloat,maxVariation,shapeRatio = getFor6();
	end
	if numMajorCivs == 7 then
		spawnFloat,maxVariation,shapeRatio = getFor7();
	end
	if numMajorCivs == 8 then
		spawnFloat,maxVariation,shapeRatio = getFor8();
	end
	if numMajorCivs == 9 then
		spawnFloat,maxVariation,shapeRatio = getFor9();
	end
	if numMajorCivs == 10 then
		spawnFloat,maxVariation,shapeRatio = getFor10();
	end
	if numMajorCivs == 11 then
		spawnFloat,maxVariation,shapeRatio = getFor12();
	end
	if numMajorCivs == 12 then
		spawnFloat,maxVariation,shapeRatio = getFor12();
	end
	if numMajorCivs > 12 and numMajorCivs <= 16 then
		spawnFloat,maxVariation,shapeRatio = getFor16();
	end

	-- scale up to world coordinates
	local spawnXy = {};
	for k,xy in ipairs(spawnFloat) do
		local coordinate = {math.floor(maxX * xy[1]), math.floor(maxY * xy[2])};
		table.insert(spawnXy, coordinate);
	end
	return spawnXy,maxVariation,shapeRatio;
end
------------------------------------------------------------------------------
-- 1,1 is far lower right corner, 0,0 is other far left top corner
-- remember that X wraps, which means X distances should be considered accross that boundary
------------------------------------------------------------------------------
function getFor1()
	local f = {};
	table.insert(f, {1/2, 1/2}); -- dead center
	local maxVariation = {2/5, 2/5};
	return f,maxVariation,{1,1};
end
 -- REMEMBER! x wraps, these are equal distance accross X dimension, positive and negative
function getFor2()
	local f = {};
	table.insert(f, {1/4, 1/3}); -- opposite corners
	table.insert(f, {3/4, 2/3});
	local maxVariation = {2/5, 1/4};
	return f,maxVariation,{1,1};
end
function getFor3()
	local f = {};
	table.insert(f, {1/6, 1/2}); -- 3 accross the middle
	table.insert(f, {3/6, 1/2});
	table.insert(f, {5/6, 1/2});
	local maxVariation = {1/7, 1/3};
	return f,maxVariation,{3,2};
end
function getFor4()
	local f = {};
	table.insert(f, {1/8, 1/4}); -- four corners
	table.insert(f, {3/8, 3/4});
	table.insert(f, {5/8, 1/4});
	table.insert(f, {7/8, 3/4});
	local maxVariation = {1/9, 1/5};
	return f,maxVariation,{1,1};
end
function getFor5()
	local f = {};
	table.insert(f, {2/10, 1/4}); -- 4 around
	table.insert(f, {2/10, 3/4});
	table.insert(f, {8/10, 1/4});
	table.insert(f, {8/10, 3/4});

	table.insert(f, {5/10, 2/4}); -- one in middle
	local maxVariation = {2/11, 1/5};
	return f,maxVariation,{4,4};
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
	return f,maxVariation,{4,3};
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
	return f,maxVariation,{1,1};
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
	return f,maxVariation,{4,3};
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
	return f,maxVariation,{1,1};
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
	return f,maxVariation,{3,2};
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
	return f,maxVariation,{4,3};
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
	return f,maxVariation,{16,16};
end




