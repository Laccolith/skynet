#include "SkynetSquadManager.h"

#include "SkynetSquad.h"
#include "UnitTracker.h"
#include "PlayerTracker.h"

SkynetSquadManager::SkynetSquadManager( Core & core )
	: SquadManagerInterface( core )
	, MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>( getUnitTracker() )
{
	core.registerUpdateProcess( 8.0f, [this]() { update(); } );

	setDebugging( Debug::Default, true );

	m_squads.push_back( std::make_unique<SkynetSquad>() );
}

void SkynetSquadManager::update()
{
	// One squad for now
	for( Unit unit : m_unassigned_units )
	{
		m_squads.back()->add_unit( unit );
	}

	m_unassigned_units.clear();

	m_squads.back()->update();
}

bool isArmyUnit( UnitType unit_type )
{
	if( unit_type.isWorker() )
		return false;

	if( unit_type.isBuilding() )
		return false;

	if( unit_type == UnitTypes::Protoss_Observer )
		return false;

	return true;
}

void SkynetSquadManager::notify( const UnitDiscover & message )
{
	if( !message.unit->getPlayer()->isLocalPlayer() )
		return;

	if( !isArmyUnit( message.unit->getType() ) )
		return;

	m_unassigned_units.push_back( message.unit );
}

void SkynetSquadManager::notify( const UnitMorphRenegade & message )
{
	if( !message.isRenegade() )
		return;

	if( message.last_player->isLocalPlayer() )
	{
		if( message.isMorph() ? !isArmyUnit( message.last_type ) : !isArmyUnit( message.unit->getType() ) )
			return;

		auto it = m_squad_assignments.find( message.unit );
		if( it != m_squad_assignments.end() )
		{
			it->second->remove_unit( message.unit );
			m_squad_assignments.erase( it );
		}
	}
	else if( message.unit->getPlayer()->isLocalPlayer() )
	{
		if( !isArmyUnit( message.unit->getType() ) )
			return;

		m_unassigned_units.push_back( message.unit );
	}
}

void SkynetSquadManager::notify( const UnitDestroy & message )
{
	if( !message.unit->getPlayer()->isLocalPlayer() )
		return;

	if( !isArmyUnit( message.unit->getType() ) )
		return;
	
	auto it = m_squad_assignments.find( message.unit );
	if( it != m_squad_assignments.end() )
	{
		it->second->remove_unit( message.unit );
		m_squad_assignments.erase( it );
	}
}
