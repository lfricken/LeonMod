


-- Add a button to popup.
-- NOTE: Current Limitation is 4 buttons
function AddButton(buttonText, buttonClickFunc, strToolTip, bPreventClose, bDisable)
	local i = 1;
	repeat
		local button = Controls["Button"..i];
		if button and button:IsHidden() then
			local buttonLabel = Controls["Button"..i.."Text"];
			buttonLabel:SetText( buttonText );
			
			button:SetToolTipString(strToolTip);

			--By default, button clicks will hide the popup window after
			--executing the click function
			local clickHandler = function()
				if buttonClickFunc ~= nil then
					buttonClickFunc();
				end
				
				HideWindow();
			end
			local clickHandlerPreventClose = function()
				if buttonClickFunc ~= nil then
					buttonClickFunc();
				end
			end
			
			-- This is only used in one case, when viewing a captured city (PuppetCityPopup)
			if (bPreventClose) then
				button:RegisterCallback(Mouse.eLClick, clickHandlerPreventClose);
			else
				button:RegisterCallback(Mouse.eLClick, clickHandler);
			end
			
			button:SetHide(false);
			button:SetDisabled(bDisable);
			
			return;
		end
		i = i + 1;
	until button == nil;
end

-- ANNEX CITY POPUP
-- This popup occurs when a player clicks on a puppeted City
PopupLayouts[ButtonPopupTypes.BUTTONPOPUP_ANNEX_CITY] = function(popupInfo)
	
	local cityID				= popupInfo.Data1;
	
	local activePlayer	= Players[Game.GetActivePlayer()];
	local newCity		= activePlayer:GetCityByID(cityID);
	
	-- Initialize popup text.	
	local cityNameKey = newCity:GetNameKey();
	popupText = Locale.ConvertTextKey("TXT_KEY_POPUP_ANNEX_PUPPET", cityNameKey);
	
	SetPopupText(popupText);
	
	-- Initialize 'Annex City' button.
	local OnAnnexClicked = function()
		Network.SendDoTask(cityID, TaskTypes.TASK_ANNEX_PUPPET, -1, -1, false, false, false, false);
	end
	
	local buttonText = Locale.ConvertTextKey("TXT_KEY_POPUP_ANNEX_CITY");
	local maxTurns = 3;

	local turnsTill = newCity:GetTurnsTillCanDoMajorTask();
	if (turnsTill > 0) then
		local turnsRemainStr = Locale.ConvertTextKey("TXT_KEY_CITYVIEW_MAJORTASK_LIMIT", maxTurns, turnsTill);
		AddButton(buttonText, OnAnnexClicked, turnsRemainStr, false, true);
	else
		local hoverText = Locale.ConvertTextKey("TXT_KEY_CITYVIEW_ADDENDUM", maxTurns);
		AddButton(buttonText, OnAnnexClicked, hoverText);
	end
	
	-- Initialize 'Leave City as a Puppet' button.
	--local OnNoClicked = function()
		--newCity:ChooseProduction();
	--end
	
	local buttonText = Locale.ConvertTextKey("TXT_KEY_POPUP_DONT_ANNEX_CITY");
	AddButton(buttonText);
	
	-- Initialize 'View City' button.
	local OnViewCityClicked = function()
		UI.SetCityScreenViewingMode(true);
		UI.DoSelectCityAtPlot( newCity:Plot() );
	end
	
	buttonText = Locale.ConvertTextKey("TXT_KEY_POPUP_VIEW_CITY");
	strToolTip = Locale.ConvertTextKey("TXT_KEY_POPUP_VIEW_CITY_DETAILS");
	AddButton(buttonText, OnViewCityClicked, strToolTip);
	
	Controls.CloseButton:SetHide( false );

end

----------------------------------------------------------------        
-- Key Down Processing
----------------------------------------------------------------        
PopupInputHandlers[ButtonPopupTypes.BUTTONPOPUP_ANNEX_CITY] = function( uiMsg, wParam, lParam )
    if uiMsg == KeyEvents.KeyDown then
        if( wParam == Keys.VK_ESCAPE or wParam == Keys.VK_RETURN ) then
			HideWindow();
            return true;
        end
    end
end

----------------------------------------------------------------
-- 'Active' (local human) player has changed
----------------------------------------------------------------
Events.GameplaySetActivePlayer.Add(HideWindow);
