#include "Skynet.h"

#include <BWAPI.h>

Skynet::Skynet()
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );
}

void Skynet::update()
{
}