#pragma once

#include "CoreModule.h"

class SquadManagerInterface : public CoreModule
{
public:
	SquadManagerInterface( Core & core ) : CoreModule( core, "SquadManager" ) {}
};
