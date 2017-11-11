#pragma once

#include <functional>

class CoreModule;
class DrawBuffer;
class PlayerTrackerInterface;
class UnitTrackerInterface;
class UnitManagerInterface;
class TerrainAnalyserInterface;
class BaseTrackerInterface;
class BaseManagerInterface;
class ResourceManagerInterface;
class TaskManagerInterface;
class ControlTaskFactoryInterface;
class BuildLocationManagerInterface;
class BuildOrderManagerInterface;
class SupplyManagerInterface;

class Core
{
public:
	virtual void update() = 0;

	virtual void registerModule( CoreModule & inter ) = 0;
	virtual void registerUpdateProcess( float priority, std::function<void()> update_function ) = 0;

	virtual DrawBuffer & getDrawBuffer() const = 0;

	virtual PlayerTrackerInterface & getPlayerTracker() = 0;
	virtual const PlayerTrackerInterface & getPlayerTracker() const = 0;

	virtual UnitTrackerInterface & getUnitTracker() = 0;
	virtual const UnitTrackerInterface & getUnitTracker() const = 0;

	virtual UnitManagerInterface & getUnitManager() = 0;
	virtual const UnitManagerInterface & getUnitManager() const = 0;

	virtual TerrainAnalyserInterface & getTerrainAnalyser() = 0;
	virtual const TerrainAnalyserInterface & getTerrainAnalyser() const = 0;

	virtual BaseTrackerInterface & getBaseTracker() = 0;
	virtual const BaseTrackerInterface & getBaseTracker() const = 0;

	virtual BaseManagerInterface & getBaseManager() = 0;
	virtual const BaseManagerInterface & getBaseManager() const = 0;

	virtual ResourceManagerInterface & getResourceManager() = 0;
	virtual const ResourceManagerInterface & getResourceManager() const = 0;

	virtual TaskManagerInterface & getTaskManager() = 0;
	virtual const TaskManagerInterface & getTaskManager() const = 0;

	virtual ControlTaskFactoryInterface & getControlTaskFactory() = 0;
	virtual const ControlTaskFactoryInterface & getControlTaskFactory() const = 0;

	virtual BuildLocationManagerInterface & getBuildLocationManager() = 0;
	virtual const BuildLocationManagerInterface & getBuildLocationManager() const = 0;

	virtual BuildOrderManagerInterface & getBuildOrderManager() = 0;
	virtual const BuildOrderManagerInterface & getBuildOrderManager() const = 0;

	virtual SupplyManagerInterface & getSupplyManager() = 0;
	virtual const SupplyManagerInterface & getSupplyManager() const = 0;

	~Core() {}
};
