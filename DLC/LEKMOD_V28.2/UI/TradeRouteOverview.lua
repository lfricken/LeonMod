-------------------------------------------------
-- Religion Overview Popup
-------------------------------------------------
include( "IconSupport" );
include( "InstanceManager" );
include( "TradeRouteHelpers" );

local g_PopupInfo = nil;

-------------------------------------------------
-- Global Variables
-------------------------------------------------
g_CurrentTab = nil;		-- The currently selected Tab.
g_iSelectedPlayerID = Game.GetActivePlayer();

-------------------------------------------------
-- Global Constants
-------------------------------------------------
g_Tabs = {
	["YourTR"] = {
		SelectHighlight = Controls.YourTRSelectHighlight,
	},
	
	["AvailableTR"] = {
		SelectHighlight = Controls.AvailableTRSelectHighlight,
	},
	
	["TRWithYou"] = {
		SelectHighlight = Controls.TRWithYouSelectHighlight,
	},
}

g_SortOptions = {
	{
		Button = Controls.Domain,
		Column = "Domain",
		DefaultDirection = "asc",
		CurrentDirection = nil,
	},
	{
		Button = Controls.FromOwnerHeader,
		Column = "FromCiv",
		SecondaryColumn = "FromCityName",
		SecondaryDirection = "asc",
		DefaultDirection = "asc",
		CurrentDirection = nil,
	},
	{
		Button = Controls.FromCityHeader,
		Column = "FromCityName",
		DefaultDirection = "asc",
		CurrentDirection = "asc",
	},
	{
		Button = Controls.ToOwnerHeader,
		Column = "ToCiv",
		SecondaryColumn = "ToCityName",
		SecondaryDirection = "asc",
		DefaultDirection = "asc",
		CurrentDirection = nil,
	},
	{
		Button = Controls.ToCityHeader,
		Column = "ToCityName",
		DefaultDirection = "asc",
		CurrentDirection = nil,
	},


	{
		Button = Controls.ToFood,
		Column = "ToFood",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.ToProduction,
		Column = "ToProduction",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.ToGPT,
		Column = "ToGPT",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},


	{
		Button = Controls.Science,
		Column = "Science",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.Culture,
		Column = "Culture",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.Faith,
		Column = "Faith",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},


	{
		Button = Controls.Golden,
		Column = "GoldenAgePoints",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},


	{
		Button = Controls.ScientificInsight,
		Column = "ScientificInsight",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.DiplomaticSupport,
		Column = "DiplomaticSupport",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.CulturalInfluence,
		Column = "Tourism",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},



	{
		Button = Controls.FromReligion,
		Column = "FromReligion",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
	{
		Button = Controls.ToReligion,
		Column = "ToReligion",
		DefaultDirection = "desc",
		CurrentDirection = nil,
		SortType = "numeric",
	},


	{
		Button = Controls.TurnsLeft,
		Column = "TurnsLeft",
		DefaultDirection = "asc",
		CurrentDirection = nil,
		SortType = "numeric",
	},
};

g_SortFunction = nil;

-------------------------------------------------------------------------------
-- Sorting Support
-------------------------------------------------------------------------------
function AlphabeticalSortFunction(field, direction, secondarySort)
	if(direction == "asc") then
		return function(a,b)
			local va = (a ~= nil and a[field] ~= nil) and a[field] or "";
			local vb = (b ~= nil and b[field] ~= nil) and b[field] or "";
			
			if(secondarySort ~= nil and va == vb) then
				return secondarySort(a,b);
			else
				return Locale.Compare(va, vb) == -1;
			end
		end
	elseif(direction == "desc") then
		return function(a,b)
			local va = (a ~= nil and a[field] ~= nil) and a[field] or "";
			local vb = (b ~= nil and b[field] ~= nil) and b[field] or "";
			
			if(secondarySort ~= nil and va == vb) then
				return secondarySort(a,b);
			else
				return Locale.Compare(va, vb) == 1;
			end
		end
	end
end

function secVal(r)
	local val = r.ToFood + r.ToProduction + r.ToGPT + r.Culture + r.ToScience + r.Faith + r.GoldenAgePoints + r.ScientificInsight + r.DiplomaticSupport + r.Tourism + r.ToPressure;
	return val;
end
function sortFallback(a,b)
	local va = secVal(a);
	local vb = secVal(b);
	
	return tonumber(va) > tonumber(vb);
end

function NumericSortFunction(field, direction, secondarySort)
	if(direction == "asc") then
		return function(a,b)
			local va = (a ~= nil and a[field] ~= nil) and a[field] or -1;
			local vb = (b ~= nil and b[field] ~= nil) and b[field] or -1;
			
			if(tonumber(va) == tonumber(vb)) then
				if (secondarySort ~= nil) then
					return secondarySort(a,b);
				else
					return sortFallback(a,b);
				end
			else
				return tonumber(va) < tonumber(vb);
			end
		end
	elseif(direction == "desc") then
		return function(a,b)
			local va = (a ~= nil and a[field] ~= nil) and a[field] or -1;
			local vb = (b ~= nil and b[field] ~= nil) and b[field] or -1;

			if(tonumber(va) == tonumber(vb)) then
				if (secondarySort ~= nil) then
					return secondarySort(a,b);
				else
					return sortFallback(a,b);
				end
			else
				return tonumber(vb) < tonumber(va);
			end
		end
	end
end

-- Registers the sort option controls click events
function RegisterSortOptions()
	for i,v in ipairs(g_SortOptions) do
		if(v.Button ~= nil) then
			v.Button:RegisterCallback(Mouse.eLClick, function() SortOptionSelected(v); end);
		else
			print("RegisterSortOptions button is null: " .. v.Column);
		end
	end
	
	g_SortFunction = GetSortFunction(g_SortOptions);
end

function GetSortFunction(sortOptions)
	local orderBy = nil;
	for i,v in ipairs(sortOptions) do
		if(v.CurrentDirection ~= nil) then
			local secondarySort = nil;
			if(v.SecondaryColumn ~= nil) then
				if(v.SecondarySortType == "numeric") then
					secondarySort = NumericSortFunction(v.SecondaryColumn, v.SecondaryDirection)
				else
					secondarySort = AlphabeticalSortFunction(v.SecondaryColumn, v.SecondaryDirection);
				end
			end
		
			if(v.SortType == "numeric") then
				return NumericSortFunction(v.Column, v.CurrentDirection, secondarySort);
			else
				return AlphabeticalSortFunction(v.Column, v.CurrentDirection, secondarySort);
			end
		end
	end
	
	return nil;
end

-- Updates the sort option structure
function UpdateSortOptionState(sortOptions, selectedOption)
	-- Current behavior is to only have 1 sort option enabled at a time 
	-- though the rest of the structure is built to support multiple in the future.
	-- If a sort option was selected that wasn't already selected, use the default 
	-- direction.  Otherwise, toggle to the other direction.
	for i,v in ipairs(sortOptions) do
		if(v == selectedOption) then
			if(v.CurrentDirection == nil) then			
				v.CurrentDirection = v.DefaultDirection;
			else
				if(v.CurrentDirection == "asc") then
					v.CurrentDirection = "desc";
				else
					v.CurrentDirection = "asc";
				end
			end
		else
			v.CurrentDirection = nil;
		end
	end
end

-- Callback for when sort options are selected.
function SortOptionSelected(option)
	local sortOptions = g_SortOptions;
	UpdateSortOptionState(sortOptions, option);
	g_SortFunction = GetSortFunction(sortOptions);
	
	SortData();
	DisplayData();
end

-------------------------------------------------
-------------------------------------------------
function OnPopupMessage(popupInfo)

	local popupType = popupInfo.Type;
	if popupType ~= ButtonPopupTypes.BUTTONPOPUP_TRADE_ROUTE_OVERVIEW then
		return;
	end
	
	g_PopupInfo = popupInfo;
	
	-- Data 2 parameter holds desired tab to open on
	if (g_PopupInfo.Data2 == 1) then
		g_CurrentTab = "YourTR";
	elseif (g_PopupInfo.Data2 == 2) then
		g_CurrentTab = "AvailableTR";
	elseif (g_PopupInfo.Data2 == 3) then
		g_CurrentTab = "TRWithYou";
	end
	
	if( g_PopupInfo.Data1 == 1 ) then
    	if( ContextPtr:IsHidden() == false ) then
    		OnClose();
		else
        	UIManager:QueuePopup( ContextPtr, PopupPriority.InGameUtmost );
    	end
	else
		UIManager:QueuePopup( ContextPtr, PopupPriority.SocialPolicy );
	end
end
Events.SerialEventGameMessagePopup.Add( OnPopupMessage );

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function IgnoreLeftClick( Id )
	-- just swallow it
end
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function InputHandler( uiMsg, wParam, lParam )
    ----------------------------------------------------------------        
    -- Key Down Processing
    ----------------------------------------------------------------        
    if(uiMsg == KeyEvents.KeyDown) then
        if (wParam == Keys.VK_ESCAPE) then
			OnClose();
			return true;
        end
        
        -- Do Nothing.
        if(wParam == Keys.VK_RETURN) then
			return true;
        end
    end
end
ContextPtr:SetInputHandler( InputHandler );
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function OnClose()
	UIManager:DequeuePopup(ContextPtr);
end
Controls.CloseButton:RegisterCallback(Mouse.eLClick, OnClose);
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function TabSelect(tab)
	for i,v in pairs(g_Tabs) do
		local bHide = i ~= tab;
		v.SelectHighlight:SetHide(bHide);
	end
	g_CurrentTab = tab;
	g_Tabs[tab].RefreshContent();	
end
Controls.TabButtonYourTR:RegisterCallback( Mouse.eLClick, function() TabSelect("YourTR"); end);
Controls.TabButtonAvailableTR:RegisterCallback( Mouse.eLClick, function() TabSelect("AvailableTR"); end );
Controls.TabButtonTRWithYou:RegisterCallback( Mouse.eLClick, function() TabSelect("TRWithYou"); end );

function RefreshYourTR()
	local pPlayer = Players[ Game.GetActivePlayer() ];
    
    --print("RefreshYourTR");
	SetData(pPlayer:GetTradeRoutes());
	SortData();
	DisplayData();
	
end
g_Tabs["YourTR"].RefreshContent = RefreshYourTR;

function RefreshAvailableTR()
	local pPlayer = Players[ Game.GetActivePlayer() ];

    SetData(pPlayer:GetTradeRoutesAvailable());
	SortData();
	DisplayData();
end
g_Tabs["AvailableTR"].RefreshContent = RefreshAvailableTR;

function RefreshTRWithYou()
	local pPlayer = Players[ Game.GetActivePlayer() ];
    
    SetData(pPlayer:GetTradeRoutesToYou());
    SortData();
    DisplayData();
    
end
g_Tabs["TRWithYou"].RefreshContent = RefreshTRWithYou;


function SetData(data)
	local cityStateCiv = GameInfo.Civilizations["CIVILIZATION_MINOR"];
	local cityStateCivID = cityStateCiv and cityStateCiv.ID or -1;

	function GetCivName(playerID)
		local civType = PreGame.GetCivilization(playerID);
		local civ = GameInfo.Civilizations[civType];
		if(civ.ID == cityStateCivID) then
			local minorCivPlayer = Players[playerID];
			local minorCivType = minorCivPlayer:GetMinorCivType();
			local minorCiv = GameInfo.MinorCivilizations[minorCivType];
		
			return Locale.Lookup(minorCiv.ShortDescription);
		else
			return Locale.Lookup(civ.Description);
		end
	end
	
	for i,v in ipairs(data) do
		v.FromCiv = GetCivName(v.FromID);
		v.ToCiv = GetCivName(v.ToID);
	end
	g_Data = data;
end

function SortData()
	table.sort(g_Data, g_SortFunction);
end

function round(num, idp)
  return tonumber(string.format("%." .. (idp or 0) .. "f", num))
end

function DisplayData()
	Controls.MainStack:DestroyAllChildren(); 
    for i,v in ipairs(g_Data) do
        local instance = {};
		ContextPtr:BuildInstanceForControl( "TRInstance", instance, Controls.MainStack );
	
		instance.Domain_Land:LocalizeAndSetToolTip("TXT_KEY_TRO_LAND_DOMAIN_TT"); 
		instance.Domain_Sea:LocalizeAndSetToolTip("TXT_KEY_TRO_SEA_DOMAIN_TT"); 	
	
		instance.Domain_Land:SetHide(true);
		instance.Domain_Sea:SetHide(true);
		
		if (v.FromCity == nil) then
			print("v.FromCity is nil");
		end
		
		if (v.ToCity == nil) then
			print("v.ToCity is nil");
		end
		
		local strTT = BuildTradeRouteToolTipString(Players[v.FromID], v.FromCity, v.ToCity, v.Domain);
		strTT = strTT or "";
		strTT = strTT .. "tooltip needs fixing";

		if (v.Domain == DomainTypes.DOMAIN_LAND) then
			instance.Domain_Land:SetHide(false);
			instance.Domain_Land:SetToolTipString(strTT);
		elseif (v.Domain == DomainTypes.DOMAIN_SEA) then
			instance.Domain_Sea:SetHide(false);
			instance.Domain_Sea:SetToolTipString(strTT);
		end
		
		CivIconHookup(v.FromID, 32, instance.FromCivIcon, instance.FromCivIconBG, instance.FromCivIconShadow, false, true, instance.FromCivIconHighlight);
		
		--Since I don't know which icon to use, use em all!
		instance.FromCivIcon:SetToolTipString(v.FromCiv);
		instance.FromCivIconBG:SetToolTipString(v.FromCiv);
		instance.FromCivIconShadow:SetToolTipString(v.FromCiv);
		instance.FromCivIconHighlight:SetToolTipString(v.FromCiv);
		instance.FromCity:SetText(v.FromCityName);
		instance.FromCity:SetToolTipString(v.FromCityName);
		
		CivIconHookup(v.ToID, 32, instance.ToCivIcon, instance.ToCivIconBG, instance.ToCivIconShadow, false, true, instance.ToCivIconHighlight);
		instance.ToCivIcon:SetToolTipString(v.ToCiv);
		instance.ToCivIconBG:SetToolTipString(v.ToCiv);
		instance.ToCivIconShadow:SetToolTipString(v.ToCiv);
		instance.ToCivIconHighlight:SetToolTipString(v.ToCiv);
		instance.ToCity:SetText(v.ToCityName);
		instance.ToCity:SetToolTipString(v.ToCityName);

		local routeInfoStr = v.Tooltip;
		local str;
		
		str = "";
		if (v.ToFood ~= 0) then
			str = round(v.ToFood / 100, 1);
			instance.ToFood:SetToolTipString(routeInfoStr);
		end
		instance.ToFood:SetText(str);
		
		str = "";
		if (v.ToProduction ~= 0) then
			str = round(v.ToProduction / 100, 1);
			instance.ToProduction:SetToolTipString(routeInfoStr);
		end
		instance.ToProduction:SetText(str);

		str = "";
		if (v.ToGPT ~= 0) then
			str = Locale.ConvertTextKey("TXT_KEY_TRO_GPT_ENTRY", v.ToGPT / 100);
			instance.ToGPT:SetToolTipString(routeInfoStr);
		end
		instance.ToGPT:SetText(str);


		
		str = "";			
		if(v.ToScience ~= 0) then
			str = v.ToScience / 100;
			instance.Science:SetToolTipString(routeInfoStr);
		end
		instance.Science:SetText(str);
		
		str = "";
		if (v.Culture ~= 0) then
			str = Locale.ConvertTextKey("TXT_KEY_TRO_GPT_ENTRY", v.Culture / 100);
			instance.Culture:SetToolTipString(routeInfoStr);
		end
		instance.Culture:SetText(str);
		
		str = "";
		if (v.Faith ~= 0) then
			str = Locale.ConvertTextKey("TXT_KEY_TRO_GPT_ENTRY", v.Faith / 100);
			instance.Faith:SetToolTipString(routeInfoStr);
		end
		instance.Faith:SetText(str);
		


		str = "";
		if (v.GoldenAgePoints ~= 0) then
			str = Locale.ConvertTextKey("TXT_KEY_TRO_GPT_ENTRY", v.GoldenAgePoints / 100);
		end
		instance.Golden:SetText(str);



		str = "";
		val = v.ScientificInsight;
		if (val ~= 0) then
			str = round(val / 100, 1);
		end
		instance.ScientificInsight:SetText(str);

		str = "";
		val = v.DiplomaticSupport;
		if (val ~= 0) then
			str = round(val / 100, 1);
		end
		instance.DiplomaticSupport:SetText(str);

		str = "";
		val = v.Tourism;
		if (val ~= 0) then
			str = round(val / 100, 1);
		end
		instance.CulturalInfluence:SetText(str);


		-- Religion
		local strToPressure = "";
		if (v.ToReligion > 0 and v.ToPressure ~= 0) then
			local religion = GameInfo.Religions[v.ToReligion];
			local strIcon = religion.IconString;
			strToPressure = Locale.ConvertTextKey("TXT_KEY_TRO_RELIGIOUS_PRESSURE_ENTRY", strIcon, v.ToPressure);
			instance.ToReligion:SetToolTipString(strTT);	
		end
		instance.ToReligion:SetText(strToPressure);
		
		local strFromPressure = "";
		if (v.FromReligion > 0 and v.FromPressure ~= 0) then
			local religion = GameInfo.Religions[v.FromReligion];
			local strIcon = religion.IconString;
			strFromPressure = Locale.ConvertTextKey("TXT_KEY_TRO_RELIGIOUS_PRESSURE_ENTRY", strIcon, v.FromPressure);
			instance.FromReligion:SetToolTipString(strTT);	
		end
		instance.FromReligion:SetText(strFromPressure);


		-- Turns Remaining
		local color = "[COLOR_WHITE]";
		strTT = "This trade route would take this many turns.";
		local strTurnsRemaining = "";
		local val = v.TurnsLeft or 0;
		if (val >= 0) then
			color = "[COLOR_YELLOW]";
			strTT = "This trade route is active and has this many turns remaining.";
		end		
		strTurnsRemaining = color .. math.abs(val) .. "[ENDCOLOR]";
		instance.TurnsLeft:SetToolTipString(strTT);
		instance.TurnsLeft:SetText(strTurnsRemaining);
    end
    
    Controls.MainStack:CalculateSize();
    Controls.MainStack:ReprocessAnchoring();
    Controls.MainScroll:CalculateInternalSize();

end

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function ShowHideHandler( bIsHide, bInitState )

	-- Set Civ Icon
	CivIconHookup( Game.GetActivePlayer(), 64, Controls.CivIcon, Controls.CivIconBG, Controls.CivIconShadow, false, true );

    if( not bInitState ) then
        if( not bIsHide ) then
        	UI.incTurnTimerSemaphore();  
        	Events.SerialEventGameMessagePopupShown(g_PopupInfo);
        	
        	TabSelect(g_CurrentTab);
        else
			if(g_PopupInfo ~= nil) then
				Events.SerialEventGameMessagePopupProcessed.CallImmediate(g_PopupInfo.Type, 0);
            end
            UI.decTurnTimerSemaphore();
        end
    end
end
ContextPtr:SetShowHideHandler( ShowHideHandler );

RegisterSortOptions();
TabSelect("YourTR");