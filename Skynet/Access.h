#pragma once

#include <memory>
#include <functional>

#include "Types.h"

class DrawBuffer;
class PlayerTrackerInterface;
class UnitTrackerInterface;
class TerrainAnalyserInterface;
class BaseTrackerInterface;
class ResourceTrackerInterface;
class TaskManagerInterface;

class Skynet;
class Access
{
public:
	Access( Skynet & skynet );
	~Access();

	DrawBuffer & getDrawBuffer() { return *m_draw_buffer; }

	PlayerTrackerInterface & getPlayerTracker() { return *m_player_tracker; }
	const PlayerTrackerInterface & getPlayerTracker() const { return *m_player_tracker; }

	UnitTrackerInterface & getUnitTracker() { return *m_unit_tracker; }
	const UnitTrackerInterface & getUnitTracker() const { return *m_unit_tracker; }

	TerrainAnalyserInterface & getTerrainAnalyser() { return *m_terrain_analyser; }
	const TerrainAnalyserInterface & getTerrainAnalyser() const { return *m_terrain_analyser; }

	BaseTrackerInterface & getBaseTracker() { return *m_base_tracker; }
	const BaseTrackerInterface & getBaseTracker() const { return *m_base_tracker; }

	ResourceTrackerInterface & getResourceTracker() { return *m_resource_tracker; }
	const ResourceTrackerInterface & getResourceTracker() const { return *m_resource_tracker; }

	TaskManagerInterface & getTaskManager() { return *m_task_manager; }
	const TaskManagerInterface & getTaskManager() const { return *m_task_manager; }

	Skynet & getSkynet() { return m_skynet; }
	const Skynet & getSkynet() const { return m_skynet; }

private:
	Skynet & m_skynet;

	std::unique_ptr<DrawBuffer> m_draw_buffer;
	std::unique_ptr<PlayerTrackerInterface> m_player_tracker;
	std::unique_ptr<UnitTrackerInterface> m_unit_tracker;
	std::unique_ptr<TerrainAnalyserInterface> m_terrain_analyser;
	std::unique_ptr<BaseTrackerInterface> m_base_tracker;
	std::unique_ptr<ResourceTrackerInterface> m_resource_tracker;
	std::unique_ptr<TaskManagerInterface> m_task_manager;
};