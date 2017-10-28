#include "SkynetBaseLocation.h"

#include "Unit.h"

SkynetBaseLocation::SkynetBaseLocation( TilePosition build_location, Region region, const UnitGroup & resources )
	: m_center_position( Position( build_location ) + (Position( UnitTypes::Protoss_Nexus.tileSize() ) / 2) )
	, m_build_location( build_location )
	, m_region( region )
{
	addResources( resources );
}

void SkynetBaseLocation::addResources( const UnitGroup & resources )
{
	for( Unit resource : resources )
	{
		if( resource->getType().isMineralField() )
			m_minerals.insert( resource );
		else
			m_geysers.insert( resource );
	}
}

void SkynetBaseLocation::draw( Color color ) const
{
	BWAPI::Broodwar->drawCircleMap( m_center_position, 78, color, false );
	BWAPI::Broodwar->drawBoxMap( Position( m_build_location ), Position( m_build_location + UnitTypes::Protoss_Nexus.tileSize() ), color, false );

	for( Unit mineral : m_minerals )
	{
		BWAPI::Broodwar->drawCircleMap( mineral->getPosition(), 32, BWAPI::Colors::Blue );
		BWAPI::Broodwar->drawLineMap( mineral->getPosition(), m_center_position, BWAPI::Colors::Blue );
	}

	for( Unit geyser : m_geysers )
	{
		BWAPI::Broodwar->drawCircleMap( geyser->getPosition(), 32, BWAPI::Colors::Green );
		BWAPI::Broodwar->drawLineMap( geyser->getPosition(), m_center_position, BWAPI::Colors::Green );
	}
}