----------------------------------------------------------------
----------------------------------------------------------------
include( "IconSupport" );
include( "SupportFunctions"  );
include( "InstanceManager" );

local doneString = " [ICON_TROPHY_GOLD] [COLOR_POSITIVE_TEXT]DONE[ENDCOLOR]";

-- Domination Variables
local g_DominationRowsIM = InstanceManager:new( "DominationRow", "RowStack", Controls.DominationStack );
local g_DominationRowList = {};
local g_DominationItemIMList = {};
local g_DominationItemList = {};

MAX_DOMINATION_ICONS_PER_ROW = 8;

-- Tech Variables
local g_TechPreReqList = {};
local g_PreReqsAcquired = 0;

-- Tech Details Variables
local g_TechIM = InstanceManager:new( "TechCiv", "Civ", Controls.TechRaceStack );
local g_TechList = {};

-- Tech
local g_TechRowsIM = InstanceManager:new( "TechRow", "RowStack", Controls.TechStack );
local g_TechRowList = {};
local g_TechItemIMList = {};
local g_TechItemList = {};

-- Diplo Details Variables
local g_DiploRowsIM = InstanceManager:new( "DiploRow", "RowStack", Controls.DiploStack );
local g_DiploRowList = {};
local g_DiploItemIMList = {};
local g_DiploItemList = {};

-- Cultural Details Variables
local g_CultureRowsIM = InstanceManager:new( "CultureRow", "RowStack", Controls.CultureStack );
local g_CultureRowList = {};
local g_CultureItemIMList = {};
local g_CultureItemList = {};

MAX_CULTURE_ICONS_PER_ROW = 8;

-- Score Details Variables
local g_ScoreIM = InstanceManager:new( "ScoreCiv", "Civ", Controls.ScoreStack );
local g_ScoreList = {};

local g_VictorBoxSizeX = 850;
local g_VictorBoxSizeY = 250;
local g_VictorTrimSizeX = 860;
local g_LabelContainerSizeX = 400;
local white = {x = 1, y = 1, z = 1, w = 1};
local gray = {x = 0.50, y = 0.50, z = 0.50, w = 1.00};

----------------------------------------------------------------
----------------------------------------------------------------
function OnBack()
	Controls.YourDetails:SetHide(false);
	Controls.ScoreScreen:SetHide(true);
	Controls.SpaceRaceScreen:SetHide(true);
	Controls.DiploScreen:SetHide(true);
			
	
	-- Delete instanced screens
	DeleteSpaceRaceScreen();
	DeleteTechScreen();
	DeleteDiploScreen();

	DeleteScoreScreen();
	
	UIManager:DequeuePopup( ContextPtr );
end
Controls.BackButton:RegisterCallback( Mouse.eLClick, OnBack );

----------------------------------------------------------------
----------------------------------------------------------------
function OnSpaceRaceDetails()
	Controls.YourDetails:SetHide(true);
	Controls.SpaceRaceScreen:SetHide(false);
	Controls.BackButton:SetHide(true);
	Controls.SpaceRaceClose:SetHide(false);
	
	-- Populate Detail Screens
	PopulateSpaceRaceScreen();
end
Controls.SpaceRaceDetails:RegisterCallback( Mouse.eLClick, OnSpaceRaceDetails );
----------------------------------------------------------------
----------------------------------------------------------------
function OnScoreDetails()
	Controls.YourDetails:SetHide(true);
	Controls.ScoreScreen:SetHide(true);
	Controls.SpaceRaceScreen:SetHide(true);
	Controls.DiploScreen:SetHide(true);
	Controls.ScoreScreen:SetHide(false);
	Controls.BackButton:SetHide(true);
	
	Controls.ScoreClose:SetHide(false);
	Controls.DiploClose:SetHide(true);
	Controls.SpaceRaceClose:SetHide(true);
	
	-- Populate Detail Screens
	PopulateScoreScreen();
end
Controls.ScoreDetails:RegisterCallback( Mouse.eLClick, OnScoreDetails );

----------------------------------------------------------------
----------------------------------------------------------------
function OnSpaceRaceClose()
	Controls.YourDetails:SetHide(false);
	Controls.SpaceRaceScreen:SetHide(true);
	Controls.BackButton:SetHide(false);
	Controls.SpaceRaceClose:SetHide(true);
	
	-- Delete instanced parts of culture screen
	DeleteSpaceRaceScreen();
end
Controls.SpaceRaceClose:RegisterCallback( Mouse.eLClick, OnSpaceRaceClose );

----------------------------------------------------------------
----------------------------------------------------------------
function OnDiploClose()
	Controls.YourDetails:SetHide(false);
	Controls.DiploScreen:SetHide(true);
	Controls.BackButton:SetHide(false);
	Controls.DiploClose:SetHide(true);
	
	-- Delete instanced parts of culture screen
	DeleteDiploScreen();
end
Controls.DiploClose:RegisterCallback( Mouse.eLClick, OnDiploClose );

----------------------------------------------------------------
----------------------------------------------------------------
function OnScoreClose()
	Controls.YourDetails:SetHide(false);
	Controls.ScoreScreen:SetHide(true);
	Controls.BackButton:SetHide(false);
	Controls.ScoreClose:SetHide(true);
	
	-- Delete instanced parts of culture screen
	DeleteScoreScreen();
end
Controls.ScoreClose:RegisterCallback( Mouse.eLClick, OnScoreClose );


function PopulateScoreBreakdown()
	local pPlayer = Players[Game.GetActivePlayer()];
	local bScenario = PreGame.GetLoadWBScenario();
	Controls.ScoreDetails:SetHide(true); -- hide score details
	
	if(PreGame.IsVictory(GameInfo.Victories["VICTORY_TIME"].ID))then
		Controls.Cities:SetText(pPlayer:GetScoreFromCities());
		Controls.Population:SetText(pPlayer:GetScoreFromPopulation());
		Controls.Land:SetText(pPlayer:GetScoreFromLand());
		Controls.Wonders:SetText(pPlayer:GetScoreFromWonders());
		if (not Game.IsOption(GameOptionTypes.GAMEOPTION_NO_SCIENCE)) then
			Controls.Tech:SetText(pPlayer:GetScoreFromTechs());
			Controls.FutureTech:SetText(pPlayer:GetScoreFromFutureTech());
			Controls.TechLabel:SetHide(false);
			Controls.FutureTechLabel:SetHide(false);
			Controls.TechDetailsBox:SetHide(false);
			Controls.FutureTechBox:SetHide(false);
			Controls.Tech:SetHide(false);
			Controls.FutureTech:SetHide(false);
		else
			Controls.TechLabel:SetHide(true);
			Controls.FutureTechLabel:SetHide(true);
			Controls.TechDetailsBox:SetHide(true);
			Controls.FutureTechBox:SetHide(true);
			Controls.Tech:SetHide(true);
			Controls.FutureTech:SetHide(true);
		end
		if (not Game.IsOption(GameOptionTypes.GAMEOPTION_NO_POLICIES)) then
			Controls.Policies:SetText(pPlayer:GetScoreFromPolicies());
			Controls.PolicyLabel:SetHide(false);
			Controls.PolicyBox:SetHide(false);
			Controls.Policies:SetHide(false);
		else
			Controls.PolicyLabel:SetHide(true);
			Controls.PolicyBox:SetHide(true);
			Controls.Policies:SetHide(true);
		end
		Controls.GreatWorks:SetText(pPlayer:GetScoreFromGreatWorks());
		if (not Game.IsOption(GameOptionTypes.GAMEOPTION_NO_RELIGION)) then
			Controls.Religion:SetText(pPlayer:GetScoreFromReligion());
			Controls.ReligionLabel:SetHide(false);
			Controls.ReligionBox:SetHide(false);
			Controls.Religion:SetHide(false);
		else
			Controls.ReligionLabel:SetHide(true);
			Controls.ReligionBox:SetHide(true);
			Controls.Religion:SetHide(true);
		end
		if (bScenario == true) then
			Controls.Scenario1:SetText(pPlayer:GetScoreFromScenario1());
			Controls.Scenario2:SetText(pPlayer:GetScoreFromScenario2());
			Controls.Scenario3:SetText(pPlayer:GetScoreFromScenario3());
			Controls.Scenario4:SetText(pPlayer:GetScoreFromScenario4());
			Controls.Scenario1:SetHide(false);
			Controls.Scenario2:SetHide(false);
			Controls.Scenario3:SetHide(false);
			Controls.Scenario4:SetHide(false);		
			Controls.ScenarioBox1:SetHide(false);
			Controls.ScenarioBox2:SetHide(false);
			Controls.ScenarioBox3:SetHide(false);
			Controls.ScenarioBox4:SetHide(false);		
			Controls.VPLabel1:SetHide(false);		
			Controls.VPLabel2:SetHide(false);		
			Controls.VPLabel3:SetHide(false);		
			Controls.VPLabel4:SetHide(false);		
		else
			Controls.Scenario1:SetHide(true);
			Controls.Scenario2:SetHide(true);
			Controls.Scenario3:SetHide(true);
			Controls.Scenario4:SetHide(true);
			Controls.ScenarioBox1:SetHide(true);
			Controls.ScenarioBox2:SetHide(true);
			Controls.ScenarioBox3:SetHide(true);
			Controls.ScenarioBox4:SetHide(true);		
			Controls.VPLabel1:SetHide(true);		
			Controls.VPLabel2:SetHide(true);		
			Controls.VPLabel3:SetHide(true);		
			Controls.VPLabel4:SetHide(true);		
		end
		Controls.Score:SetText(pPlayer:GetScore());
		Controls.ScoreDetailsStack:CalculateSize();
		Controls.ScoreDetailsStack:ReprocessAnchoring();
		Controls.ScoreLabelStack:CalculateSize();
		Controls.ScoreLabelStack:ReprocessAnchoring();
		
		Controls.TimeVictoryDisabledBox:SetHide(true);
		
	else
		Controls.ScoreDetails:SetHide(true);
		Controls.TimeStack:SetHide(true);
		
		Controls.TimeVictoryDisabledBox:SetHide(false);
	end
end

----------------------------------------------------------------
----------------------------------------------------------------
function PopulateDomination()
		
	if(PreGame.IsVictory(GameInfo.Victories["VICTORY_DOMINATION"].ID))then		
		for i, v in pairs(g_DominationItemIMList) do
			v:ResetInstances();
		end
		g_DominationRowsIM:ResetInstances();
		
		g_DominationRowList = {};
		g_DominationItemList = {};
		g_DominationItemIMList = {};
		
		local curRow = nil;
		local numRows = 0;
		local curItemIM = nil;
		local numItemIM = 0;
		local curCol = 0;
		local numItems = 0;
		local numCapitals = 0;
		
		local iLeadingNumCapitals = 0;
		local iLeadingTeam = -1;
		local iLeadingPlayer = -1;
		local iLeadingNumPlayersOnTeam = 0;
		local bAnyoneLostControlOfCapital = false;
		
		local aiCapitalOwner = {};
		
		print("GameDefines.MAX_CIV_PLAYERS: " .. GameDefines.MAX_MAJOR_CIVS);
		
		for iTeamLoop = 0, GameDefines.MAX_CIV_TEAMS-1, 1 do
			-- Loop through all the civs the active player knows
			local iNumCapitals = 0;
			local iNumPlayersOnTeam = 0;
			local iPlayerOnTeam = -1;
			
			for iPlayerLoop = 0, GameDefines.MAX_MAJOR_CIVS-1, 1 do
				local pPlayer = Players[iPlayerLoop];
				if (not pPlayer:IsMinorCiv() and pPlayer:IsEverAlive() and pPlayer:GetTeam() == iTeamLoop) then
					if (pPlayer:IsHasLostCapital()) then
						bAnyoneLostControlOfCapital = true;
					end
				
					iPlayerOnTeam = iPlayerLoop;
					iNumPlayersOnTeam = iNumPlayersOnTeam + 1;

					for pCity in pPlayer:Cities() do
						if (pCity:IsOriginalMajorCapital()) then
						
							iNumCapitals = iNumCapitals + 1;
							aiCapitalOwner[pCity:GetOriginalOwner()] = iPlayerLoop;
						end
					end
				end
			end
			
			if (iNumCapitals - iNumPlayersOnTeam > iLeadingNumCapitals - iLeadingNumPlayersOnTeam) then
				iLeadingTeam = iTeamLoop;
				iLeadingNumCapitals = iNumCapitals;
				iLeadingNumPlayersOnTeam = iNumPlayersOnTeam;
				iLeadingPlayer = iPlayerOnTeam;
			end
		end
		
		-- Loop through all the civs the active player knows
		for iPlayerLoop = 0, GameDefines.MAX_CIV_PLAYERS-1, 1 do

			local pPlayer = Players[iPlayerLoop];
			
			if(not pPlayer:IsMinorCiv() and pPlayer:IsEverAlive()) then
				-- Create new row if one does not alread exist.
				if(curRow == nil) then
					numRows = numRows + 1;
					curRow = g_DominationRowsIM:GetInstance();
					g_DominationRowList[numRows] = curRow;
					
					numItemIM = numItemIM + 1;
					curItemIM = InstanceManager:new( "DominationItem", "IconFrame", curRow.RowStack );
					g_DominationItemIMList[numItemIM] = curItemIM;
					
				end
				
				local dominatingPlayer = Players[aiCapitalOwner[iPlayerLoop]];
				local newNumItems = AddDominationCiv(pPlayer, dominatingPlayer, numItems, curItemIM);
				
				if(newNumItems ~= numItems) then
					numItems = newNumItems;				
					curCol = curCol + 1;
					if(curCol >= MAX_DOMINATION_ICONS_PER_ROW) then
						curRow = nil;
						curCol = 0;
					end
				end
			end
		end
		
		local pActivePlayer = Players[Game.GetActivePlayer()];
		local pActiveTeam = Teams[pActivePlayer:GetTeam()];
		
		if (iLeadingTeam == -1) then
			if (bAnyoneLostControlOfCapital) then
				--print("Hiding string because it is a weird circumstance");
				Controls.DominationLabel:SetHide(true);
			else
				--print("Everybody has their own capital.");
				Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_NEW_CAPITALS_REMAINING"));	
				Controls.DominationLabel:SetHide(false);
			end
		elseif (iLeadingNumPlayersOnTeam > 1) then
			--print("A team is winning");
			Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_TEAM_LEADING", iLeadingTeam + 1, iLeadingNumCapitals));	
			Controls.DominationLabel:SetHide(false);		
		else
			if (iLeadingPlayer == Game.GetActivePlayer()) then
				--print("The current player is winning");
				Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_ACTIVE_PLAYER_LEADING", iLeadingNumCapitals));
			else
				--print("Some other player is winning");
				local pOtherPlayer = Players[iLeadingPlayer];			
				if (pOtherPlayer:GetNickName() ~= "" and pOtherPlayer:IsHuman()) then
					Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_PLAYER_LEADING", pOtherPlayer:GetNickName(), iLeadingNumCapitals));
				elseif (not pActiveTeam:IsHasMet(iLeadingTeam)) then
					Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_UNMET_PLAYER_LEADING", iLeadingNumCapitals));
				else
					Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_PLAYER_LEADING", pOtherPlayer:GetName(), iLeadingNumCapitals));
    			end
			end
			Controls.DominationLabel:SetHide(false);	
		end
		
		--Controls.DominationLabel:SetText(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_CAPITALS_REMAINING",  numCapitals));
			
		Controls.DominationStack:CalculateSize();
		Controls.DominationStack:ReprocessAnchoring();
		Controls.DominationScrollPanel:CalculateInternalSize();
		
		Controls.DominationVictoryProgress:SetHide(false);
		Controls.DominationDisabledLabel:SetHide(true);
	else
		Controls.DominationVictoryProgress:SetHide(true);
		Controls.DominationDisabledLabel:SetHide(false);
	end
end

----------------------------------------------------------------
----------------------------------------------------------------
function AddDominationCiv(pPlayer, pDominatingPlayer, numItems, civMgr)
	
	if(pPlayer:IsEverAlive()) then
		local item = civMgr:GetInstance();
		g_DominationItemList[numItems] = item;
		numItems = numItems + 1;
		
		local iPlayer = pPlayer:GetID();
		-- Set Civ Icon
		SetConquestCivIcon(pPlayer, 64, item.Icon, item.IconFrame,
				   item.ConqueredCivSmallFrame, item.ConqueredCivIconBG, item.ConqueredCivIconShadow, item.ConqueredCivIcon, item.IconOut, pDominatingPlayer,
				   item.CivIconBG, item.CivIconShadow); 

        local pTeam = Teams[ pPlayer:GetTeam() ];
        if( pTeam:GetNumMembers() > 1 ) then
    	    item.TeamID:LocalizeAndSetText( "TXT_KEY_MULTIPLAYER_DEFAULT_TEAM_NAME", pTeam:GetID() + 1 );
    	    item.TeamID:SetHide( false );
	    else
    	    item.TeamID:SetHide( true );
	    end
	end
	return numItems;
end 

----------------------------------------------------------------
----------------------------------------------------------------
function PopulateSpaceRace()
	Controls.TechBox:SetSizeX(g_VictorBoxSizeX);
	Controls.TechBox:SetSizeY(g_VictorBoxSizeY);
	local pPlayer = Players[Game.GetActivePlayer()];
	local have = pPlayer:GetScientificInfluence();
	local needed = pPlayer:GetScientificInfluenceNeeded();
	if needed == 0 then needed = 1; end -- Prevent a divide by 0.

	if(PreGame.IsVictory(GameInfo.Victories["VICTORY_SPACE_RACE"].ID))then
		DeleteTechScreen();
		local apolloProj = GameInfoTypes["PROJECT_APOLLO_PROGRAM"];
		local strPlayer = Locale.ConvertTextKey("TXT_KEY_CITY_STATE_NOBODY");

		local curRow = nil;
		local numRows = 0;
		local curItemIM = nil;
		local numItemIM = 0;
		local curCol = 0;
		local numItems = 0;
		
		local iActivePlayer = Game.GetActivePlayer();
		local pActivePlayer = Players[iActivePlayer];
			
		-- populate Tech competitions TODO MiniCompetitionTypes.COMPETITION_DIPLOMATIC_START
		for iCompetition = MiniCompetitionTypes.COMPETITION_SCIENCE_START, MiniCompetitionTypes.COMPETITION_SCIENCE_END-1, 1 do
			-- Create new row if one does not already exist
			if(curRow == nil) then
				numRows = numRows + 1;
				curRow = g_TechRowsIM:GetInstance();
				g_TechRowList[numRows] = curRow;
				
				numItemIM = numItemIM + 1;
				curItemIM = InstanceManager:new( "TechItem", "ItemChild", curRow.RowStack );
				g_TechItemIMList[numItemIM] = curItemIM;
				
			end							
			local item = curItemIM:GetInstance();
			g_TechItemList[numItems] = item;
			numItems = numItems + 1;
			
			UpdateCompetitionLine(iActivePlayer, iCompetition, item);
		end


		if(apolloProj ~= -1) then
			local iTeam = Game.GetActiveTeam();
			local completedPercent = math.max(0, math.min(1, have / needed));
			if completedPercent >= 1 then -- Teams[iTeam]:GetProjectCount(apolloProj) == 1) then
				Controls.ApolloIcon:SetColor(white); -- this screws up the TechProgress texture
				Controls.TechProgress:SetHide(true);
				Controls.BubblesAnim:SetHide(true);
				Controls.TechProgress:SetPercent(completedPercent);
			else
				--Controls.ApolloIcon:SetColor(gray); -- this screws up the TechProgress texture
				Controls.TechProgress:SetHide(false);
				Controls.BubblesAnim:SetHide(false);
				Controls.TechProgress:SetPercent(completedPercent);
				--Controls.TechIconAnim:SetHide(false); -- flashes
			end
			--[[
			local numApollo = 0;
			for i, v in pairs(Teams) do
				if(v:GetProjectCount(apolloProj) == 1) then
					numApollo = numApollo + 1;
				end
			end
			--]]

			-- show status
			local status = Locale.ConvertTextKey("You have " .. have .. " of " .. needed .. " {SCIENTIFIC_INFLUENCE} needed.");
			-- show turns remaining
			local statusDelta = "";
			local perTurn = pPlayer:GetScientificInfluencePerTurn();
			if ((have < needed) and (perTurn > 0)) then
				local turnsTillCompletion = math.ceil((needed - have) / perTurn);
				statusDelta = Locale.ConvertTextKey("You are receiving " .. perTurn .. " per turn, and will complete this in " .. turnsTillCompletion .. " turns.");
			end

			Controls.SpaceInfo:LocalizeAndSetText(status .. "[NEWLINE]" .. statusDelta);
		end

		local doneLabel = "";
		if (have >= needed) then
			doneLabel = doneString;
		end
		Controls.TechBoxLabel:SetHide(false);
		Controls.TechBoxLabel:LocalizeAndSetText("{TXT_KEY_VICTORYSCREEN_SPACE_RACE}" .. doneLabel);
		
		Controls.ScienceVictoryProgress:SetHide(false);
		Controls.ScienceVictoryDisabled:SetHide(true);
		Controls.SpaceRaceDetails:ReprocessAnchoring();
	else
		Controls.ScienceVictoryProgress:SetHide(true);
		Controls.ScienceVictoryDisabled:SetHide(false);
	end
	Controls.TechScrollPanel:CalculateInternalSize();
end


----------------------------------------------------------------
----------------------------------------------------------------
function SetProjectValue( sProject, iTeam, controlTable, controlTag )

	local white = {x = 1, y = 1, z = 1, w = 1};
	local proj = GameInfoTypes[sProject];
	local vProj = GameInfo.Project_VictoryThresholds( "ProjectType = '" .. sProject .. "'" )();
	
	if( proj ~= -1 and vProj ~= -1 ) then
		local numBuilt = Teams[ iTeam ]:GetProjectCount( proj );
		local count = math.min( vProj.Threshold, numBuilt );
		
		if( count > 0 ) then
			for i = 1, count do
				local control = controlTag .. tostring( i );
				controlTable[ control ]:SetColor( white );
			end
		end
	end
end 


----------------------------------------------------------------
----------------------------------------------------------------
function PopulateDiplomatic()
	Controls.DiploBox:SetSizeX(g_VictorBoxSizeX);
	Controls.DiploBox:SetSizeY(g_VictorBoxSizeY);
	Controls.UNIcon:SetHide(true);
	Controls.DiploVictoryProgress:SetHide(true);
	Controls.DiploVictoryDisabled:SetHide(false);

	local pPlayer = Players[Game.GetActivePlayer()];
	local victoryId;
	local victoryInfo = GameInfo.Victories["VICTORY_DIPLOMATIC"];
	if(victoryInfo ~= nil) then
		victoryId = victoryInfo.ID;
	end
	 
	-- is victory enabled?
	if (victoryId ~= nil and PreGame.IsVictory(victoryId)) then
		DeleteDiploScreen();
		
		local curRow = nil;
		local numRows = 0;
		local curItemIM = nil;
		local numItemIM = 0;
		local curCol = 0;
		local numItems = 0;	

		local have = 0;
		local needed = 0;
		
		local iActivePlayer = Game.GetActivePlayer();
		local pActivePlayer = Players[iActivePlayer];
			
		-- populate Diplo competitions TODO MiniCompetitionTypes.COMPETITION_DIPLOMATIC_START
		for iCompetition = MiniCompetitionTypes.COMPETITION_DIPLOMATIC_START, MiniCompetitionTypes.COMPETITION_DIPLOMATIC_END-1, 1 do
			-- Create new row if one does not already exist
			if(curRow == nil) then
				numRows = numRows + 1;
				curRow = g_DiploRowsIM:GetInstance();
				g_DiploRowList[numRows] = curRow;
				
				numItemIM = numItemIM + 1;
				curItemIM = InstanceManager:new( "DiploItem", "ItemChild", curRow.RowStack );
				g_DiploItemIMList[numItemIM] = curItemIM;
				
			end							
			local item = curItemIM:GetInstance();
			g_DiploItemList[numItems] = item;
			numItems = numItems + 1;

			UpdateCompetitionLine(iActivePlayer, iCompetition, item);
		end

		local have = pPlayer:GetTeamDiplomaticInfluence();
		local needed = pPlayer:GetTeamDiplomaticInfluenceNeeded();

		-- show progress
		local status = Locale.ConvertTextKey("You have " .. have .. " of " .. needed .. " {TXT_KEY_DIPLOMATIC_INFLUENCE} needed.");
		-- show turns remaining
		local statusDelta = "";
		local perTurn = pPlayer:GetTeamDiplomaticInfluencePerTurn();
		if ((have < needed) and (perTurn > 0)) then
			local turnsTillCompletion = math.ceil((needed - have) / perTurn);
			statusDelta = Locale.ConvertTextKey("You are receiving " .. perTurn .. " per turn, and will complete this in " .. turnsTillCompletion .. " turns.");
		end
			
		Controls.UNInfo:SetText(status .. "[NEWLINE]" .. statusDelta);-- .. sUNInfo);

		-- is done?
		local doneLabel = "";
		if (have >= needed) then
			doneLabel = doneString;
		end
		Controls.DiploBoxLabel:SetHide(false);
		Controls.DiploBoxLabel:LocalizeAndSetText("{TXT_KEY_VICTORYSCREEN_DIPLOMATIC}" .. doneLabel);

		-- show progress icon
		local completedPercent = math.max(0, math.min(1, have / needed));
		Controls.DiploProgress:SetHide(false);
		Controls.DiploProgress:SetPercent(completedPercent);
		if completedPercent >= 1 then
			Controls.UNIcon:SetHide(false); -- fill color
		end
	
		-- diplo screen is enabled
		Controls.DiploVictoryProgress:SetHide(false);
		Controls.DiploVictoryDisabled:SetHide(true);
		--[[
		local sUNInfo = Locale.Lookup("TXT_KEY_VP_DIPLO_UN_INACTIVE");
		if (Game.GetNumActiveLeagues() > 0) then
			local pLeague = Game.GetActiveLeague();
			
			if (pLeague ~= nil and Game.IsUnitedNationsActive()) then
				sUNInfo = Locale.Lookup("TXT_KEY_VP_DIPLO_UN_ACTIVE");
				Controls.TurnsUntilSessionLabel:SetHide(false);
				Controls.TurnsUntilSession:SetHide(false);
				Controls.TurnsUntilSession:SetText(pLeague:GetTurnsUntilVictorySession());
			end
		end
		--]]
	end
	Controls.DiploScrollPanel:CalculateInternalSize();
end

----------------------------------------------------------------
----------------------------------------------------------------
function UpdateCompetitionLine(iActivePlayer, iCompetition, item)
	local pActivePlayer = Players[iActivePlayer];
	local iWinningScore = Game.GetCompetitionWinnerScore(iCompetition);
	local iWinningPlayer = Game.GetCompetitionWinnerPlayer(iCompetition);
	local sDesc = Game.GetCompetitionDesc(iCompetition, iActivePlayer);
	local sDescShort = Game.GetCompetitionDescShort(iCompetition);
	local sDescReward = Game.GetCompetitionDescReward(iCompetition);
	-- prefix
	item.RowDesc:SetText(sDescShort);
	item.RowDesc:SetToolTipString(sDesc);
	-- post fix
	item.RowDescReward:SetText(sDescReward);
	item.RowDescReward:SetToolTipString(sDesc);

	item.ItemChild:SetToolTipString(sDesc);
	item.LabelContainer:SetSizeX(g_LabelContainerSizeX);

	-- civ icon
	if (iWinningScore == 0) then iWinningPlayer = -1; end
	iWinningPlayer = -1;
	local sCivName = pActivePlayer:GetCivNameSafe(iWinningPlayer);
	local iPlayerIcon = pActivePlayer:GetCivIconSafe(iWinningPlayer);
	item.IconBox:SetToolTipString(sCivName);
	CivIconHookup(iPlayerIcon, 32, item.Icon, item.IconBackground, item.IconShadow, false, true);
end

----------------------------------------------------------------
----------------------------------------------------------------
function PopulateCultural()
	Controls.CultureBox:SetSizeX(g_VictorBoxSizeX);
	Controls.CultureBox:SetSizeY(g_VictorBoxSizeY);

	local cultureVictory = GameInfo.Victories["VICTORY_CULTURAL"];
	if(cultureVictory ~= nil and PreGame.IsVictory(cultureVictory.ID))then
		for i, v in pairs(g_CultureItemIMList) do
			v:ResetInstances();
		end
		g_CultureRowsIM:ResetInstances();
		
		g_CultureRowList = {};
		g_CultureItemList = {};
		g_CultureItemIMList = {};
		
		local curRow = nil;
		local numRows = 0;
		local curItemIM = nil;
		local numItemIM = 0;
		local curCol = 0;
		local numItems = 0;	

		local have = 0;
		local needed = 0;
		
		local iActivePlayer = Game.GetActivePlayer();
		local pActivePlayer = Players[iActivePlayer];
			
		-- Loop through all the civs the active player knows
		for iPlayerLoop = 0, GameDefines.MAX_CIV_PLAYERS-1, 1 do

			local pPlayer = Players[iPlayerLoop];
			
			if(iPlayerLoop ~= iActivePlayer and not pPlayer:IsMinorCiv() and pPlayer:IsAlive()) then
				local iPlayer = pPlayer:GetID();
			
				-- calculate influence percent
				local influencePercent = 0;						
				local iInfluence = pActivePlayer:GetInfluenceOn(iPlayer);
				local iCulture = (pPlayer:GetJONSCultureEverGeneratedTimes100() + 50) / 100;
				if (iCulture > 0) then
					influencePercent = iInfluence / iCulture;
				end

				-- count amount we have influence over
				needed = needed + 1;
				if (influencePercent >= 1) then
					have = have + 1;
				end
			end
		end

		-- populate culture competitions TODO MiniCompetitionTypes.COMPETITION_DIPLOMATIC_START
		for iCompetition = MiniCompetitionTypes.COMPETITION_CULTURE_START, MiniCompetitionTypes.COMPETITION_CULTURE_END-1, 1 do
			-- Create new row if one does not already exist
			if(curRow == nil) then
				numRows = numRows + 1;
				curRow = g_CultureRowsIM:GetInstance();
				g_CultureRowList[numRows] = curRow;
				
				numItemIM = numItemIM + 1;
				curItemIM = InstanceManager:new( "CultureItem", "ItemChild", curRow.RowStack );
				g_CultureItemIMList[numItemIM] = curItemIM;
				
			end							
			local item = curItemIM:GetInstance();
			g_CultureItemList[numItems] = item;
			numItems = numItems + 1;
			
			UpdateCompetitionLine(iActivePlayer, iCompetition, item);
		end
		--[[
		local iPlayerIcon = 0;
		CivIconHookup(iPlayerIcon, 32, Controls.icon0, Controls.icon0bg, Controls.icon0shadow, false, true);
		--]]

		-- show progress
		local status = Locale.ConvertTextKey("You have enough {TXT_KEY_CULTURAL_INFLUENCE} over " .. have .. " of " .. needed .. " Civilizations.");		
		Controls.CultureInfo:SetText(status);

		local doneLabel = "";
		if (have >= needed) then
			doneLabel = doneString;
		end

		Controls.CultureBoxLabel:SetHide(false);
		Controls.CultureBoxLabel:LocalizeAndSetText("{TXT_KEY_VICTORYSCREEN_CULTURAL}" .. doneLabel);
		
		Controls.CultureStack:CalculateSize();
		Controls.CultureStack:ReprocessAnchoring();
	
		Controls.CultureVictoryProgress:SetHide(false);
		Controls.CultureVictoryDisabled:SetHide(true);
	else
		Controls.CultureVictoryProgress:SetHide(true);
		Controls.CultureVictoryDisabled:SetHide(false);
	end
	Controls.CultureScrollPanel:CalculateInternalSize();
end

----------------------------------------------------------------
----------------------------------------------------------------
function SetupScreen()	
	-- Title Icon
	SimpleCivIconHookup( Game.GetActivePlayer(), 64, Controls.Icon ); 
	
	-- Set Remaining Turns
	local remainingTurns = Game.GetMaxTurns() - Game.GetElapsedGameTurns();
	if remainingTurns < 0 then
		remainingTurns = 0;
	end
	Controls.RemainingTurns:SetText(remainingTurns);
	
	-- Set Player Score
	PopulateScoreBreakdown();
	
	Controls.HorizontalTrimWide0:SetSizeX(g_VictorTrimSizeX);
	Controls.HorizontalTrimWide1:SetSizeX(g_VictorTrimSizeX);

	-- Populate Victories
	--PopulateDomination();
	PopulateSpaceRace();
	PopulateDiplomatic();
	PopulateCultural();
		
end

----------------------------------------------------------------
----------------------------------------------------------------
function PopulateScoreScreen()
	g_ScoreIM:ResetInstances();
	g_ScoreList = {};
	
	local sortedPlayerList = {};
	-- Loop through all the civs the active player knows
	for iPlayerLoop = 0, GameDefines.MAX_CIV_PLAYERS-1, 1 do
		local pPlayer = Players[iPlayerLoop];
		if(not pPlayer:IsMinorCiv() and pPlayer:IsEverAlive()) then
			table.insert(sortedPlayerList, iPlayerLoop);			
		end
	end
	
	table.sort(sortedPlayerList, function(a, b) return Players[b]:GetScore() < Players[a]:GetScore() end);
	
	for i, v in ipairs(sortedPlayerList) do	
		local pPlayer = Players[v];
		
		-- Create Instance
		local controlTable = g_ScoreIM:GetInstance();
		g_ScoreList[i] = controlTable;
		
		-- Set Leader Icon
		SetCivLeader(pPlayer, 64, controlTable.Portrait);
	
		-- Set Civ Name
		SetCivName(pPlayer, i, controlTable.Score, PreGame.IsMultiplayerGame(), 730);
		controlTable.Name:SetText(controlTable.Score:GetText());
		
		-- Set Civ Icon
		SetCivIcon(pPlayer, 64, controlTable.Icon, nil, controlTable.IconOut, not pPlayer:IsAlive(), 
				   controlTable.CivIconBG, controlTable.CivIconShadow);
		
		-- Set Civ Score
		controlTable.Score:SetText(pPlayer:GetScore());
	end	
	
	Controls.ScoreStack:CalculateSize();
	Controls.ScoreStack:ReprocessAnchoring();
	Controls.ScoreScrollPanel:CalculateInternalSize();
end

----------------------------------------------------------------
----------------------------------------------------------------
function PopulateSpaceRaceScreen()
	g_TechIM:ResetInstances();
	g_TechList = {};
	
	local numTechs = 0;
	local white = {x = 1, y = 1, z = 1, w = 1};
	
	local apolloProj = GameInfoTypes["PROJECT_APOLLO_PROGRAM"];
	if(apolloProj ~= -1) then
		local sortedPlayerList = {};
		-- Loop through all the civs the active player knows
		for iPlayerLoop = 0, GameDefines.MAX_CIV_PLAYERS-1, 1 do
			local pPlayer = Players[iPlayerLoop];
			if(not pPlayer:IsMinorCiv() and pPlayer:IsEverAlive()) then
				table.insert(sortedPlayerList, iPlayerLoop);			
			end
		end
		
		table.sort(sortedPlayerList, CompareSpaceRace);
	
		for i, v in ipairs(sortedPlayerList) do	
			local pPlayer = Players[v];
			local pTeam = pPlayer:GetTeam();
			
			-- Create Instance
			local controlTable = g_TechIM:GetInstance();
			g_TechList[i] = controlTable;
	
			-- Set Civ Name
			SetCivName(pPlayer, i, controlTable.Name, true, controlTable.Civ:GetSizeX() - controlTable.CivIconBG:GetSizeX());
			
			-- Set Civ Icon
			SetCivIcon(pPlayer, 45, controlTable.Icon, nil, controlTable.IconOut, not pPlayer:IsAlive(),
					   controlTable.CivIconBG, controlTable.CivIconShadow);
			
			-- Fill in Apollo Project
			if(Teams[pTeam]:GetProjectCount(apolloProj) == 1) then
				controlTable.ApolloIcon:SetColor(white);
			end
		end	
	end
	
	Controls.ScrollPanel:CalculateInternalSize();
end

----------------------------------------------------------------
----------------------------------------------------------------
function CompareSpaceRace(a, b)
	local playerA = Players[a];
	local playerB = Players[b];
	local pTeamA = Teams[ playerA:GetTeam() ];
	local pTeamB = Teams[ playerB:GetTeam() ];
	local apolloA = pTeamA:GetProjectCount(apolloProj) == 1;
	local apolloB = pTeamB:GetProjectCount(apolloProj) == 1;
	
	if(apolloA) then
		if(apolloB)then
			--Add up parts
			local partsA = 0;
			local partsB = 0;
			local proj = GameInfoTypes["PROJECT_SS_BOOSTER"];
			if(proj ~= -1) then
				partsA = partsA + pTeamA:GetProjectCount(proj);
				partsB = partsB + pTeamB:GetProjectCount(proj);
			end
			
			proj = GameInfoTypes["PROJECT_SS_COCKPIT"];
			if(proj ~= -1) then
				partsA = partsA + pTeamA:GetProjectCount(proj);
				partsB = partsB + pTeamB:GetProjectCount(proj);
			end
			
			proj = GameInfoTypes["PROJECT_SS_STASIS_CHAMBER"];
			if(proj ~= -1) then
				partsA = partsA + pTeamA:GetProjectCount(proj);
				partsB = partsB + pTeamB:GetProjectCount(proj);
			end
			
			proj = GameInfoTypes["PROJECT_SS_ENGINE"];
			if(proj ~= -1) then
				partsA = partsA + pTeamA:GetProjectCount(proj);
				partsB = partsB + pTeamB:GetProjectCount(proj);
			end
			
			return partsB < partsA;
		else
			return true;
		end
	elseif(apolloB)then
		return false;
	else
		return false;
	end
end

----------------------------------------------------------------
----------------------------------------------------------------
function SetCivLeader(pPlayer, iconSize, controlTable)
	-- Use the Civilization_Leaders table to cross reference from this civ to the Leaders table
	local pTeam = Teams[pPlayer:GetTeam()];
	if (pTeam:IsHasMet(Game.GetActiveTeam()) or Game:IsNetworkMultiPlayer()) then
		local leader = GameInfo.Leaders[pPlayer:GetLeaderType()];
		local leaderDescription = leader.Description;

		IconHookup( leader.PortraitIndex, iconSize, leader.IconAtlas, controlTable );
	else
		IconHookup( 22, iconSize, "LEADER_ATLAS", controlTable );
	end
end
----------------------------------------------------------------
----------------------------------------------------------------
function SetCivName(pPlayer, number, controlTable, truncate, controlSize)
	if(truncate == nil)then
		truncate = false;
	end
	
	-- Set Text
	local strPlayer = "";
	local strCiv = "";
	local myCivType = pPlayer:GetCivilizationType(); 
	local myCivInfo = GameInfo.Civilizations[myCivType];

	if(pPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
		strPlayer = pPlayer:GetNickName();
		strCiv = myCivInfo.ShortDescription;
	elseif(pPlayer:GetID() == Game.GetActivePlayer()) then
		if(PreGame.GetCivilizationShortDescription(pPlayer:GetID()) ~= "") then
			strPlayer = "TXT_KEY_POP_VOTE_RESULTS_YOU";
			strCiv = PreGame.GetCivilizationShortDescription(pPlayer:GetID());
		else
			strPlayer = "TXT_KEY_POP_VOTE_RESULTS_YOU";
			strCiv = myCivInfo.ShortDescription;
		end
	else
		strPlayer = pPlayer:GetNameKey();
		strCiv = myCivInfo.ShortDescription;
	end
	
	local pTeam = Teams[pPlayer:GetTeam()];
	if (pTeam:IsHasMet(Game.GetActiveTeam()) or Game:IsNetworkMultiPlayer()) then
		if(truncate)then
			local displayText = Locale.ConvertTextKey("TXT_KEY_NUMBERING_FORMAT", number) .. " " .. Locale.ConvertTextKey( "TXT_KEY_RANDOM_LEADER_CIV", strPlayer, strCiv );
			TruncateString(controlTable, controlSize, displayText);
		else
			controlTable:SetText( Locale.ConvertTextKey("TXT_KEY_NUMBERING_FORMAT", number) .. " " .. Locale.ConvertTextKey( "TXT_KEY_RANDOM_LEADER_CIV", strPlayer, strCiv ) );
		end
	else
		if(truncate)then
			local displayText = Locale.ConvertTextKey("TXT_KEY_NUMBERING_FORMAT", number) .. " " .. Locale.ConvertTextKey( "TXT_KEY_MISC_UNKNOWN");
			TruncateString(controlTable, controlSize, displayText);
		else
			controlTable:SetText( Locale.ConvertTextKey("TXT_KEY_NUMBERING_FORMAT", number) .. " " .. Locale.ConvertTextKey( "TXT_KEY_MISC_UNKNOWN"));
		end
	end
end

----------------------------------------------------------------
----------------------------------------------------------------
function SetCivIcon(pPlayer, iconSize, controlTable, controlTableTT, controlLost, lostCondition, controlBG, controlShadow)
	local iPlayer = pPlayer:GetID();
	-- Set Civ Icon
	local pTeam = Teams[pPlayer:GetTeam()];
	if (pTeam:IsHasMet(Game.GetActiveTeam()) or Game:IsNetworkMultiPlayer()) then
		CivIconHookup( iPlayer, iconSize, controlTable, controlBG, controlShadow, false, true );
		if(controlTableTT ~= nil) then
			if(Game:IsNetworkMultiPlayer() and pPlayer:GetNickName() ~= "") then
				controlTableTT:SetToolTipString(pPlayer:GetNickName());
			elseif(iPlayer == Game.GetActivePlayer()) then
				controlTableTT:SetToolTipString(Locale.ConvertTextKey( "TXT_KEY_POP_VOTE_RESULTS_YOU" ));
			else
				local myCivType = pPlayer:GetCivilizationType();
				local myCivInfo = GameInfo.Civilizations[myCivType];
				controlTableTT:SetToolTipString(Locale.ConvertTextKey(myCivInfo.ShortDescription));
			end
		end
	else
		CivIconHookup(-1, iconSize, controlTable, controlBG, controlShadow, false, true );
		if(controlTableTT ~= nil) then
			controlTableTT:SetToolTipString(Locale.ConvertTextKey( "TXT_KEY_MISC_UNKNOWN"));
		end
	end
	
	-- X out and gray any icons of civs who have lost capitals
	if(lostCondition and controlLost ~= nil) then
		local darken = {x = 1, y = 1, z = 1, w = 0.25};
		controlLost:SetHide(false);
		controlTable:SetColor(darken);
	end
end

----------------------------------------------------------------
----------------------------------------------------------------
function SetConquestCivIcon(pPlayer, iconSize, controlTable, controlTableTT, controlDominatingPlayer, controlConqueredCivIconBG, controlConqueredCivIconShadow, controlConqueredCivIcon, controlIconOut, pDominatingPlayer, controlBG, controlShadow)
	local iPlayer = pPlayer:GetID();
	
	-- Set Civ Icon
	local pTeam = Teams[pPlayer:GetTeam()];
	local bHasMetOrMultiplayer = pTeam:IsHasMet(Game.GetActiveTeam()) or Game:IsNetworkMultiPlayer();
	
	if (bHasMetOrMultiplayer == true) then
		CivIconHookup(iPlayer, iconSize, controlTable, controlBG, controlShadow, false, true );
	else
		CivIconHookup(-1, iconSize, controlTable, controlBG, controlShadow, false, true );
	end	
	
	if(pDominatingPlayer == nil) then
		if(iPlayer == Game.GetActivePlayer()) then
			controlTableTT:SetToolTipString(Locale.Lookup("TXT_KEY_VP_DIPLO_TT_YOU_NO_CAPITAL"));
	
		elseif(bHasMetOrMultiplayer) then	
			local playerName;
			if (pPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
				playerName = pPlayer:GetNickName();
			else
				playerName = pPlayer:GetName();
			end
		
			controlTableTT:SetToolTipString(Locale.Lookup("TXT_KEY_VP_DIPLO_TT_KNOWN_NO_CAPITAL", playerName));
		else
			controlTableTT:SetToolTipString(Locale.Lookup("TXT_KEY_VP_DIPLO_TT_UNKNOWN_NO_CAPITAL"));
		end
	
		return;
	end
	
	local iDominatingPlayer = pDominatingPlayer:GetID();	
	local pDominatingTeam = Teams[pDominatingPlayer:GetTeam()];
	
	if (iPlayer == iDominatingPlayer) then
		controlDominatingPlayer:SetHide(true);
	else
		controlDominatingPlayer:SetHide(false);
		if (pDominatingTeam:IsHasMet(Game.GetActiveTeam())) then
			CivIconHookup(iDominatingPlayer, 45, controlConqueredCivIcon, controlConqueredCivIconBG, controlConqueredCivIconShadow, false, true);
		else
			CivIconHookup(-1, 45, controlConqueredCivIcon, controlConqueredCivIconBG, controlConqueredCivIconShadow, false, true);
		end
	end
	
	if (controlTableTT ~= nil) then
		if (iPlayer == iDominatingPlayer) then
			if (pTeam:IsHasMet(Game.GetActiveTeam())) then
				-- get city name
				local pOriginalCapital = nil;
				for pCity in pPlayer:Cities() do
					if (pCity:IsOriginalMajorCapital() and pCity:GetOriginalOwner() == iPlayer) then
						pOriginalCapital = pCity;
					end
				end
	
				if (iPlayer == Game.GetActivePlayer()) then
					controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_YOU_CONTROL_YOUR_CAPITAL", pOriginalCapital:GetNameKey()));
				else
					if (pPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
						controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_SOMEONE_CONTROLS_THEIR_CAPITAL", pPlayer:GetNickName(), pOriginalCapital:GetNameKey()));
					else
						controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_SOMEONE_CONTROLS_THEIR_CAPITAL", pPlayer:GetName(), pOriginalCapital:GetNameKey()));
					end
				end
			else
				controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_UNMET_CONTROLS_THEIR_CAPITAL"));			
			end
		else
			-- get city name
			if (pTeam:IsHasMet(Game.GetActiveTeam())) then
				-- get city name
				local pOriginalCapital = nil;
				for pCity in pDominatingPlayer:Cities() do
					if (pCity:IsOriginalMajorCapital() and pCity:GetOriginalOwner() == iPlayer) then
						pOriginalCapital = pCity;
					end
				end
			
				if (pDominatingTeam:IsHasMet(Game.GetActiveTeam())) then
					if (iPlayer == Game.GetActivePlayer()) then
						if (pDominatingPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
							controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_YOUR_CAPITAL", pDominatingPlayer:GetNickName(), pOriginalCapital:GetNameKey()));
						else
							controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_YOUR_CAPITAL", pDominatingPlayer:GetName(), pOriginalCapital:GetNameKey()));
						end
					else
						if (iDominatingPlayer == Game.GetActivePlayer()) then
							controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_YOU_CONTROL_OTHER_PLAYER_CAPITAL", pOriginalCapital:GetNameKey(), pPlayer:GetCivilizationShortDescriptionKey()));
						else
							if (pDominatingPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
								controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_OTHER_PLAYER_CAPITAL", pDominatingPlayer:GetNickName(), pOriginalCapital:GetNameKey(), pPlayer:GetCivilizationShortDescriptionKey()));
							else
								controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_OTHER_PLAYER_CAPITAL", pDominatingPlayer:GetName(), pOriginalCapital:GetNameKey(), pPlayer:GetCivilizationShortDescriptionKey()));
							end
						end
					end
				else
					controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_UNMET_PLAYER_CONTROLS_OTHER_PLAYER_CAPITAL", pOriginalCapital:GetNameKey(), pPlayer:GetCivilizationShortDescriptionKey()));
				end
			else
				if (pDominatingTeam:IsHasMet(Game.GetActiveTeam())) then
					if (pDominatingPlayer:GetNickName() ~= "" and Game:IsNetworkMultiPlayer()) then
						controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_UNMET_PLAYER_CAPITAL", pDominatingPlayer:GetNickName()));
					else
						controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_OTHER_PLAYER_CONTROLS_UNMET_PLAYER_CAPITAL", pDominatingPlayer:GetName()));
					end
				else
					controlTableTT:SetToolTipString(Locale.ConvertTextKey("TXT_KEY_VP_DIPLO_TT_UNMET_PLAYER_CONTROLS_UNMET_PLAYER_CAPITAL"));
				end
			end
		end
	end
end


----------------------------------------------------------------
----------------------------------------------------------------
function DeleteScoreScreen()
	g_ScoreIM:ResetInstances();
	g_ScoreList = {};
end
----------------------------------------------------------------
----------------------------------------------------------------
function DeleteSpaceRaceScreen()
	g_TechIM:ResetInstances();
	g_TechList = {};
end
----------------------------------------------------------------
----------------------------------------------------------------
function DeleteTechScreen()
	for i, v in pairs(g_TechItemIMList) do
		v:ResetInstances();
	end
	g_TechRowsIM:ResetInstances();
	g_TechRowList = {};
	g_TechItemList = {};
	g_TechItemIMList = {};
end
----------------------------------------------------------------
----------------------------------------------------------------
function DeleteDiploScreen()
	for i, v in pairs(g_DiploItemIMList) do
		v:ResetInstances();
	end
	g_DiploRowsIM:ResetInstances();
	g_DiploRowList = {};
	g_DiploItemList = {};
	g_DiploItemIMList = {};
end
----------------------------------------------------------------
----------------------------------------------------------------
function GetPreReqs(techID)
	
	local found = false;
	for i,v in ipairs(g_TechPreReqList) do
		if(v == techID)then
			found = true;
			break;
		end
	end
	
	if(not found)then
		table.insert(g_TechPreReqList, techID);
	end
	
	for row in GameInfo.Technology_PrereqTechs{TechType = techID} do
		GetPreReqs(row.PrereqTech);
	end
end

--------------------------------------------
--------------------------------------------
function CountPreReqsAcquired()
	g_PreReqsAcquired = 0;
	local pTeam = Teams[Game.GetActiveTeam()];
	
	for i, v in pairs(g_TechPreReqList) do
		local tech = GameInfo.Technologies[v];
		if (tech ~= nil) then
			local techID = tech.ID;
		
			if(pTeam:IsHasTech(techID)) then
				g_PreReqsAcquired = g_PreReqsAcquired + 1;
			end
		end
	end
end
Events.TechAcquired.Add( CountPreReqsAcquired );

--------------------------------------------
--------------------------------------------
-- Find Tech Pre-Reqs Once
--------------------------------------------
local ApolloTech = GameInfo.Projects["PROJECT_APOLLO_PROGRAM"].TechPrereq;

GetPreReqs(ApolloTech);
CountPreReqsAcquired();


-------------------------------------------------
-- On Popup
-------------------------------------------------
function OnPopup( popupInfo )

	if( popupInfo.Type == ButtonPopupTypes.BUTTONPOPUP_VICTORY_INFO ) then
		m_PopupInfo = popupInfo;
		if( m_PopupInfo.Data1 == 1 ) then
		
        	if( ContextPtr:IsHidden() == false ) then
        	    OnBack();
            else
            	UIManager:QueuePopup( ContextPtr, PopupPriority.InGameUtmost );
        	end
    	else
        	UIManager:QueuePopup( ContextPtr, PopupPriority.VictoryProgress );
    	end
	end
end
Events.SerialEventGameMessagePopup.Add( OnPopup );

----------------------------------------------------------------
-- Key Down Processing
----------------------------------------------------------------
function InputHandler( uiMsg, wParam, lParam )
    if( uiMsg == KeyEvents.KeyDown )
    then
        if( wParam == Keys.VK_RETURN or wParam == Keys.VK_ESCAPE ) then
			OnBack();
            return true;
        end
    end
end
ContextPtr:SetInputHandler( InputHandler );


----------------------------------------------------------------
----------------------------------------------------------------
--function OnDisplay( type, player )
	--UIManager:QueuePopup( ContextPtr, PopupPriority.VictoryProgress );
--end


----------------------------------------------------------------        
----------------------------------------------------------------        
function ShowHideHandler( bIsHide, bIsInit )
    
    if( not bIsInit ) then
        if( not bIsHide ) then
			SetupScreen();
            UI.incTurnTimerSemaphore();
            Events.SerialEventGameMessagePopupShown(m_PopupInfo);
        else
			Events.SerialEventGameMessagePopupProcessed.CallImmediate(ButtonPopupTypes.BUTTONPOPUP_VICTORY_INFO, 0);
            UI.decTurnTimerSemaphore();
        end
    end

end
ContextPtr:SetShowHideHandler( ShowHideHandler );

----------------------------------------------------------------
-- 'Active' (local human) player has changed
----------------------------------------------------------------
Events.GameplaySetActivePlayer.Add(OnBack);
