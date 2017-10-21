#pragma once

#include "Core.h"

class CoreAccess
{
public:
	CoreAccess( Core & core )
		: m_core( core )
	{
	}

	CoreAccess( CoreAccess & core_access ) = default;

	DrawBuffer & getDrawBuffer() { return m_core.getDrawBuffer(); }

	PlayerTrackerInterface & getPlayerTracker() { return m_core.getPlayerTracker(); }
	const PlayerTrackerInterface & getPlayerTracker() const { return m_core.getPlayerTracker(); }

	UnitTrackerInterface & getUnitTracker() { return m_core.getUnitTracker(); }
	const UnitTrackerInterface & getUnitTracker() const { return m_core.getUnitTracker(); }

	TerrainAnalyserInterface & getTerrainAnalyser() { return m_core.getTerrainAnalyser(); }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return m_core.getTerrainAnalyser(); }

	BaseTrackerInterface & getBaseTracker() { return m_core.getBaseTracker(); }
	const BaseTrackerInterface & getBaseTracker() const { return m_core.getBaseTracker(); }

	ResourceTrackerInterface & getResourceTracker() { return m_core.getResourceTracker(); }
	const ResourceTrackerInterface & getResourceTracker() const { return m_core.getResourceTracker(); }

	TaskManagerInterface & getTaskManager() { return m_core.getTaskManager(); }
	const TaskManagerInterface & getTaskManager() const { return m_core.getTaskManager(); }

private:
	Core & m_core;
};
