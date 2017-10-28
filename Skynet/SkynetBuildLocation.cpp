#include "SkynetBuildLocation.h"

#include "SkynetBuildLocationManager.h"

SkynetBuildLocation::SkynetBuildLocation( SkynetBuildLocationManager & manager, UnitType unit_type )
	: m_manager( manager )
	, m_unit_type( unit_type )
{
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

Position SkynetBuildLocation::getPosition() const
{
	return Position( m_chosen_position ) + Position( m_unit_type.tileWidth() * 16, m_unit_type.tileHeight() * 16 );
}

TilePosition SkynetBuildLocation::getTilePosition() const
{
	return m_chosen_position;
}
