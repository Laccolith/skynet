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
	DrawBuffer & getDrawBuffer() { return m_access.getDrawBuffer(); }

	UnitTrackerInterface & getUnitTracker() { return m_access.getUnitTracker(); }
	const UnitTrackerInterface & getUnitTracker() const { return m_access.getUnitTracker(); }

	TerrainAnalyserInterface & getTerrainAnalyser() { return m_access.getTerrainAnalyser(); }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return m_access.getTerrainAnalyser(); }

	Skynet & getSkynet() { return m_access.getSkynet(); }
	const Skynet & getSkynet() const { return m_access.getSkynet(); }

private:
	std::string m_name;

	Access & m_access;

	bool m_debug = false;
};