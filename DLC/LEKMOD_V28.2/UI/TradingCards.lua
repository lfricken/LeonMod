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

g_Tab1 = "YourTR";
g_Tab2 = "AvailableTR";
g_Tab3 = "TRWithYou";
-------------------------------------------------
-- Global Constants
-------------------------------------------------
g_Tabs = {
	[g_Tab1] = {
		SelectHighlight = Controls.YourTRSelectHighlight,
	},
	
	[g_Tab2] = {
		SelectHighlight = Controls.AvailableTRSelectHighlight,
	},
	
	[g_Tab3] = {
		SelectHighlight = Controls.TRWithYouSelectHighlight,
	},
}

-------------------------------------------------
-------------------------------------------------
function OnPopupMessage(popupInfo)

	local popupType = popupInfo.Type;
	if popupType ~= 7091 then
		return;
	end
	
	g_PopupInfo = popupInfo;
	
	-- Data 2 parameter holds desired tab to open on
	if (g_PopupInfo.Data2 == 1) then
		g_CurrentTab = g_Tab1;
	elseif (g_PopupInfo.Data2 == 2) then
		g_CurrentTab = g_Tab2;
	elseif (g_PopupInfo.Data2 == 3) then
		g_CurrentTab = g_Tab3;
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
Controls.TabButtonYourTR:RegisterCallback( Mouse.eLClick, function() TabSelect(g_Tab1); end);
Controls.TabButtonAvailableTR:RegisterCallback( Mouse.eLClick, function() TabSelect(g_Tab2); end );
Controls.TabButtonTRWithYou:RegisterCallback( Mouse.eLClick, function() TabSelect(g_Tab3); end );

function RefreshTab1()
	DisplayData();
end
g_Tabs[g_Tab1].RefreshContent = RefreshTab1;

function RefreshTab2()
	DisplayData();
end
g_Tabs[g_Tab2].RefreshContent = RefreshTab2;

function RefreshTab3()
	DisplayData();
end
g_Tabs[g_Tab3].RefreshContent = RefreshTab3;

local function isempty(s)
  return s == nil or s == ''
end

function DisplayData()
	Controls.MainStack:DestroyAllChildren();
	local iPlayerId = Game.GetActivePlayer();
    local pPlayer = Players[iPlayerId];

	local count = pPlayer:CardCount();
	local isDebugMode = true; -- allows card manipulation

	for cardIdx = 0, count-1, 1 do
		local inst = {};
		ContextPtr:BuildInstanceForControl("TradeCardInstance", inst, Controls.MainStack);

		local cardName = pPlayer:CardName(cardIdx);
		inst.Name:SetText(cardName);
		inst.Name:SetToolTipString(cardName);

		local cardDesc = pPlayer:CardDesc(cardIdx);
		inst.Desc:SetText(cardDesc);
		inst.Desc:SetToolTipString(cardDesc);

		local passiveDesc = pPlayer:CardPassiveDesc(cardIdx);
		local hasPassive = not isempty(passiveDesc);
		inst.Passive:SetHide(not hasPassive);
		inst.Passive:SetToolTipString(passiveDesc);

		local activeDesc = pPlayer:CardActiveDesc(cardIdx);
		local hasActive = not isempty(activeDesc);
		inst.Activate:SetHide(not hasActive);
		inst.Activate:SetToolTipString(activeDesc);

		--inst.Activate:SetHide(not isPassive);
		--inst.Activate:SetDisabled(not isPassive);

		inst.Activate:RegisterCallback(Mouse.eLClick, function() OnClickedActivate(iPlayerId, cardIdx); end);

		inst.Delete:SetHide(not isDebugMode);
		inst.Delete:SetDisabled(not isDebugMode);
		inst.Delete:RegisterCallback(Mouse.eLClick, function() OnClickedDelete(iPlayerId, cardIdx); end);
	end
	
	Controls.MainStack:CalculateSize();
	Controls.MainStack:ReprocessAnchoring();
	Controls.MainScroll:CalculateInternalSize();

end

function OnClickedActivate(iPlayerId, cardIdx)
    local pPlayer = Players[iPlayerId];
    pPlayer:CardActivate(cardIdx);
end

-- DEBUG ONLY, NOT NETWORK SAFE
function OnClickedDelete(iPlayerId, cardIdx)
    local pPlayer = Players[iPlayerId];
    pPlayer:CardDelete(cardIdx);
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

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Update on dirty bits
function OnDirtyBit()
	g_Tabs[g_CurrentTab].RefreshContent();
end
-- Register Events
Events.SerialEventGreatWorksScreenDirty.Add(OnDirtyBit);

TabSelect(g_Tab1);
