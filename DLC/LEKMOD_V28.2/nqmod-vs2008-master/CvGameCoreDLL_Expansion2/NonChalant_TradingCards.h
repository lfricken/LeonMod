#pragma once


#include <vector>
#include <FireWorks/FAutoArchive.h>
#include "CvEnums.h"

class CvPlayer;

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
	static bool TryActivate(TradingCardTypes type, CvPlayer* activatingPlayer);


	static string GetActivePolicy(TradingCardTypes type);
	static string GetPassivePolicy(TradingCardTypes type);
	static string GetActivePolicyDesc(TradingCardTypes type);
	static string GetPassivePolicyDesc(TradingCardTypes type);
private:
};
FDataStream& operator <<(FDataStream& kStream, const TradingCardTypes& data);
FDataStream& operator >>(FDataStream& kStream, TradingCardTypes& data);





