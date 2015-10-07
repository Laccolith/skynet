#include "Skynet.h"

#include <BWAPI.h>

Skynet::Skynet()
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );
	BWAPI::Broodwar->printf( "Skynet is online." );
}

void Skynet::update()
{
}