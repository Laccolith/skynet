#pragma once

#include "CoreModule.h"
#include "ControlTask.h"
#include "BuildLocation.h"

#include "Types.h"

#include <memory>

class ControlTaskFactoryInterface : public CoreModule
{
public:
	ControlTaskFactoryInterface( Core & core ) : CoreModule( core, "ControlTaskFactory" ) {}

	virtual std::unique_ptr<ControlTask> createBuildControlTask( TaskPriority * priority, UnitType unit_type, BuildLocationType build_location_type = BuildLocationType::Base ) = 0;
	virtual std::unique_ptr<ControlTask> createResearchControlTask( TaskPriority * priority, TechType tech_type ) = 0;
	virtual std::unique_ptr<ControlTask> createUpgradeControlTask( TaskPriority * priority, UpgradeType upgrade_type, int upgrade_level ) = 0;
};