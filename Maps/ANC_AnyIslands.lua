-- written by Nonchalant. No, ANC doesn't stand for anything. #FlashMX

include("ANC_Core");
include("ANC_SpaceStartPlots");

------------------------------------------------------------------------------
function GetMapScriptInfo()
	return {
		Name = "A Multiplayer Balance",
		Description = "Balanced for NonChalant mod. Supports between 1 and 16 players. Recommended #CityStates = 1.5 x #Civs.",
		IsAdvancedMap = false,
		IconIndex = 1,
		SortIndex = 2,
		SupportsMultiplayer = true,
		CustomOptions = {
			{
				Name = "Spawn Variation", -- spawnVarianceOptionId 1
				Values = {
					"0 -- Fair but predictable",
					"1",
					"2 -- Default",
					"3",
					"4",
					"5",
					"6",
					"7",
					"8",
					"9 -- Unpredictable but unfair",
				},

				DefaultValue = 3,
				SortPriority = -87,
			},
		},
	};
end

local g_tilesPerMajorCiv = 700;

------------------------------------------------------------------------------
function GetMapInitData(worldSize)
	-- calculate world size
	local numMajorCivs, majorIds, numMinors, minorIds, isTeamGame, numCivsPerTeam, majorTeamIds = ANC_GetPlayerAndTeamInfo();
	local _, _, ratio = ANC_getMajorCivSpawnPoints(ANC_CreateArgs(), numMajorCivs, 20, 20);
	--print("RATIO: " .. ratio[1] .. "," .. ratio[2]);
	local grid_size = ANC_calcMapSizeXy(numMajorCivs, numMinors, ratio, g_tilesPerMajorCiv);
	print("Size: " .. grid_size[1] .. "," .. grid_size[2]);


	--local world = GameInfo.Worlds[worldSize];
	if (grid_size ~= nil) then
		return {
			Width = grid_size[1],
			Height = grid_size[2],
			WrapX = true,
		};
	else
		print("WARNING in ANC_AnyIslands.lua GetMapInitData. Invalid dimension data.")
		return {
			Width = 90,
			Height = 90,
			WrapX = true,
		};
	end
end

--Entry point defined in CvMapGenerator::pGenerateRandomMap
function GenerateMap()
	print("GenerateMap Begin");

	Map.RandSeed(9999); -- helps see changes by guaranteeing the same map gets generated

	args = ANC_CreateArgs();

	local spawnVarianceOptionId = 1;
	args.spawnVariation = (Map.GetCustomOption(spawnVarianceOptionId) - 1) / 10; -- -1 since option 1 should be value of 0


	ANC_CreateMap(args);




	--printMapToLua(plotTypes, maxX, maxY);
	print("GenerateMap End");
end


