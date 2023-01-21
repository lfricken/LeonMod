


#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"


void PlaceRandomSpy(CvPlayer* pPlayer)
{

}


FDataStream& operator <<(FDataStream& kStream, const TradingCardTypes& data)
{
	kStream << (int)data;
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, TradingCardTypes& data)
{
	int type;
	kStream >> type;
	data = (TradingCardTypes)type;
	return kStream;
}
const CvPolicyEntry* GetPolicyInfo(const string policyName)
{
	if (policyName.size() == 0)
	{
		return NULL;
	}
	const CvPolicyXMLEntries* pAllPolicies = GC.GetGamePolicies();
	const PolicyTypes ePolicy = pAllPolicies->Policy(policyName);
	const CvPolicyEntry* pInfo = GC.getPolicyInfo(ePolicy);
	return pInfo;
}
string TradingCard::GetName(TradingCardTypes type, CvPlayer* pOwner)
{
	const CvPolicyEntry* activeInfo = GetPolicyInfo(TradingCard::GetActivePolicy(type));
	string active = activeInfo == NULL ? "" : activeInfo->GetDescription();
	const CvPolicyEntry* passiveInfo = GetPolicyInfo(TradingCard::GetPassivePolicy(type));
	string passive = passiveInfo == NULL ? "" : passiveInfo->GetDescription();
	string joiner = "";
	if (active.size() != 0 && active.size() != 0)
	{
		joiner = " ";
	}
	return passive + joiner + active;
}
string TradingCard::GetDesc(TradingCardTypes type, CvPlayer* pOwner)
{
	const CvPolicyEntry* activeInfo = GetPolicyInfo(TradingCard::GetActivePolicy(type));
	string active = activeInfo == NULL ? "" : activeInfo->GetHelp();
	const CvPolicyEntry* passiveInfo = GetPolicyInfo(TradingCard::GetPassivePolicy(type));
	string passive = passiveInfo == NULL ? "" : passiveInfo->GetHelp();
	string joiner = "";
	if (active.size() != 0 && active.size() != 0)
	{
		joiner = " ";
	}
	return passive + joiner + active;
}
bool TradingCard::TryActivate(TradingCardTypes type, CvPlayer* pActivatingPlayer)
{
	switch (type)
	{
	case CARD_RANDOM_SPY: PlaceRandomSpy(pActivatingPlayer); return true;
	default: return true;
	};

	return true;
}
string TradingCard::GetActivePolicyDesc(TradingCardTypes type)
{
	const CvPolicyEntry* pInfo = GetPolicyInfo(TradingCard::GetActivePolicy(type));
	if (pInfo == NULL)
	{
		return "";
	}
	const string desc = GetLocalizedText(pInfo->GetHelp());
	return desc;
}
string TradingCard::GetPassivePolicyDesc(TradingCardTypes type)
{
	const CvPolicyEntry* pInfo = GetPolicyInfo(TradingCard::GetPassivePolicy(type));
	if (pInfo == NULL)
	{
		return "";
	}
	const string desc = GetLocalizedText(pInfo->GetHelp());
	return desc;
}
