#pragma once

#include "Access.h"

#include <string>

class SkynetInterface
{
public:
	SkynetInterface( Access & access, std::string name );
	virtual ~SkynetInterface() {}

	void setDebug( bool enabled ) { m_debug = enabled; }
	bool isDebugging() const { return m_debug; }

	const std::string &getName() const { return m_name; }

protected:
	UnitTrackerInterface & UnitTracker() { return m_access.UnitTracker(); }
	const UnitTrackerInterface & UnitTracker() const { return m_access.UnitTracker(); }

	TerrainAnalyserInterface & TerrainAnalyser() { return m_access.TerrainAnalyser(); }
	const TerrainAnalyserInterface & TerrainAnalyser() const { return m_access.TerrainAnalyser(); }

	Skynet & getSkynet() { return m_access.getSkynet(); }
	const Skynet & getSkynet() const { return m_access.getSkynet(); }

private:
	std::string m_name;

	Access & m_access;

	bool m_debug = false;
};