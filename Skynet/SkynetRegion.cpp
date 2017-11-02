#include "SkynetRegion.h"

#include "SkynetTerrainAnalyser.h"
#include "Chokepoint.h"

SkynetRegion::SkynetRegion( int id, WalkPosition center, int clearance, int connectivity )
	: m_id( id )
	, m_center( center )
	, m_clearance( clearance )
	, m_connectivity( connectivity )
	, m_size( 0 )
{
}

void SkynetRegion::draw( Color color ) const
{
	BWAPI::Broodwar->drawCircle( BWAPI::CoordinateType::Map, m_center.x * 8 + 4, m_center.y * 8 + 4, 64, color );
	for( auto chokepoint : m_chokepoints )
		BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, chokepoint->getCenter().x * 8 + 4, chokepoint->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, color );
}