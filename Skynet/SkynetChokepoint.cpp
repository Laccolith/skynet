#include "SkynetChokepoint.h"

#include "Region.h"

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

	if( m_regions.first )
	{
		BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, m_regions.first->getCenter().x * 8 + 4, m_regions.first->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, color );
		Position middle = ( ( Position( m_regions.first->getCenter() ) + Position( 4, 4 ) ) + ( Position( m_center ) + Position( 4, 4 ) ) ) / 2;
		BWAPI::Broodwar->drawText( BWAPI::CoordinateType::Map, middle.x, middle.y, "Region 1" );
	}

	if( m_regions.second )
	{
		BWAPI::Broodwar->drawLine( BWAPI::CoordinateType::Map, m_regions.second->getCenter().x * 8 + 4, m_regions.second->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, color );
		Position middle = ((Position( m_regions.second->getCenter() ) + Position( 4, 4 )) + (Position( m_center ) + Position( 4, 4 ))) / 2;
		BWAPI::Broodwar->drawText( BWAPI::CoordinateType::Map, middle.x, middle.y, "Region 1" );
	}
}