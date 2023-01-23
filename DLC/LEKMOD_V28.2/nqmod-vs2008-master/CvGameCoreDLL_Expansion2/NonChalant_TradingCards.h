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
	// what is this card called?
	static string GetName(TradingCardTypes type, CvPlayer* pOwner);
	// what does this card do?
	static string GetDesc(TradingCardTypes type, CvPlayer* pOwner);
	// Applies the active benefit of this card. returns true if this card did activate.
	// you need to destroy this card now.
	static bool TryActivate(TradingCardTypes type, CvPlayer* player);
	// called when the players card count changes
	static void OnCountChanged(TradingCardTypes type, CvPlayer* player, int delta);
	// Called after the card and passive policy has been removed, lets us do custom passive logic
	static void TryApplyPassiveEffects(TradingCardTypes type, CvPlayer* player, int delta);
	// Maps a card type with a policy that gets granted when the card activates
	static string GetActivePolicy(TradingCardTypes type);
	// Maps a card type with a policy that gets granted when a player has the card
	static string GetPassivePolicy(TradingCardTypes type);
	static string GetActivePolicyDesc(TradingCardTypes type);
	static string GetPassivePolicyDesc(TradingCardTypes type);
	// Used by the AI to estimate how much value a card type has
	static int GetEstimatedValue(TradingCardTypes type);
private:
};
FDataStream& operator <<(FDataStream& kStream, const TradingCardTypes& data);
FDataStream& operator >>(FDataStream& kStream, TradingCardTypes& data);





