-- CONFIRM CITY TASK POPUP
-- This popup occurs when an action needs confirmation.

PopupLayouts[ButtonPopupTypes.BUTTONPOPUP_CONFIRM_CITY_TASK] = function(popupInfo)

	local iCityID	= popupInfo.Data1;
	local iTask	= popupInfo.Data2;
	
	local popupText = "NO POPUP TEXT DEFINED";
	if (iTask == TaskTypes.TASK_RAZE) then
		popupText = "TXT_KEY_POPUP_ARE_YOU_SURE_RAZE";
	elseif (popupInfo.Text ~= nil) then
		popupText = popupInfo.Text;
	end
	
	SetPopupText(Locale.ConvertTextKey(popupText));
		
	-- Initialize 'yes' button.
	local OnYesClicked = function()
		
		-- Confirm action
		Network.SendDoTask(iCityID, iTask, -1, -1, false, false, false, false);
	end
	
	local buttonText = Locale.ConvertTextKey("TXT_KEY_POPUP_YES");
	AddButton(buttonText, OnYesClicked)
		
	-- Initialize 'no' button.
	local buttonText = Locale.ConvertTextKey("TXT_KEY_POPUP_NO");
	AddButton(buttonText, nil);

end