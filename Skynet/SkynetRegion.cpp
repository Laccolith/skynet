#include "SkynetRegion.h"

#include "SkynetTerrainAnalyser.h"
#include "Chokepoint.h"

SkynetRegion::SkynetRegion( SkynetTerrainAnalyser & terrain_analyser, WalkPosition center, int clearance )
	: m_terrain_analyser( terrain_analyser )
	, m_center( center )
	, m_clearance( clearance )
	, m_size( 0 )
{}

bool SkynetRegion::isConnected( Region other ) const
{
	return m_terrain_analyser.getConnectivity( m_center ) == m_terrain_analyser.getConnectivity( other->getCenter() );
}

void SkynetRegion::draw( Color color ) const
{
	BWAPI::Broodwar->drawCircle( BWAPI::CoordinateType::Map, m_center.x * 8 + 4, m_center.y * 8 + 4, 64, color );
	for( auto chokepoint : m_chokepoints )
		BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, chokepoint->getCenter().x * 8 + 4, chokepoint->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, color );
}