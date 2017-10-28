#pragma once

#include "CoreModule.h"
#include "ControlTask.h"

#include "Types.h"

#include <memory>

class ControlTaskFactoryInterface : public CoreModule
{
public:
	ControlTaskFactoryInterface( Core & core ) : CoreModule( core, "ControlTaskFactory" ) {}

	virtual std::unique_ptr<ControlTask> createBuildControlTask( UnitType unit_type ) = 0;
};