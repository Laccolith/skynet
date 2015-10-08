#pragma once

#include <memory>
#include <functional>

#include "Types.h"

class UnitTrackerInterface;
class TerrainAnalyserInterface;

class Skynet;
class Access
{
public:
	Access( Skynet & skynet );
	~Access();

	UnitTrackerInterface & UnitTracker() { return *m_unit_tracker; }
	const UnitTrackerInterface & UnitTracker() const { return *m_unit_tracker; }

	TerrainAnalyserInterface & TerrainAnalyser() { return *m_terrain_analyser; }
	const TerrainAnalyserInterface & TerrainAnalyser() const { return *m_terrain_analyser; }

	Skynet & getSkynet() { return m_skynet; }
	const Skynet & getSkynet() const { return m_skynet; }

private:
	Skynet & m_skynet;

	std::unique_ptr<UnitTrackerInterface> m_unit_tracker;
	std::unique_ptr<TerrainAnalyserInterface> m_terrain_analyser;
};