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

	UnitManagerInterface & getUnitManager() { return m_core.getUnitManager(); }
	const UnitManagerInterface & getUnitManager() const { return m_core.getUnitManager(); }

	TerrainAnalyserInterface & getTerrainAnalyser() { return m_core.getTerrainAnalyser(); }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return m_core.getTerrainAnalyser(); }

	BaseTrackerInterface & getBaseTracker() { return m_core.getBaseTracker(); }
	const BaseTrackerInterface & getBaseTracker() const { return m_core.getBaseTracker(); }

	ResourceManagerInterface & getResourceManager() { return m_core.getResourceManager(); }
	const ResourceManagerInterface & getResourceManager() const { return m_core.getResourceManager(); }

	TaskManagerInterface & getTaskManager() { return m_core.getTaskManager(); }
	const TaskManagerInterface & getTaskManager() const { return m_core.getTaskManager(); }

	virtual ControlTaskFactoryInterface & getControlTaskFactory() { return m_core.getControlTaskFactory(); }
	virtual const ControlTaskFactoryInterface & getControlTaskFactory() const { return m_core.getControlTaskFactory(); }

	virtual BuildLocationManagerInterface & getBuildLocationManager() { return m_core.getBuildLocationManager(); }
	virtual const BuildLocationManagerInterface & getBuildLocationManager() const { return m_core.getBuildLocationManager(); }

private:
	Core & m_core;
};
