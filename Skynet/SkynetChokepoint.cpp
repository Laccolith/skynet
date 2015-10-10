#include "SkynetChokepoint.h"

SkynetChokepoint::SkynetChokepoint( WalkPosition center, WalkPosition side1, WalkPosition side2, int clearance )
	: m_center( center )
	, m_sides( std::make_pair( side1, side2 ) )
	, m_clearance( clearance )
{
}

void SkynetChokepoint::draw( Color color ) const
{
	BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, m_center.x * 8 + 4, m_center.y * 8 + 4, m_sides.first.x * 8 + 4, m_sides.first.y * 8 + 4, color );
	BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, m_center.x * 8 + 4, m_center.y * 8 + 4, m_sides.second.x * 8 + 4, m_sides.second.y * 8 + 4, color );
}