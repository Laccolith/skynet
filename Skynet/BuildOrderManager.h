#pragma once

#include "CoreModule.h"

class BuildOrderManagerInterface : public CoreModule
{
public:
	BuildOrderManagerInterface( Core & core ) : CoreModule( core, "BuildOrderManager" ) {}
};