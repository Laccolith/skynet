#pragma once

#include "CoreModule.h"

class SupplyManagerInterface : public CoreModule
{
public:
	SupplyManagerInterface( Core & core ) : CoreModule( core, "SupplyManager" ) {}

	virtual void setBuilding( bool enabled ) = 0;
};