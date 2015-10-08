#include "SkynetUnitTracker.h"

#include "Skynet.h"

SkynetUnitTracker::SkynetUnitTracker( Access & access )
	: UnitTrackerInterface( access )
{
	getSkynet().registerUpdateProcess( 0.0f, [this]() { update(); } );
}

Unit SkynetUnitTracker::getUnit( BWAPI::Unit unit ) const
{
	auto it = m_units.find( unit );
	if( it != m_units.end() )
		return it->second;

	return nullptr;
}

UnitGroup SkynetUnitTracker::getUnitGroup( const BWAPI::Unitset & units ) const
{
	UnitGroup return_units;

	for( BWAPI::Unit unit : units )
	{
		auto it = m_units.find( unit );
		if( it != m_units.end() )
			return_units.insert( it->second );
	}

	return return_units;
}

const UnitGroup & SkynetUnitTracker::getAllUnits( UnitType type, Player player ) const
{
	auto it = m_player_to_type_to_units.find( player );
	if( it == m_player_to_type_to_units.end() )
		return empty_unit_group;

	auto it2 = it->second.find( type );
	if( it2 == it->second.end() )
		return empty_unit_group;

	return it2->second;
}

const UnitGroup & SkynetUnitTracker::getAllUnits( Player player ) const
{
	auto it = m_player_to_units.find( player );
	if( it == m_player_to_units.end() )
		return empty_unit_group;

	return it->second;
}

UnitGroup SkynetUnitTracker::getAllEnemyUnits( Player player ) const
{
	UnitGroup enemies;
	for( Player enemy : BWAPI::Broodwar->getPlayers() )
	{
		if( player->isEnemy( enemy ) )
			enemies += getAllUnits( enemy );
	}
	return enemies;
}

UnitGroup SkynetUnitTracker::getAllEnemyUnits( UnitType type, Player player ) const
{
	UnitGroup enemies;
	for( BWAPI::Player enemy : BWAPI::Broodwar->getPlayers() )
	{
		if( player->isEnemy( enemy ) )
			enemies += getAllUnits( type, enemy );
	}
	return enemies;
}

void SkynetUnitTracker::update()
{
	BWAPI::Broodwar->printf( "SkynetUnitTracker updated." );
}
