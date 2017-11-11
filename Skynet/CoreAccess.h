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

	DrawBuffer & getDrawBuffer() const { return m_core.getDrawBuffer(); }

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

	BaseManagerInterface & getBaseManager() { return m_core.getBaseManager(); }
	const BaseManagerInterface & getBaseManager() const { return m_core.getBaseManager(); }

	ResourceManagerInterface & getResourceManager() { return m_core.getResourceManager(); }
	const ResourceManagerInterface & getResourceManager() const { return m_core.getResourceManager(); }

	TaskManagerInterface & getTaskManager() { return m_core.getTaskManager(); }
	const TaskManagerInterface & getTaskManager() const { return m_core.getTaskManager(); }

	ControlTaskFactoryInterface & getControlTaskFactory() { return m_core.getControlTaskFactory(); }
	const ControlTaskFactoryInterface & getControlTaskFactory() const { return m_core.getControlTaskFactory(); }

	BuildLocationManagerInterface & getBuildLocationManager() { return m_core.getBuildLocationManager(); }
	const BuildLocationManagerInterface & getBuildLocationManager() const { return m_core.getBuildLocationManager(); }

	BuildOrderManagerInterface & getBuildOrderManager() { return m_core.getBuildOrderManager(); }
	const BuildOrderManagerInterface & getBuildOrderManager() const { return m_core.getBuildOrderManager(); }

	SupplyManagerInterface & getSupplyManager() { return m_core.getSupplyManager(); }
	const SupplyManagerInterface & getSupplyManager() const { return m_core.getSupplyManager(); }

private:
	Core & m_core;
};
