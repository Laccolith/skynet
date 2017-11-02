#include "SkynetChokepoint.h"

#include "Region.h"

SkynetChokepoint::SkynetChokepoint( int id, WalkPosition center, WalkPosition side1, WalkPosition side2, int clearance )
	: m_id( id)
	, m_center( center )
	, m_sides( std::make_pair( side1, side2 ) )
	, m_clearance( clearance )
{
}

void SkynetChokepoint::draw( Color color ) const
{
	BWAPI::Broodwar->drawLineMap( m_center.x * 8 + 4, m_center.y * 8 + 4, m_sides.first.x * 8 + 4, m_sides.first.y * 8 + 4, color );
	BWAPI::Broodwar->drawLineMap( m_center.x * 8 + 4, m_center.y * 8 + 4, m_sides.second.x * 8 + 4, m_sides.second.y * 8 + 4, color );

	if( m_regions.first )
	{
		BWAPI::Broodwar->drawLineMap( m_regions.first->getCenter().x * 8 + 4, m_regions.first->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, Colors::Blue );
		PositionFloat direction = (PositionFloat( m_regions.first->getCenter() ) + PositionFloat( 4, 4 )) - (PositionFloat( m_center ) + PositionFloat( 4, 4 ));
		normalise( direction );
		direction *= 48;
		BWAPI::Broodwar->drawTextMap( Position( m_center ) + Position( direction ), "Region 1" );
	}

	if( m_regions.second )
	{
		BWAPI::Broodwar->drawLineMap( m_regions.second->getCenter().x * 8 + 4, m_regions.second->getCenter().y * 8 + 4, m_center.x * 8 + 4, m_center.y * 8 + 4, Colors::Blue );
		PositionFloat direction = (PositionFloat( m_regions.second->getCenter() ) + PositionFloat( 4, 4 )) - (PositionFloat( m_center ) + PositionFloat( 4, 4 ));
		normalise( direction );
		direction *= 48;
		BWAPI::Broodwar->drawTextMap( Position( m_center ) + Position( direction ), "Region 2" );
	}
}