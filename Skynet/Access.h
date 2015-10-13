#pragma once

#include <memory>
#include <functional>

#include "Types.h"

class DrawBuffer;
class UnitTrackerInterface;
class TerrainAnalyserInterface;

class Skynet;
class Access
{
public:
	Access( Skynet & skynet );
	~Access();

	DrawBuffer & getDrawBuffer() { return *m_draw_buffer; }

	UnitTrackerInterface & getUnitTracker() { return *m_unit_tracker; }
	const UnitTrackerInterface & getUnitTracker() const { return *m_unit_tracker; }

	TerrainAnalyserInterface & getTerrainAnalyser() { return *m_terrain_analyser; }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return *m_terrain_analyser; }

	Skynet & getSkynet() { return m_skynet; }
	const Skynet & getSkynet() const { return m_skynet; }

private:
	Skynet & m_skynet;

	std::unique_ptr<DrawBuffer> m_draw_buffer;
	std::unique_ptr<UnitTrackerInterface> m_unit_tracker;
	std::unique_ptr<TerrainAnalyserInterface> m_terrain_analyser;
};