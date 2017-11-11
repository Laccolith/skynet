#pragma once

#include "CoreModule.h"
#include "Messaging.h"

struct BuildOrderChanged
{
};

class BuildOrderManagerInterface : public CoreModule, public MessageReporter<BuildOrderChanged>
{
public:
	BuildOrderManagerInterface( Core & core ) : CoreModule( core, "BuildOrderManager" ) {}
};