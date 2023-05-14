

#include <algorithm>
#include "CvGameCoreDLLPCH.h"
#include "FStlContainerSerialization.h"
#include "CvGameCoreUtils.h"

// interface for functions that calculate mini competion information
struct CompetitionDelegates
{
	virtual int EvalScore(const CvPlayer& player) const = 0;
	virtual string DescReward(const CvCompetition& rCompetition) const = 0;
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const = 0;
	virtual string DescShort(const int iWinningScore) const = 0;
	virtual int Reward(const YieldTypes eType) const = 0;
};

float scoreToDisplay(int score)
{
	return (float)score / (float)10;
}
const int INVALID_SCORE = 0;
string tryAddCurrentScore(const CvCompetition& rCompetition, const PlayerTypes ePlayer)
{
	stringstream ss;
	if (ePlayer != NO_PLAYER)
	{
		ss << "[NEWLINE][NEWLINE]You currently have [COLOR_WARNING_TEXT]" << scoreToDisplay(rCompetition.GetScoreOfPlayer(ePlayer)) << "[ENDCOLOR]";
	}
	return ss.str();
}


// COMPETITION_TRADE_ROUTES_INTERNATIONAL
struct TradeRoutes : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Most International {TRADE_ROUTE}s: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_DIPLOMATIC_SUPPORT) << " [ICON_DIPLOMATIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most International {TRADE_ROUTE}s. (routes ending on another Civilization or City State)";
		ss << tryAddCurrentScore(rCompetition, ePlayer);
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int score = player.GetTrade()->GetNumForeignTradeRoutes(player.GetID());
		return score;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_DIPLOMATIC_SUPPORT)
			return 15;
		return 0;
	}
};
// COMPETITION_ALLIES
struct Allies : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Most Controlled {CITY_STATE}s: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_DIPLOMATIC_SUPPORT) << " [ICON_DIPLOMATIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most {CITY_STATE}s controlled. (allies or conquered)";
		ss << tryAddCurrentScore(rCompetition, ePlayer);
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int perTurn, numControlled;
		player.GetDiplomaticInfluencePerTurn(&perTurn, &numControlled);
		return numControlled;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_DIPLOMATIC_SUPPORT)
			return 20;
		return 0;
	}
};
// COMPETITION_GOLD_GIFTS
struct GoldGifts : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Most [ICON_INVEST] Gold Gifted: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_DIPLOMATIC_SUPPORT) << " [ICON_DIPLOMATIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most total [ICON_GOLD] Gold gifted to city states.";
		ss << tryAddCurrentScore(rCompetition, ePlayer);
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = player.GetNumGoldGiftedToMinors();
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_DIPLOMATIC_SUPPORT)
			return 20;
		return 0;
	}
};
// COMPETITION_NUCLEAR_STOCKPILE
struct Nukes : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Largest [ICON_RES_URANIUM] Nuclear Arsenal: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_DIPLOMATIC_SUPPORT) << " [ICON_DIPLOMATIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most [ICON_RES_URANIUM] nuclear weapons.";
		ss << tryAddCurrentScore(rCompetition, ePlayer);
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = player.GetNumNuclearWeapons();
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_DIPLOMATIC_SUPPORT)
			return 50;
		return 0;
	}
};
// COMPETITION_SCIENCE_SPECIALISTS
struct ScienceSpecialists : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Most [ICON_SPECIALIST_SCIENTIST] Scientist Specialists: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_SCIENTIFIC_INSIGHT) << " [ICON_SCIENTIFIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most citizens working as science specialists.";
		ss << tryAddCurrentScore(rCompetition, ePlayer);
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = player.GetNumScienceSpecialists();
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_SCIENTIFIC_INSIGHT)
			return 5;
		return 0;
	}
};
// COMPETITION_SCIENCE_COMPETITION
struct ScienceHammerCompetition : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Best {SCIENCE_FAIR}: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_SCIENTIFIC_INSIGHT) << " [ICON_SCIENTIFIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization that has devoted the most [ICON_PRODUCTION] Production towards their {SCIENCE_FAIR}.";
		ss << tryAddCurrentScore(rCompetition, ePlayer) << " [ICON_PRODUCTION]";
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = (int)(player.GetCompetitionHammersT100(HAMMERCOMPETITION_SCIENTIFIC_INSIGHT) / 100);
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_SCIENTIFIC_INSIGHT)
			return 5;
		return 0;
	}
};
// COMPETITION_SCIENCE_IMPROVEMENTS
struct ScienceImprovements : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Most Academies: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_SCIENTIFIC_INSIGHT) << " [ICON_SCIENTIFIC_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization with the most Academies.";
		ss << tryAddCurrentScore(rCompetition, ePlayer) << "";
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = player.GetNumScienceImprovements();
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_SCIENTIFIC_INSIGHT)
			return 5;
		return 0;
	}
};
// COMPETITION_CULTURE_COMPETITION
struct CulturalCompetition : CompetitionDelegates
{
	virtual string DescShort(const int iWinningScore) const
	{
		stringstream ss;
		ss << "Best {ARTS_FAIR}: [COLOR_POSITIVE_TEXT]" << scoreToDisplay(iWinningScore) << "[ENDCOLOR]";
		return ss.str();
	}
	virtual string DescReward(const CvCompetition& rCompetition) const
	{
		stringstream ss;
		ss << "+" << Reward(YIELD_TOURISM) << "% [ICON_CULTURAL_INFLUENCE]";
		return ss.str();
	}
	virtual string Desc(const CvCompetition& rCompetition, const PlayerTypes ePlayer) const
	{
		stringstream ss;
		ss << "The civilization that has devoted the most [ICON_PRODUCTION] Production towards their {ARTS_FAIR} ";
		ss << "will receive the influence reward over any civilization that has a lower score.";
		ss << tryAddCurrentScore(rCompetition, ePlayer) << " [ICON_PRODUCTION]";
		return ss.str();
	}
	virtual int EvalScore(const CvPlayer& player) const
	{
		int iScore = (int)(player.GetCompetitionHammersT100(HAMMERCOMPETITION_CULTURAL_INFLUENCE) / 100);
		return iScore;
	}
	virtual int Reward(const YieldTypes eType) const
	{
		if (eType == YIELD_TOURISM)
			return GC.getTOURISM_MODIFIER_HAMMERCOMPETITION(NO_PLAYER);
		return 0;
	}
};
void CvGlobals::initCompetitions()
{
	GetDelegatesFor.push_back(new CulturalCompetition());

	GetDelegatesFor.push_back(new TradeRoutes());
	GetDelegatesFor.push_back(new Allies());
	GetDelegatesFor.push_back(new GoldGifts());
	GetDelegatesFor.push_back(new Nukes());

	GetDelegatesFor.push_back(new ScienceSpecialists());
	GetDelegatesFor.push_back(new ScienceHammerCompetition());
	GetDelegatesFor.push_back(new ScienceImprovements());
}
void CvGlobals::uninitCompetitions()
{
	for (uint i = 0; i < GetDelegatesFor.size(); ++i)
	{
		delete GetDelegatesFor[i];
	}
	GetDelegatesFor.clear();
}
FDataStream& operator<<(FDataStream& kStream, const MiniCompetitionTypes& data)
{
	kStream << (int)data;
	return kStream;
}
FDataStream& operator>>(FDataStream& kStream, MiniCompetitionTypes& data)
{
	int temp;
	kStream >> temp;
	data = (MiniCompetitionTypes)temp;
	return kStream;
}
FDataStream& operator <<(FDataStream& kStream, const CvCompetitionEntry& data)
{
	kStream << data.eType;
	kStream << data.ePlayer;
	kStream << data.iScore;
	kStream << data.iTempScore;
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, CvCompetitionEntry& data)
{
	kStream >> data.eType;
	kStream >> data.ePlayer;
	kStream >> data.iScore;
	kStream >> data.iTempScore;
	return kStream;
}
FDataStream& operator <<(FDataStream& kStream, const CvCompetition& data)
{
	kStream << data.m_eCompetitionType;
	kStream << data.m_entries;
	return kStream;
}
FDataStream& operator >>(FDataStream& kStream, CvCompetition& data)
{
	kStream >> data.m_eCompetitionType;
	kStream >> data.m_entries;
	return kStream;
}
CvCompetition::CvCompetition()
{

}
CvCompetition::CvCompetition(const int iNumPlayers, const MiniCompetitionTypes eCompetition)
{
	for (int i = 0; i < iNumPlayers; ++i) // add an entry for each player
		m_entries.push_back(CvCompetitionEntry((PlayerTypes)i, eCompetition));

	m_eCompetitionType = eCompetition;
}
PlayerTypes CvCompetition::GetPlayerOfRank(const int iRank) const
{
	if (iRank < 0 || iRank >= (int)m_entries.size())
		return NO_PLAYER;

	if (m_entries[iRank].iScore == INVALID_SCORE) // no score is always last place
		return NO_PLAYER;
	else
		return m_entries[iRank].ePlayer;
}
int CvCompetition::GetRankOfPlayer(const PlayerTypes ePlayer) const
{
	int result = m_entries.size() - 1;
	for (int i = 0; i < (int)m_entries.size(); ++i)
	{
		if (m_entries[i].ePlayer == ePlayer) // found player entry
		{
			//if (m_entries[i].iScore > INVALID_SCORE) // no score is always last place
			result = i;
			//else // automatically set up top
			//	result = m_entries.size() - 1;
			break;
		}
	}
	return result;
}
int CvCompetition::GetScoreOfPlayer(const PlayerTypes ePlayer) const
{
	int result = 0;
	for (int i = 0; i < (int)m_entries.size(); ++i)
	{
		if (m_entries[i].ePlayer == ePlayer) // found player entry
		{
			result = m_entries[i].iScore;
			break;
		}
	}
	return result;
}
bool compareEntries(const CvCompetitionEntry& lhs, const CvCompetitionEntry& rhs)
{
	if (lhs.iScore == rhs.iScore) // randomly determine a tie
	{
		unsigned long seed = 0;
		seed += 3456213594 * lhs.eType;
		seed += 98615 * lhs.ePlayer;
		seed += 321891373 * rhs.eType;
		seed += 96429789 * rhs.ePlayer;
		int randomTieResolution = GC.rand(100, "Competition Tie Resolution", NULL, seed);
		return (randomTieResolution < 50);
	}
	else // higher values go earlier in the array
		return lhs.iScore > rhs.iScore;
}
int CvCompetition::GetCompetitionWinnerScore() const
{
	return GetScoreOfPlayer(GetPlayerOfRank(0));
}
string CvCompetition::GetDescriptionShort() const
{
	return GetLocalizedText(GC.GetDelegatesFor[(int)m_eCompetitionType]->DescShort(GetCompetitionWinnerScore()).c_str());
}
string CvCompetition::GetDescriptionReward() const
{
	return GetLocalizedText(GC.GetDelegatesFor[(int)m_eCompetitionType]->DescReward(*this).c_str());
}
string CvCompetition::GetDescription(const PlayerTypes ePlayer) const
{
	return GetLocalizedText(GC.GetDelegatesFor[(int)m_eCompetitionType]->Desc(*this, ePlayer).c_str());
}
int CvCompetition::GetReward(const YieldTypes eType, const PlayerTypes ePlayer) const
{
	const int reward = GC.GetDelegatesFor[(int)m_eCompetitionType]->Reward(eType);
	if (ePlayer == NO_PLAYER)
		return reward;

	const int score = GetScoreOfPlayer(ePlayer);
	const int rank = GetRankOfPlayer(ePlayer);
	// first place with a valid score?
	if (score != INVALID_SCORE && rank == 0)
	{
		return reward;
	}
	// consider 2nd place?
	else
	{
		return 0;
	}
}
void CvCompetition::Update(bool shouldMoveToNextSession, int numTurnsPerSession)
{
	// update
	for (int i = 0; i < (int)m_entries.size(); ++i)
	{
		const CvPlayer& player = GET_PLAYER(m_entries[i].ePlayer);
		int score = 0; // no score for dead or minor civs
		if (player.isAlive() && player.isMajorCiv())
			score = GC.GetDelegatesFor[(int)m_eCompetitionType]->EvalScore(player);

		const int tempScore = m_entries[i].iTempScore;
		m_entries[i].iTempScore = tempScore + score;
		if (shouldMoveToNextSession)
		{
			const int calcScore = (m_entries[i].iTempScore * 10) / numTurnsPerSession;
			m_entries[i].iScore = calcScore;
			m_entries[i].iTempScore = 0;
			std::stable_sort(m_entries.begin(), m_entries.end(), compareEntries);
		}
	}
}






