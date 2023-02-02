


#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"




bool operator==(const TradingCardState& lhs, const TradingCardState& rhs)
{
	if (&lhs == &rhs) return true;
	if (lhs.type != rhs.type) return false;
	if (lhs.isVisible != rhs.isVisible) return false;
	return true;
}
FDataStream& operator <<(FDataStream& kStream, const TradingCardState& data)
{
	kStream << data.type;
	kStream << data.isVisible;
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, TradingCardState& data)
{
	kStream >> data.type;
	kStream >> data.isVisible;
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
string TradingCard::GetName(TradingCardTypes type, CvPlayer* player)
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
string TradingCard::GetDesc(TradingCardTypes type, CvPlayer* player)
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
void TradingCard::OnCountChanged(TradingCardTypes cardType, CvPlayer* player, int delta)
{
	player->CardsOnChanged();
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
bool TradingCard::CanActivate(TradingCardTypes cardType, const CvPlayer* pPlayer, stringstream* noActiveReason)
{
	const string activePolicyName = TradingCard::GetActivePolicy(cardType);
	// no active policy
	if (activePolicyName.size() <= 0)
	{
		return false;
	}
	bool success = true;
	// fails condition
	if (!TradingCard::IsConditionSatisfied(cardType, pPlayer, true))
	{
		(*noActiveReason) << "You do not meet the conditions of the card.[NEWLINE]";
		success = false;
	}
	// cannot activate if we have the policy
	if (pPlayer->HasPolicy(activePolicyName))
	{
		(*noActiveReason) << "You already activated another copy of this card.[NEWLINE]";
		success = false;
	}
	return success;
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
