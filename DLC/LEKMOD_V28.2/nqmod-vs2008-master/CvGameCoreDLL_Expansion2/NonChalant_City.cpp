

#include "CvGameCoreDLLPCH.h"
#include "CvCity.h"







int CvCity::getInfrastructureDamagePenalty() const
{
	int penalty = 2;

	return penalty;
}

int CvCity::getRange() const
{
	int totalRange = GC.getCITY_ATTACK_RANGE();



	return totalRange;
}
