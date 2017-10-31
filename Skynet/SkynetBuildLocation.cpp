#include "SkynetBuildLocation.h"

#include "SkynetBuildLocationManager.h"

SkynetBuildLocation::SkynetBuildLocation( SkynetBuildLocationManager & manager, UnitType unit_type )
	: m_manager( manager )
	, m_unit_type( unit_type )
{
}

SkynetBuildLocation::~SkynetBuildLocation()
{
	m_manager.onBuildLocationDestroyed( this );
}

int SkynetBuildLocation::calculatePosition( int earliest_time )
{
	std::tie( earliest_time, m_chosen_position ) = m_manager.choosePosition( earliest_time, m_unit_type );
	return earliest_time;
}

void SkynetBuildLocation::reservePosition( int time )
{
	m_manager.reservePosition( time, m_chosen_position, m_unit_type );
}

void SkynetBuildLocation::freeReservation()
{
	m_manager.freeReservation( m_chosen_position, m_unit_type );
}

TilePosition SkynetBuildLocation::getTilePosition() const
{
	return m_chosen_position;
}

UnitType SkynetBuildLocation::getUnitType() const
{
	return m_unit_type;
}

void SkynetBuildLocation::drawInfo()
{
	if( m_chosen_position != TilePositions::None )
	{
		Position top_left( m_chosen_position );
		Position bottom_right( m_chosen_position + m_unit_type.tileSize() );

		BWAPI::Broodwar->drawBoxMap( top_left, bottom_right, Colors::Orange );
		BWAPI::Broodwar->drawTextMap( bottom_right.x, top_left.y, "%s", m_unit_type.getName().c_str() );
	}
}
