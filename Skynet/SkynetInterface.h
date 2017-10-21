#pragma once

#include "Access.h"

#include <string>

class SkynetInterface
{
public:
	SkynetInterface( Access & access, std::string name );
	virtual ~SkynetInterface() {}

	virtual void debugCommand( const std::string & str ) {}

	const std::string &getName() const { return m_name; }

protected:
	DrawBuffer & getDrawBuffer() { return m_access.getDrawBuffer(); }

	PlayerTrackerInterface & getPlayerTracker() { return m_access.getPlayerTracker(); }
	const PlayerTrackerInterface & getPlayerTracker() const { return m_access.getPlayerTracker(); }

	UnitTrackerInterface & getUnitTracker() { return m_access.getUnitTracker(); }
	const UnitTrackerInterface & getUnitTracker() const { return m_access.getUnitTracker(); }

	TerrainAnalyserInterface & getTerrainAnalyser() { return m_access.getTerrainAnalyser(); }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return m_access.getTerrainAnalyser(); }

	BaseTrackerInterface & getBaseTracker() { return m_access.getBaseTracker(); }
	const BaseTrackerInterface & getBaseTracker() const { return m_access.getBaseTracker(); }

	ResourceTrackerInterface & getResourceTracker() { return m_access.getResourceTracker(); }
	const ResourceTrackerInterface & getResourceTracker() const { return m_access.getResourceTracker(); }

	TaskManagerInterface & getTaskManager() { return m_access.getTaskManager(); }
	const TaskManagerInterface & getTaskManager() const { return m_access.getTaskManager(); }

	Skynet & getSkynet() { return m_access.getSkynet(); }
	const Skynet & getSkynet() const { return m_access.getSkynet(); }

private:
	std::string m_name;

	Access & m_access;
};

#include "SmartEnum.h"
#include <bitset>
#define DEFINE_DEBUGGING_INTERFACE( ... ) \
	enum class Debug : unsigned int { __VA_ARGS__, Count }; \
	std::bitset<unsigned int(Debug::Count)> m_debug_flags; \
	\
	bool isDebugging( Debug d ) const \
	{ \
		return m_debug_flags[(unsigned int)d]; \
	} \
	void setDebugging( Debug d, bool enabled ) \
	{ \
		m_debug_flags[(unsigned int)d] = enabled; \
	} \
	\
	void debugCommand( const std::string & str ) override \
	{ \
		static const auto names = SmartEnum::makeEnumNames<unsigned int>( #__VA_ARGS__ ); \
		for( unsigned int i = 0; i < names.size(); ++i ) \
		{ \
			if( str == names[i] ) \
			{ \
				m_debug_flags[i] = !m_debug_flags[i]; \
				return; \
			} \
		} \
		\
		BWAPI::Broodwar->printf( "%s is not a debug command.", str.c_str() ); \
	}