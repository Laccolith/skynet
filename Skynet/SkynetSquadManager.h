#pragma once

#include "SquadManager.h"

class SkynetSquadManager : public SquadManagerInterface
{
public:
	SkynetSquadManager( Core & core );

	void update();
};
