#include "SkynetTerrainAnalyser.h"

#include "Skynet.h"

SkynetTerrainAnalyser::SkynetTerrainAnalyser( Access & access )
	: TerrainAnalyserInterface( access )
{
	getSkynet().registerUpdateProcess( 1.0f, [this](){ update(); } );
}

void SkynetTerrainAnalyser::update()
{
	BWAPI::Broodwar->printf( "SkynetTerrainAnalyser updated." );
}
