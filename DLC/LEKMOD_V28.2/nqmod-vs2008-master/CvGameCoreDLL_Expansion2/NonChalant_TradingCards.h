#pragma once


#include <vector>
#include <FireWorks/FAutoArchive.h>
#include "CvEnums.h"

class CvPlayer;


class TradingCard
{
public:
	TradingCard(const TradingCardTypes type);

	TradingCardTypes GetType() const;
	bool IsPassive() const;
	// returns true if this card activated
	bool TryActivate(CvPlayer* activatingPlayer) const;
private:
	TradingCardTypes m_type;
};
FDataStream& operator <<(FDataStream& kStream, const TradingCard& data);
FDataStream& operator >>(FDataStream& kStream, TradingCard& data);





