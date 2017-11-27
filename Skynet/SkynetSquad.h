#pragma once

#include "Types.h"
#include "UnitGroup.h"

class SkynetSquad
{
public:
	void update();

	void add_unit( Unit unit );
	void remove_unit( Unit unit );

private:
	UnitGroup m_units;
};