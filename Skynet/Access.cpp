#include "Access.h"

#include "DrawBuffer.h"
#include "SkynetUnitTracker.h"
#include "SkynetTerrainAnalyser.h"
#include "SkynetBaseTracker.h"

Access::Access( Skynet & skynet )
	: m_skynet( skynet )
{
	m_draw_buffer = std::make_unique<DrawBuffer>( *this );
	m_unit_tracker = std::make_unique<SkynetUnitTracker>( *this );
	m_terrain_analyser = std::make_unique<SkynetTerrainAnalyser>( *this );
	m_base_tracker = std::make_unique<SkynetBaseTracker>( *this );
}

Access::~Access() = default;