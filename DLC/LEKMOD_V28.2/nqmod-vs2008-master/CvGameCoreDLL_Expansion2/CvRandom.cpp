/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */

#include "CvGameCoreDLLPCH.h"
#include "CvRandom.h"
#include "CvGlobals.h"
#include "FCallStack.h"
#include "FStlContainerSerialization.h"

#ifdef WIN32
#	include "Win32/FDebugHelper.h"
#endif//_WINPC

// include this after all other headers!
#include "LintFree.h"


#define RANDOM_A      (1103515245)
#define RANDOM_C      (12345)
#define RANDOM_SHIFT  (16)

CvRandom::CvRandom() :
	m_ulRandomSeed(0)
	, m_ulCallCount(0)
	, m_ulResetCount(0)
	, m_bSynchronous(false)
{
	reset();
}

CvRandom::CvRandom(bool extendedCallStackDebugging) :
	m_ulRandomSeed(0)
	, m_ulCallCount(0)
	, m_ulResetCount(0)
	, m_bSynchronous(true)
{
	extendedCallStackDebugging;
}

CvRandom::CvRandom(const CvRandom& source) :
	m_ulRandomSeed(source.m_ulRandomSeed)
	, m_ulCallCount(source.m_ulCallCount)
	, m_ulResetCount(source.m_ulResetCount)
	, m_bSynchronous(source.m_bSynchronous)
{

}

bool CvRandom::operator==(const CvRandom& source) const
{
	return(m_ulRandomSeed == source.m_ulRandomSeed);
}

bool CvRandom::operator!=(const CvRandom& source) const
{
	return !(*this == source);
}

CvRandom::~CvRandom()
{
	uninit();
}

void CvRandom::init(unsigned long ulSeed)
{
	//--------------------------------
	// Init saved data
	reset(ulSeed);

	//--------------------------------
	// Init non-saved data
}


void CvRandom::uninit()
{
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvRandom::reset(unsigned long ulSeed)
{
	//--------------------------------
	// Uninit class
	uninit();

	recordCallStack();
	m_ulRandomSeed = ulSeed;
	m_ulResetCount++;
}

unsigned short rngFromSeed(unsigned short usMaxExclusive, unsigned long seed)
{
	// do this 3 times, otherwise slight changes in seed seem to
	// not do enough and patterns in for loops appear
	seed = (RANDOM_A * seed) + RANDOM_C;
	seed = (RANDOM_A * seed) + RANDOM_C;
	seed = (RANDOM_A * seed) + RANDOM_C;
	unsigned short us = ((unsigned short)((((seed >> RANDOM_SHIFT) & MAX_UNSIGNED_SHORT) * ((unsigned long)usMaxExclusive)) / (MAX_UNSIGNED_SHORT + 1)));
	return us;
}

unsigned short CvRandom::getSafe(unsigned short usMaxExclusive, const unsigned long extraSeed) const
{
	return rngFromSeed(usMaxExclusive, m_ulRandomSeed + extraSeed);
}

void log(unsigned short usMaxExclusive, unsigned short newVal, unsigned long seed, const bool isSync, const char* pszLog)
{
	if (GC.getLogging())
	{
		int iRandLogging = GC.getRandLogging();
		if (iRandLogging > 0)
		{
			CvGame& kGame = GC.getGame();
			if (kGame.getTurnSlice() > 0 || ((iRandLogging & RAND_LOGGING_PREGAME_FLAG) != 0))
			{
				FILogFile* pLog = LOGFILEMGR.GetLog("RandCalls.csv", FILogFile::kDontTimeStamp, "Game Turn, Turn Slice, Range, Value, Seed, Instance, Type, Location\n");

				if (pLog)
				{
					char szOut[1024] = { 0 };
					sprintf_s(szOut, "%d, %d, %u, %u, %u, %8x, %s, %s\n", kGame.getGameTurn(), kGame.getTurnSlice(), (uint)usMaxExclusive, (uint)newVal, seed, (uint)0, isSync ? "sync" : "async", (pszLog != NULL) ? pszLog : "Unknown");
					pLog->Msg(szOut);
				}
			}
		}
	}
}



unsigned short CvRandom::get(unsigned short usMaxExclusive, const unsigned long extraSeed, const char* pszLog)
{
	unsigned short us = 0;

	if (extraSeed != MutateSeed)
	{
		const unsigned long totalSeed = m_ulRandomSeed + extraSeed;

		us = getSafe(usMaxExclusive, extraSeed);
		log(usMaxExclusive, us, totalSeed, m_bSynchronous, pszLog);
	}
	else
	{
		recordCallStack();
		m_ulCallCount++;
		unsigned long ulNewSeed = ((RANDOM_A * m_ulRandomSeed) + RANDOM_C);

		us = getSafe(usMaxExclusive, ulNewSeed);
		log(usMaxExclusive, us, ulNewSeed, m_bSynchronous, pszLog);

		m_ulRandomSeed = ulNewSeed;
	}
	return us;
}
//float CvRandom::getFloat(const unsigned long extraSeed)
//{
//	return (((float)(get(MAX_UNSIGNED_SHORT, extraSeed))) / ((float)MAX_UNSIGNED_SHORT));
//}
float CvRandom::getFloatSafe(const unsigned long extraSeed) const
{
	return (((float)(getSafe(MAX_UNSIGNED_SHORT, extraSeed))) / ((float)MAX_UNSIGNED_SHORT));
}

void CvRandom::reseed(unsigned long ulNewValue)
{
	recordCallStack();
	m_ulResetCount++;
	m_ulRandomSeed = ulNewValue;
}


unsigned long CvRandom::getSeed() const
{
	return m_ulRandomSeed;
}

unsigned long CvRandom::getCallCount() const
{
	return m_ulCallCount;
}

unsigned long CvRandom::getResetCount() const
{
	return m_ulResetCount;
}

void CvRandom::read(FDataStream& kStream)
{
	reset();

	// Version number to maintain backwards compatibility
	uint uiVersion;
	kStream >> uiVersion;
	kStream >> m_ulRandomSeed;
	kStream >> m_ulCallCount;
	kStream >> m_ulResetCount;
	bool b;
	kStream >> b;
}


void CvRandom::write(FDataStream& kStream) const
{
	// Current version number
	uint uiVersion = 1;
	kStream << uiVersion;
	kStream << m_ulRandomSeed;
	kStream << m_ulCallCount;
	kStream << m_ulResetCount;
	kStream << false;
}

void CvRandom::recordCallStack()
{
#ifdef _DEBUG
	if(m_bExtendedCallStackDebugging)
	{
		FDebugHelper& debugHelper = FDebugHelper::GetInstance();
		FCallStack callStack;
		debugHelper.GetCallStack(&callStack, 1, 8);
		m_kCallStacks.push_back(callStack);
		m_seedHistory.push_back(m_ulRandomSeed);
	}
#endif//_DEBUG
}

void CvRandom::resolveCallStacks() const
{
#ifdef _DEBUG
	std::vector<FCallStack>::const_iterator i;
	for(i = m_kCallStacks.begin() + m_resolvedCallStacks.size(); i != m_kCallStacks.end(); ++i)
	{
		const FCallStack callStack = *i;
		std::string stackTrace = callStack.toString(true);
		m_resolvedCallStacks.push_back(stackTrace);
	}
#endif//_DEBUG
}

const std::vector<std::string>& CvRandom::getResolvedCallStacks() const
{
	static std::vector<std::string> empty;
	return empty;
}

const std::vector<unsigned long>& CvRandom::getSeedHistory() const
{
	static std::vector<unsigned long> empty;
	return empty;
}

bool CvRandom::callStackDebuggingEnabled() const
{
	return false;
}

void CvRandom::setCallStackDebuggingEnabled(bool enabled)
{
#ifdef _DEBUG
	m_bExtendedCallStackDebugging = enabled;
#endif//_DEBUG
	enabled;
}

void CvRandom::clearCallstacks()
{
#ifdef _DEBUG
	m_kCallStacks.clear();
	m_seedHistory.clear();
	m_resolvedCallStacks.clear();
#endif//_DEBUG
}
FDataStream& operator<<(FDataStream& saveTo, const CvRandom& readFrom)
{
	readFrom.write(saveTo);
	return saveTo;
}

FDataStream& operator>>(FDataStream& loadFrom, CvRandom& writeTo)
{
	writeTo.read(loadFrom);
	return loadFrom;
}
