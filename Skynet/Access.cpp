#include "Access.h"

#include "SkynetUnitTracker.h"
#include "SkynetTerrainAnalyser.h"

Access::Access( Skynet & skynet )
	: m_skynet( skynet )
{
	m_unit_tracker = std::make_unique<SkynetUnitTracker>( *this );
	m_terrain_analyser = std::make_unique<SkynetTerrainAnalyser>( *this );
}

Access::~Access() = default;