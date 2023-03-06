#pragma once


#include <vector>
#include <FireWorks/FAutoArchive.h>
#include "CvEnums.h"

class CvPlayer;

// Keeps track of info for a particular card.
struct TradingCardState
{
	TradingCardTypes type;
	// is this visible for trading? Can be toggled by the player.
	bool isVisible;
};
bool operator==(const TradingCardState& lhs, const TradingCardState& rhs);
FDataStream& operator <<(FDataStream& kStream, const TradingCardState& data);
FDataStream& operator >>(FDataStream& kStream, TradingCardState& data);

// Applies some sort of benefit to the player. Can be traded.
class TradingCard
{
public:
	// is this policy id a card?
	static bool IsCard(int type);
	// what era should this card be showing up for?
	static int Era(int type);
	// units, buildings, vps, etc.
	static int Genre(int type);
	// what is this card called?
	static string GetName(TradingCardTypes type, CvPlayer* pOwner);
	// what does this card do?
	static string GetDesc(TradingCardTypes type, CvPlayer* pOwner);
	// Applies the active benefit of this card. returns true if this card did activate.
	// you need to destroy this card if it activated. Checks condition.
	static bool ApplyActiveEffects(TradingCardTypes type, CvPlayer* player);
	// called when the players card count changes
	static void OnCountChanged(TradingCardTypes type, CvPlayer* player, int delta);
	// Called after the card and passive policy has been removed, lets us do custom passive logic
	static void ApplyPassiveEffects(TradingCardTypes type, CvPlayer* player, int delta);
	// Maps a card type with a policy that gets granted when the card activates
	static PolicyTypes GetActivePolicy(TradingCardTypes type);
	// Maps a card type with a policy that gets granted when a player has the card
	static PolicyTypes GetPassivePolicy(TradingCardTypes type);
	static string GetActivePolicyDesc(TradingCardTypes type);
	static string GetPassivePolicyDesc(TradingCardTypes type);
	// Used by the AI to estimate how much value a card type has
	static int GetEstimatedValue(bool bFromMe, TradingCardTypes type);
	static bool IsConditionSatisfied(TradingCardTypes type, const CvPlayer* player, bool isActive);
	// return true if we should delete every card of this type in this players inventory
	static bool ShouldDeleteCard(TradingCardTypes type, const CvPlayer* player);
	static bool CanActivate(TradingCardTypes type, const CvPlayer* pPlayer, stringstream* noActiveReason);
private:
};
FDataStream& operator <<(FDataStream& kStream, const TradingCardTypes& data);
FDataStream& operator >>(FDataStream& kStream, TradingCardTypes& data);





