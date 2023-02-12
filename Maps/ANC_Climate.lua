

include("ANC_Utils");



function ANC_Climate(this)
	print("ANC_Climate Begin");

	for i, plot in Plots() do
		if(plot:IsWater()) then
			if(plot:IsAdjacentToLand()) then
				plot:SetTerrainType(TerrainTypes.TERRAIN_COAST, false, false);
			else
				plot:SetTerrainType(TerrainTypes.TERRAIN_OCEAN, false, false);
			end
		else
			local grass = TerrainTypes.TERRAIN_GRASS;
			plot:SetTerrainType(grass, false, false);
		end	
	end

	print("ANC_Climate End");
end
