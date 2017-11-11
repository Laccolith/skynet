#pragma once

#include "CoreModule.h"

#include "Types.h"
#include "LazyLogic.h"

class ProductionManagerInterface : public CoreModule
{
public:
	ProductionManagerInterface( Core & core ) : CoreModule( core, "ProductionManager" ) {}

	virtual void setArmyBuilding( bool enabled ) = 0;
	virtual void setProductionBuilding( bool enabled ) = 0;
	virtual void setTechBuilding( bool enabled ) = 0;

	virtual void addArmyUnit( UnitType unit_type, double weight, Condition unit_condition, Condition production_condition ) = 0;
};