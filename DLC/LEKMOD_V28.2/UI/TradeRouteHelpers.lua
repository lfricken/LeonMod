
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function BuildTradeRouteToolTipString (pPlayer, pOriginCity, pTargetCity, eDomain, conType)
	local strResult = pPlayer:GetTradeRouteToolTip(pOriginCity, pTargetCity, eDomain, false, conType);
	
	return strResult;
end
