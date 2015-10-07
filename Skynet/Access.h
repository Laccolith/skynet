#pragma once

#include <memory>
#include <functional>

#include <BWAPI.h>

using BWAPI::TilePosition;
using BWAPI::WalkPosition;
using BWAPI::Position;

class UnitTrackerInterface;
class TerrainAnalyserInterface;

class Access
{
public:
	Access();
	~Access();

	UnitTrackerInterface & UnitTracker() { return *m_unit_tracker; }
	const UnitTrackerInterface & UnitTracker() const { return *m_unit_tracker; }

	TerrainAnalyserInterface & TerrainAnalyser() { return *m_terrain_analyser; }
	const TerrainAnalyserInterface & TerrainAnalyser() const { return *m_terrain_analyser; }

private:
	std::unique_ptr<UnitTrackerInterface> m_unit_tracker;
	std::unique_ptr<TerrainAnalyserInterface> m_terrain_analyser;
};