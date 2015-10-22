#include "SkynetUnitTracker.h"

#include "Skynet.h"

SkynetUnitTracker::SkynetUnitTracker( Access & access )
	: UnitTrackerInterface( access )
{
	m_player_to_type_to_units.resize( BWAPI::Broodwar->getPlayers().size() );
	m_player_to_units.resize( BWAPI::Broodwar->getPlayers().size() );

	getSkynet().registerUpdateProcess( 0.0f, [this]() { update(); } );
}

Unit SkynetUnitTracker::getUnit( BWAPI::Unit unit ) const
{
	return m_bwapi_units[unit->getID()].get();
}

UnitGroup SkynetUnitTracker::getUnitGroup( const BWAPI::Unitset & units ) const
{
	UnitGroup return_units;

	for( BWAPI::Unit unit : units )
	{
		if( m_bwapi_units[unit->getID()] )
			return_units.insert( m_bwapi_units[unit->getID()].get() );
	}

	return return_units;
}

const UnitGroup & SkynetUnitTracker::getAllUnits( UnitType type, Player player ) const
{
	return m_player_to_type_to_units[player->getID()][type];
}

const UnitGroup & SkynetUnitTracker::getAllUnits( Player player ) const
{
	return m_player_to_units[player->getID()];
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
	for( auto & dead_unit : m_dead_units )
		m_free_ids.push_back( dead_unit->getID() );
	m_dead_units.clear();

	for( const Event &event : BWAPI::Broodwar->getEvents() )
	{
		if( event.getType() == Events::UnitDestroy )
			onUnitDestroy( event.getUnit() );
	}

	for( const Event &event : BWAPI::Broodwar->getEvents() )
	{
		if( event.getType() == Events::UnitDiscover )
			onUnitDiscover( event.getUnit() );
	}

	for( auto & unit : m_bwapi_units )
	{
		if( unit )
			updateUnit( unit.get() );
	}

	if( isDebugging( Debug::Default ) )
	{
		for( auto unit : m_all_units )
			unit->drawUnitPosition();
	}
}

void SkynetUnitTracker::onUnitDiscover( BWAPI::Unit unit )
{
	if( unit->getID() >= (int)m_bwapi_units.size() )
		m_bwapi_units.resize( unit->getID() + 1 );

	auto & new_unit = m_bwapi_units[unit->getID()];

	if( new_unit )
		return;

	// TODO: Prediction
	
	if( !m_free_ids.empty() )
	{
		new_unit = std::make_unique<SkynetUnit>( unit, m_free_ids.back(), *this );
		m_free_ids.pop_back();
	}
	else
		new_unit = std::make_unique<SkynetUnit>( unit, ++m_current_id_counter, *this );

	onDiscover( new_unit.get() );
}

void SkynetUnitTracker::onUnitDestroy( BWAPI::Unit unit )
{
	onDestroy( m_bwapi_units[unit->getID()] );
}

void SkynetUnitTracker::onDiscover( Unit unit )
{
	m_player_to_type_to_units[unit->getPlayer()->getID()][unit->getType()].insert( unit );
	m_player_to_units[unit->getPlayer()->getID()].insert( unit );
	m_all_units.insert( unit );

	postMessage( UnitDiscover{ unit } );
}

void SkynetUnitTracker::onMorphRenegade( Unit unit, Player last_player, UnitType last_type )
{
	Player passed_last_player = last_player == unit->getPlayer() ? nullptr : last_player;
	UnitType passed_last_type = last_type == unit->getType() ? BWAPI::UnitTypes::None : last_type;

	if( passed_last_player )
	{
		m_player_to_units[last_player->getID()].remove( unit );
		m_player_to_units[unit->getPlayer()->getID()].insert( unit );

		m_player_to_type_to_units[last_player->getID()][last_type].remove( unit );
		m_player_to_type_to_units[unit->getPlayer()->getID()][unit->getType()].insert( unit );
	}
	else
	{
		auto & player_units = m_player_to_type_to_units[unit->getPlayer()->getID()];
		player_units[last_type].remove( unit );
		player_units[unit->getType()].insert( unit );
	}

	postMessage( UnitMorphRenegade{ unit, passed_last_player, passed_last_type } );
}

void SkynetUnitTracker::onDestroy( std::unique_ptr<SkynetUnit> & unit )
{
	unit->markDead();

	m_player_to_units[unit->getLastPlayer()->getID()].remove( unit.get() );
	m_player_to_type_to_units[unit->getLastPlayer()->getID()][unit->getLastType()].remove( unit.get() );
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