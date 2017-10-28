#pragma once

#include "CoreModule.h"

#include "BuildLocation.h"

#include <memory>

class BuildLocationManagerInterface : public CoreModule
{
public:
	BuildLocationManagerInterface( Core & core ) : CoreModule( core, "BuildLocationManager" ) {}

	virtual std::unique_ptr<BuildLocation> createBuildLocation( UnitType unit_type ) = 0;
};
