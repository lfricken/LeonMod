











------------------------------------------------------------------------------
------------------------------------------------------------------------------
function getMajorCivSpawnPoints(numMajorCivs, maxX, minX)
	local spawnFloat = {}; -- floating points
	if numMajorCivs == 1 then
		spawnFloat = getFor1(numMajorCivs);
	end
	if numMajorCivs == 2 then
		spawnFloat = getFor2(numMajorCivs);
	end
	if numMajorCivs == 3 then
		spawnFloat = getFor3(numMajorCivs);
	end
	if numMajorCivs == 4 then
		spawnFloat = getFor4(numMajorCivs);
	end
	if numMajorCivs == 5 then
		spawnFloat = getFor5(numMajorCivs);
	end
	if numMajorCivs == 6 then
		spawnFloat = getFor6(numMajorCivs);
	end

end
------------------------------------------------------------------------------
-- 1,1 is far lower right corner, 0,0 is other far left top corner
-- remember that X wraps, which means X distances should be considered accross that boundary
------------------------------------------------------------------------------
function getFor1(numMajorCivs)
	local f = {};
	table.insert(f, {0.5, 0.5});
	return f;
end
function getFor2(numMajorCivs)




end











