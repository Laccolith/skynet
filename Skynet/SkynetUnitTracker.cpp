#include "SkynetUnitTracker.h"

#include "Skynet.h"

SkynetUnitTracker::SkynetUnitTracker( Access & access )
	: UnitTrackerInterface( access )
{
	getSkynet().registerUpdateProcess( 0.0f, [this]() { update(); } );
}

Unit SkynetUnitTracker::getUnit( BWAPI::Unit unit ) const
{
	auto it = m_bwapi_units.find( unit );
	if( it != m_bwapi_units.end() )
		return it->second.get();

	return nullptr;
}

UnitGroup SkynetUnitTracker::getUnitGroup( const BWAPI::Unitset & units ) const
{
	UnitGroup return_units;

	for( BWAPI::Unit unit : units )
	{
		auto it = m_bwapi_units.find( unit );
		if( it != m_bwapi_units.end() )
			return_units.insert( it->second.get() );
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
	// We only keep dead units for one frame
	m_dead_units.clear();

	for( const Event &event : BWAPI::Broodwar->getEvents() )
	{
		if( event.getType() == Events::UnitDiscover )
			onUnitDiscover( event.getUnit() );
	}

	for( const Event &event : BWAPI::Broodwar->getEvents() )
	{
		if( event.getType() == Events::UnitDestroy )
			onUnitDestroy( event.getUnit() );
	}

	for( auto & unit : m_bwapi_units )
		updateUnit( unit.second.get() );

	if( isDebugging( Debug::Default ) )
	{
		for( auto unit : m_all_units )
			unit->drawUnitPosition();
	}
}

void SkynetUnitTracker::onUnitDiscover( BWAPI::Unit unit )
{
	auto & new_unit = m_bwapi_units[unit];

	if( new_unit )
		return;

	/*Unit prediction = mAccess.getUnitPredictor().onNewUnit( unit );
	if( prediction )
	{
		new_unit = prediction;
		return;
	}*/
	
	new_unit = std::make_unique<SkynetUnit>( unit, *this );

	onDiscover( new_unit.get() );
}

void SkynetUnitTracker::onUnitDestroy( BWAPI::Unit unit )
{
	auto it = m_bwapi_units.find( unit );
	if( it != m_bwapi_units.end() )
	{
		onDestroy( it->second );
		m_bwapi_units.erase( it );
	}
}

void SkynetUnitTracker::onDiscover( Unit unit )
{
	m_player_to_type_to_units[unit->getPlayer()][unit->getType()].insert( unit );
	m_player_to_units[unit->getPlayer()].insert( unit );
	m_all_units.insert( unit );

	postMessage( UnitDiscover{ unit } );
}

void SkynetUnitTracker::onMorphRenegade( Unit unit, Player last_player, UnitType last_type )
{
	Player passed_last_player = last_player == unit->getPlayer() ? nullptr : last_player;
	UnitType passed_last_type = last_type == unit->getType() ? BWAPI::UnitTypes::None : last_type;

	if( passed_last_player )
	{
		m_player_to_units[last_player].remove( unit );
		m_player_to_units[unit->getPlayer()].insert( unit );

		m_player_to_type_to_units[last_player][last_type].remove( unit );
		m_player_to_type_to_units[unit->getPlayer()][unit->getType()].insert( unit );
	}
	else
	{
		auto & player_units = m_player_to_type_to_units[unit->getPlayer()];
		player_units[last_type].remove( unit );
		player_units[unit->getType()].insert( unit );
	}

	postMessage( UnitMorphRenegade{ unit, passed_last_player, passed_last_type } );
}

void SkynetUnitTracker::onDestroy( std::unique_ptr<SkynetUnit> & unit )
{
	unit->markDead();

	m_player_to_units[unit->getLastPlayer()].remove( unit.get() );
	m_player_to_type_to_units[unit->getLastPlayer()][unit->getLastType()].remove( unit.get() );
	m_all_units.remove( unit.get() );

	postMessage( UnitDestroy{ unit.get() } );

	m_dead_units.push_back( std::move( unit ) );
}

void SkynetUnitTracker::updateUnit( SkynetUnit * unit )
{
	Player last_player = unit->getLastPlayer();
	UnitType last_type = unit->getLastType();

	unit->update( *this );

	if( last_player != unit->getPlayer() || last_type != unit->getType() )
		onMorphRenegade( unit, last_player, last_type );
}