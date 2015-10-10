#pragma once

#include "Types.h"

namespace UnitUtil
{
	bool hasAddon( UnitType type );
	bool isArmyUnit( UnitType type );
	bool isStaticDefense( UnitType type );
	bool unitProducesGround( UnitType type );
}