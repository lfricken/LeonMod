

function ANC_DoResources(this)
	populateResourceInfo(this);
	createResourceClumps(this);
end

function createResourceClumps(this)
	local mapXScaling = math.cos(math.rad(30)); -- hexagon packing means the 
	print("COSINE" .. mapXScaling);
	-- redo the clump sizes so they fit more evenly
	print("Original Lux Diameter: " .. this.cfg.luxClumpDiameter);
	local shiftDistance = this.cfg.luxClumpDiameter * mapXScaling;
	local numClumpsX = math.floor(0.5 + (this.maxX / shiftDistance)); -- 60 / 11 > 5.45 > 5
	local eachXShift = (this.maxX / numClumpsX);
	this.cfg.luxClumpDiameter = eachXShift / mapXScaling; -- 60 / 5 > 12 -- divide my map scaling since this was scaled by it initially
	print("New Lux Diameter: " .. this.cfg.luxClumpDiameter);
	local numClumpsY = 1 + math.floor(0.5 + (this.maxY / this.cfg.luxClumpDiameter)); -- +1 because when we shift we'll need some overlap
	local eachYShift = this.cfg.luxClumpDiameter;

	for layer=1,1 do
		for x=1,numClumpsX do
			local isEvenX = x % 2 == 0; -- every other column should be shifted up
			for y=1,numClumpsY do
				local realX = (x * eachXShift) - (eachXShift / 2);
				local realY = (y * eachYShift) - (eachYShift / 2);
				if isEvenX then realY = realY - (eachYShift / 2); end -- shift up
				createResourceClump(this, layer, realX, realY, resource);
			end
		end
	end
end
function createResourceClump(this, layer, realX, realY, resource)
	print("Clump: " .. realX .. "," .. realY);
	local clump = {
		x = realX,
		y = realY,
		res = resource,
		distFrom = function (x1,y1, shift, maxX,maxY)
			local x = ((self.x + shift) % maxX) - ((x1 + shift) % maxX);
			local y = self.y - y1;
			return math.sqrt(x*x + y*y);
		end
	};

	if (this.resourceClumps[layer] == nil) then this.resourceClumps[layer] = {}; end -- populate layer with empty array
	local layerClumps = this.resourceClumps[layer];
	table.insert(layerClumps, clump);
end
function findClosestClump(this, layer, realX, realY)
	local shiftDistance = this.cfg.luxClumpDiameter;
	local layerClumps = this.resourceClumps[layer];
	local closestDist = 999999;
	local closestClump = {};

	-- try shifting to handle edge of map X wrap
	-- the idea is, we'll shift the x coordinates back and forth over the wrap edge so we can correctly calculate the
	-- distance as if it was adjacent. we don't need to do a shift of 0 because... why would we?
	for shift = -1, 1, 2 do
		for k,clump in ipairs(layerClumps) do
			local dist = clump:distFrom(realX,realY, shift * shiftDistance, this.maxX, this.maxY);
			if (dist < closestDist) then
				closestDist = dist;
				closestClump = clump;
			end
		end
	end
	print("closest: " .. closestDist);
	return closestClump;
end

------------------------------------------------------------------------------
-- avoid crashes by initializing the map plots to SOMETHING
------------------------------------------------------------------------------
function populateResourceInfo(this)
	this.strats = {};

	this.bonusPolar = {};
	this.bonusTropical = {};

	this.bonusWater = {};

	this.luxPolar = {};
	this.luxTropical = {};

	this.luxWater = {};

	for resource_data in GameInfo.Resources() do

		local rid = resource_data.ID;
		local rName = resource_data.Type;
		print("" .. rName .. ":" .. rid);


		-- Set up Strategic IDs
		if rName == "RESOURCE_IRON" then
			this.iron_ID = rid;
			table.insert(this.strats, rid);
		elseif rName == "RESOURCE_HORSE" then
			this.horse_ID = rid;
			table.insert(this.strats, rid);
		elseif rName == "RESOURCE_COAL" then
			this.coal_ID = rid;
			table.insert(this.strats, rid);
		elseif rName == "RESOURCE_OIL" then
			this.oil_ID = rid;
			table.insert(this.strats, rid);
		elseif rName == "RESOURCE_ALUMINUM" then
			this.aluminum_ID = rid;
			table.insert(this.strats, rid);
		elseif rName == "RESOURCE_URANIUM" then
			this.uranium_ID = rid;
			table.insert(this.strats, rid);


		-- Set up Bonus IDs
		elseif rName == "RESOURCE_STONE" then
			this.stone_ID = rid;
			table.insert(this.bonusPolar, rid);
		elseif rName == "RESOURCE_HARDWOOD" then	-- MOD.HungryForFood: New
			this.hardwood_ID = rid;
			table.insert(this.bonusPolar, rid);
		elseif rName == "RESOURCE_DEER" then
			this.deer_ID = rid;
			table.insert(this.bonusPolar, rid);
		elseif rName == "RESOURCE_BISON" then
			this.bison_ID = rid;
			table.insert(this.bonusPolar, rid);

		elseif rName == "RESOURCE_FISH" then
			this.fish_ID = rid;
			table.insert(this.bonusWater, rid);

		elseif rName == "RESOURCE_SHEEP" then
			this.sheep_ID = rid;
			table.insert(this.bonusTropical, rid);
		elseif rName == "RESOURCE_WHEAT" then
			this.wheat_ID = rid;
			table.insert(this.bonusTropical, rid);
		elseif rName == "RESOURCE_COW" then -- cattle
			this.cow_ID = rid;
			table.insert(this.bonusTropical, rid);
		elseif rName == "RESOURCE_MAIZE" then	-- MOD.HungryForFood: New
			this.maize_ID = rid;
			table.insert(this.bonusTropical, rid);
		elseif rName == "RESOURCE_BANANA" then
			this.banana_ID = rid;
			table.insert(this.bonusTropical, rid);


		-- Set up Luxury IDs
		elseif rName == "RESOURCE_LAPIS" then	-- MOD.Barathor: New
			this.lapis_ID = rid;
			table.insert(this.luxPolar, rid);
		elseif rName == "RESOURCE_GOLD" then
			this.gold_ID = rid;
		elseif rName == "RESOURCE_SILVER" then
			this.silver_ID = rid;
		elseif rName == "RESOURCE_GEMS" then
			this.gems_ID = rid;
		elseif rName == "RESOURCE_AMBER" then	-- MOD.Barathor: New
			this.amber_ID = rid;
		elseif rName == "RESOURCE_JADE" then		-- MOD.Barathor: New
			this.jade_ID = rid;
		elseif rName == "RESOURCE_COPPER" then
			this.copper_ID = rid;
		elseif rName == "RESOURCE_SALT" then
			this.salt_ID = rid;
		elseif rName == "RESOURCE_FUR" then
			this.fur_ID = rid;
		elseif rName == "RESOURCE_MARBLE" then
			this.marble_ID = rid;
		elseif rName == "RESOURCE_OBSIDIAN" then	-- MOD.HungryForFood: New
			this.obsidian_ID = rid;


		elseif rName == "RESOURCE_WHALE" then
			this.whale_ID = rid;
			table.insert(this.luxWater, rid);
		elseif rName == "RESOURCE_CRAB" then
			this.crab_ID = rid;
			table.insert(this.luxWater, rid);
		elseif rName == "RESOURCE_CORAL" then	-- MOD.Barathor: New
			this.coral_ID = rid;
			table.insert(this.luxWater, rid);
		elseif rName == "RESOURCE_PEARLS" then
			this.pearls_ID = rid;
			table.insert(this.luxWater, rid);


		elseif rName == "RESOURCE_IVORY" then
			this.ivory_ID = rid;
			table.insert(this.luxTropical, rid);
		elseif rName == "RESOURCE_SILK" then
			this.silk_ID = rid;
		elseif rName == "RESOURCE_DYE" then
			this.dye_ID = rid;
		elseif rName == "RESOURCE_SPICES" then
			this.spices_ID = rid;
		elseif rName == "RESOURCE_SUGAR" then
			this.sugar_ID = rid;
		elseif rName == "RESOURCE_COTTON" then
			this.cotton_ID = rid;
		elseif rName == "RESOURCE_WINE" then
			this.wine_ID = rid;
		elseif rName == "RESOURCE_INCENSE" then
			this.incense_ID = rid;
		elseif rName == "RESOURCE_CITRUS" then
			this.citrus_ID = rid;
		elseif rName == "RESOURCE_TRUFFLES" then
			this.truffles_ID = rid;
		elseif rName == "RESOURCE_COCOA" then
			this.cocoa_ID = rid;
		elseif rName == "RESOURCE_COFFEE" then	-- MOD.Barathor: New
			this.coffee_ID = rid;
		elseif rName == "RESOURCE_TEA" then		-- MOD.Barathor: New
			this.tea_ID = rid;
		elseif rName == "RESOURCE_TOBACCO" then	-- MOD.Barathor: New
			this.tobacco_ID = rid;
		elseif rName == "RESOURCE_OLIVE" then	-- MOD.Barathor: New
			this.olives_ID = rid;
		elseif rName == "RESOURCE_PERFUME" then	-- MOD.Barathor: New
			this.perfume_ID = rid;
		elseif rName == "RESOURCE_COCONUT" then	-- MOD.HungryForFood: New
			this.coconut_ID = rid;
		elseif rName == "RESOURCE_RUBBER" then	-- MOD.HungryForFood: New
			this.rubber_ID = rid;




		-- NOT USED
		elseif rName == "RESOURCE_PLATINUM" then	-- MOD.HungryForFood: New
			this.platinum_ID = rid;
		elseif rName == "RESOURCE_POPPY" then	-- MOD.HungryForFood: New
			this.poppy_ID = rid;
		elseif rName == "RESOURCE_TIN" then		-- MOD.HungryForFood: New
			this.tin_ID = rid;
		elseif rName == "RESOURCE_LAVENDER" then	-- MOD.HungryForFood: New
			this.lavender_ID = rid;

		-- Even More Resources for Vox Populi (bonus)
		elseif rName == "RESOURCE_LEAD" then		-- MOD.HungryForFood: New
			this.lead_ID = rid;
		elseif rName == "RESOURCE_PINEAPPLE" then	-- MOD.HungryForFood: New
			this.pineapple_ID = rid;
		elseif rName == "RESOURCE_POTATO" then	-- MOD.HungryForFood: New
			this.potato_ID = rid;
		elseif rName == "RESOURCE_RICE" then	-- MOD.HungryForFood: New
			this.rice_ID = rid;
		elseif rName == "RESOURCE_SULFUR" then	-- MOD.HungryForFood: New
			this.sulfur_ID = rid;
		elseif rName == "RESOURCE_TITANIUM" then	-- MOD.HungryForFood: New
			this.titanium_ID = rid;
		end
	end
end


