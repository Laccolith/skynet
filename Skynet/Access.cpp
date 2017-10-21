#include "Access.h"

#include "DrawBuffer.h"
#include "SkynetPlayerTracker.h"
#include "SkynetUnitTracker.h"
#include "SkynetTerrainAnalyser.h"
#include "SkynetBaseTracker.h"
#include "SkynetResourceTracker.h"
#include "SkynetTaskManager.h"

Access::Access( Skynet & skynet )
	: m_skynet( skynet )
{
	m_draw_buffer = std::make_unique<DrawBuffer>( *this );
	m_player_tracker = std::make_unique<SkynetPlayerTracker>( *this );
	m_unit_tracker = std::make_unique<SkynetUnitTracker>( *this );
	m_terrain_analyser = std::make_unique<SkynetTerrainAnalyser>( *this );
	m_base_tracker = std::make_unique<SkynetBaseTracker>( *this );
	m_resource_tracker = std::make_unique<SkynetResourceTracker>( *this );
	m_task_manager = std::make_unique<SkynetTaskManager>( *this );
}

Access::~Access() = default;