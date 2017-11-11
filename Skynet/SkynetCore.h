#pragma once

#include "Core.h"

#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

class ControlTask;
class SkynetCore : public Core
{
public:
	SkynetCore();
	~SkynetCore();

	void update() override;

	void registerModule( CoreModule & inter ) override;
	void registerUpdateProcess( float priority, std::function<void()> update_function ) override;

	DrawBuffer & getDrawBuffer() const override { return *m_draw_buffer; }

	PlayerTrackerInterface & getPlayerTracker() override { return *m_player_tracker; }
	const PlayerTrackerInterface & getPlayerTracker() const override { return *m_player_tracker; }

	UnitTrackerInterface & getUnitTracker() override { return *m_unit_tracker; }
	const UnitTrackerInterface & getUnitTracker() const override { return *m_unit_tracker; }

	UnitManagerInterface & getUnitManager() override { return *m_unit_manager; }
	const UnitManagerInterface & getUnitManager() const override { return *m_unit_manager; }

	TerrainAnalyserInterface & getTerrainAnalyser() override { return *m_terrain_analyser; }
	const TerrainAnalyserInterface & getTerrainAnalyser() const override { return *m_terrain_analyser; }

	BaseTrackerInterface & getBaseTracker() override { return *m_base_tracker; }
	const BaseTrackerInterface & getBaseTracker() const override { return *m_base_tracker; }

	BaseManagerInterface & getBaseManager() override { return *m_base_manager; }
	const BaseManagerInterface & getBaseManager() const override { return *m_base_manager; }

	ResourceManagerInterface & getResourceManager() override { return *m_resource_tracker; }
	const ResourceManagerInterface & getResourceManager() const override { return *m_resource_tracker; }

	TaskManagerInterface & getTaskManager() override { return *m_task_manager; }
	const TaskManagerInterface & getTaskManager() const override { return *m_task_manager; }

	ControlTaskFactoryInterface & getControlTaskFactory() override { return *m_control_task_factory; }
	const ControlTaskFactoryInterface & getControlTaskFactory() const override { return *m_control_task_factory; }

	BuildLocationManagerInterface & getBuildLocationManager() override { return *m_build_location_manager; }
	const BuildLocationManagerInterface & getBuildLocationManager() const override { return *m_build_location_manager; }

	BuildOrderManagerInterface & getBuildOrderManager() override { return *m_build_order_manager; }
	const BuildOrderManagerInterface & getBuildOrderManager() const override { return *m_build_order_manager; }

	SupplyManagerInterface & getSupplyManager() override { return *m_supply_manager; }
	const SupplyManagerInterface & getSupplyManager() const override { return *m_supply_manager; }

	ProductionManagerInterface & getProductionManager() override { return *m_production_manager; }
	const ProductionManagerInterface & getProductionManager() const override { return *m_production_manager; }

private:
	bool m_in_startup = true;

	std::unordered_map<std::string, CoreModule*> m_interfaces;
	std::vector<std::pair<float, std::function<void()>>> m_update_processes;

	std::unique_ptr<DrawBuffer> m_draw_buffer;
	std::unique_ptr<PlayerTrackerInterface> m_player_tracker;
	std::unique_ptr<UnitTrackerInterface> m_unit_tracker;
	std::unique_ptr<UnitManagerInterface> m_unit_manager;
	std::unique_ptr<TerrainAnalyserInterface> m_terrain_analyser;
	std::unique_ptr<BaseTrackerInterface> m_base_tracker;
	std::unique_ptr<TaskManagerInterface> m_task_manager;
	std::unique_ptr<ControlTaskFactoryInterface> m_control_task_factory;
	std::unique_ptr<BaseManagerInterface> m_base_manager;
	std::unique_ptr<ResourceManagerInterface> m_resource_tracker;
	std::unique_ptr<BuildLocationManagerInterface> m_build_location_manager;
	std::unique_ptr<BuildOrderManagerInterface> m_build_order_manager;
	std::unique_ptr<SupplyManagerInterface> m_supply_manager;
	std::unique_ptr<ProductionManagerInterface> m_production_manager;
};
