-------------------------------------------------
-- Trade Card Popup
-------------------------------------------------
include( "IconSupport" );
include( "InstanceManager" );
include( "TradeRouteHelpers" );

local g_PopupInfo = nil; -- info we were launched with
local g_isDebugMode = true; -- allows card manipulation

g_CurrentTab = Game.GetActivePlayer();		-- The currently selected Tab.
g_Tabs = {};

function OnPopupMessage(popupInfo)

	local popupType = popupInfo.Type;
	if popupType ~= 7091 then
		return;
	end
	
	g_PopupInfo = popupInfo;
	
	-- Data 2 parameter holds desired tab to open on
	g_CurrentTab = Game.GetActivePlayer();
	--[[
	if (g_PopupInfo.Data2 == 1) then
		g_CurrentTab = g_Tab1;
	elseif (g_PopupInfo.Data2 == 2) then
		g_CurrentTab = g_Tab2;
	elseif (g_PopupInfo.Data2 == 3) then
		g_CurrentTab = g_Tab3;
	end
	--]]

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

function IgnoreLeftClick( Id )
	-- just swallow it
end

-- CLOSE BUTTON
function InputHandler( uiMsg, wParam, lParam )	
	if(uiMsg == KeyEvents.KeyDown) then
		if (wParam == Keys.VK_ESCAPE) then
			OnClose();
			return true;
		end
		if(wParam == Keys.VK_RETURN) then
			return true;
		end
	end
end
ContextPtr:SetInputHandler( InputHandler );
function OnClose()
	UIManager:DequeuePopup(ContextPtr);
end
Controls.CloseButton:RegisterCallback(Mouse.eLClick, OnClose);

-- TAB SELECT
function TabSelect(tab)
	for i,v in pairs(g_Tabs) do
		local bHide = i ~= tab;
		v.SelectHighlight:SetHide(bHide);
	end
	print("" .. tab);
	g_CurrentTab = tab;
	g_Tabs[tab].RefreshContent();
end

-- CREATE TABS
local pid = Game.GetActivePlayer();
local pLocal = Players[pid];
for pid=0,16,1 do
	local pPlayer = Players[pid];
	if (pPlayer:IsAlive()) then
		local instTab = {};
		ContextPtr:BuildInstanceForControl("PlayerCardTab", instTab, Controls.Tabs);
		tabObj = {};
		tabObj.SelectHighlight = instTab.Highlight;
		-- refresh
		tabObj.RefreshContent = function() DisplayData(pid, true, true); end
		-- click on tab
		instTab.Button:SetText("" .. 1 + pid);
		instTab.Button:RegisterCallback( Mouse.eLClick, function() TabSelect(pid); end);
		instTab.Button:SetToolTipString(pLocal:GetCivNameSafe(pid));
		table.insert(g_Tabs, pid, tabObj);
	end
end

-- DISPLAY CARDS
local function isempty(s)
  return s == nil or s == ''
end
function DisplayData(iPlayerId, includePassive, includeActive)
	Controls.MainStack:DestroyAllChildren();
	local pPlayer = Players[iPlayerId];
	local bLocal = iPlayerId == Game.GetActivePlayer();

	local count = pPlayer:CardCount();
	for cardIdx = 0, count-1, 1 do
		local cardType = pPlayer:CardName(cardIdx);

		local cardName = pPlayer:CardName(cardType);
		local cardDesc = pPlayer:CardDesc(cardType);
		local passiveDesc = pPlayer:CardPassiveDesc(cardType);
		local hasPassive = not isempty(passiveDesc);
		local activeDesc = pPlayer:CardActiveDesc(cardType);
		local hasActive = not isempty(activeDesc);
		local isVisible = pPlayer:CardIsVisible(cardIdx);

		local shown = (hasPassive and includePassive) or (hasActive and includeActive);
		local hide = not bLocal and not isVisible;
		-- should we limit the display of the cards?
		if (not hide) then
			local inst = {};
			ContextPtr:BuildInstanceForControl("TradeCardInstance", inst, Controls.MainStack);

			inst.Name:SetText(cardName);
			inst.Name:SetToolTipString(cardName);

			inst.Desc:SetText(cardDesc);
			inst.Desc:SetToolTipString(cardDesc);

			-- passive info
			inst.Passive:SetHide(not hasPassive);
			inst.Passive:SetToolTipString(passiveDesc);

			-- activate button
			inst.Activate:SetDisabled(not bLocal);
			if (not bLocal) then
				inst.ActivateLabel:SetColorByName("Gray_Black");
			end
			inst.Activate:SetHide(not hasActive);
			inst.Activate:LocalizeAndSetToolTip(activeDesc .. ". {TXT_KEY_CARD_ACTIVATE_CONSUMED}");
			inst.Activate:RegisterCallback(Mouse.eLClick, function() OnClickedActivate(iPlayerId, cardIdx); end);

			-- visibility button
			inst.VisibilityToggle:RegisterCallback(Mouse.eLClick, function() OnClickedVisibility(iPlayerId, cardIdx); end);
			if (not isVisible) then
				inst.VisibilityToggle:LocalizeAndSetText("TXT_KEY_CARD_VISIBILITY_BUTTON_ON");
				inst.VisibilityToggle:LocalizeAndSetToolTip("TXT_KEY_CARD_VISIBILITY_BUTTON_ON_TIP");
			end
			inst.VisibilityToggle:SetDisabled(not bLocal);
			inst.VisibilityToggle:SetHide(not bLocal);

			-- delete button (DEBUG ONLY)
			inst.Delete:SetHide(not g_isDebugMode);
			inst.Delete:SetDisabled(not g_isDebugMode);
			inst.Delete:RegisterCallback(Mouse.eLClick, function() OnClickedDelete(iPlayerId, cardIdx); end);
		end
	end
	
	Controls.MainStack:CalculateSize();
	Controls.MainStack:ReprocessAnchoring();
	Controls.MainScroll:CalculateInternalSize();

end
function OnClickedActivate(iPlayerId, cardIdx)
	local pPlayer = Players[iPlayerId];
	pPlayer:CardActivate(cardIdx);
end
function OnClickedVisibility(iPlayerId, cardIdx)
	local pPlayer = Players[iPlayerId];
	pPlayer:CardToggleVisibility(cardIdx);
end
-- DEBUG ONLY, NOT NETWORK SAFE
function OnClickedDelete(iPlayerId, cardIdx)
	local pPlayer = Players[iPlayerId];
	pPlayer:CardDelete(cardIdx);
end

-- REFRESH
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

function OnDirtyBit() -- Update on dirty bits
	g_Tabs[g_CurrentTab].RefreshContent();
end
-- Register Events
Events.SerialEventGreatWorksScreenDirty.Add(OnDirtyBit);

TabSelect(g_CurrentTab); -- default to this player
