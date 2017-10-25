#pragma once

#include "CoreModule.h"

class BaseManagerInterface : public CoreModule
{
public:
	BaseManagerInterface( Core & core ) : CoreModule( core, "BaseManager" ) {}
};