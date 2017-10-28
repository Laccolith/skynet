#include "SkynetBase.h"

#include "BaseLocation.h"
#include "Unit.h"
#include "Player.h"
#include "Region.h"
#include "PlayerTracker.h"

template <typename ...ARGS>
inline void drawLabel( Position pos, int &yOffset, const ARGS &... args )
{
	BWAPI::Broodwar->drawTextMap( pos.x + 60, pos.y - yOffset, args... );
	yOffset -= 10;
}

SkynetBase::SkynetBase( CoreAccess & core_access, Position center_position, Region region, BaseLocation base_location )
	: CoreAccess( core_access )
	, m_center_position( center_position )
	, m_build_position( base_location->getBuildLocation() )
	, m_region( region )
	, m_base_location( base_location )
{
	for( auto mineral : base_location->getStaticMinerals() )
	{
		if( mineral->accessibility() != UnitAccessType::Dead )
			m_minerals.insert( mineral );
	}

	for( auto geyser : base_location->getStaticGeysers() )
	{
		if( geyser->getResources() <= 0 )
			continue;

		if( geyser->getType() == UnitTypes::Resource_Vespene_Geyser )
			m_geysers.insert( geyser );
		else
			m_refineries.insert( geyser );
	}
}

SkynetBase::SkynetBase( CoreAccess & core_access, Region region )
	: CoreAccess( core_access )
	, m_center_position( region->getCenter() )
	, m_build_position( region->getCenter() )
	, m_region( region )
{
}

bool SkynetBase::isEnemyBase() const
{
	return BWAPI::Broodwar->self()->isEnemy( m_player->getBWAPIPlayer() );
}

bool SkynetBase::isMyBase() const
{
	return m_player->isLocalPlayer();
}

bool SkynetBase::isAllyBase() const
{
	return BWAPI::Broodwar->self()->isAlly( m_player->getBWAPIPlayer() );
}

void SkynetBase::draw() const
{
	BWAPI::Broodwar->drawCircleMap( m_center_position, 78, (m_player != nullptr ? m_player->getColor() : BWAPI::Broodwar->neutral()->getColor()), false );

	int y_pos = 30;
	if( m_is_start_location )
		drawLabel( m_center_position, y_pos, "Start Location" );

	if( !m_buildings.empty() )
		drawLabel( m_center_position, y_pos, "Buildings: %i", m_buildings.size() );

	if( m_player )
		drawLabel( m_center_position, y_pos, "Player: %s", m_player->getName().c_str() );

	if( isMyBase() )
		drawLabel( m_center_position, y_pos, "Is Mine" );
	else if( isEnemyBase() )
		drawLabel( m_center_position, y_pos, "Is Enemy" );
	else if( isAllyBase() )
		drawLabel( m_center_position, y_pos, "Is Ally" );

	if( m_is_contested )
		drawLabel( m_center_position, y_pos, "Contested" );

	if( m_active_time >= 0 )
	{
		if( m_active_time > BWAPI::Broodwar->getFrameCount() )
			drawLabel( m_center_position, y_pos, "Active in %i frames", m_active_time - BWAPI::Broodwar->getFrameCount() );
		else
			drawLabel( m_center_position, y_pos, "Is Active" );
	}

	for( auto building : m_buildings )
		BWAPI::Broodwar->drawLineMap( building->getPosition(), m_center_position, building->getPlayer()->getColor() );

	for( auto mineral : m_minerals )
	{
		BWAPI::Broodwar->drawCircleMap( mineral->getPosition(), 32, BWAPI::Colors::Blue );
		BWAPI::Broodwar->drawLineMap( mineral->getPosition(), m_center_position, BWAPI::Colors::Blue );
	}

	for( auto geyser : m_geysers )
	{
		BWAPI::Broodwar->drawCircleMap( geyser->getPosition(), 32, BWAPI::Colors::Green );
		BWAPI::Broodwar->drawLineMap( geyser->getPosition(), m_center_position, BWAPI::Colors::Green );
	}

	for( auto refinery : m_refineries )
	{
		BWAPI::Broodwar->drawCircleMap( refinery->getPosition(), 32, BWAPI::Colors::Orange );
		BWAPI::Broodwar->drawLineMap( refinery->getPosition(), m_center_position, BWAPI::Colors::Green );
	}
}

void SkynetBase::update()
{
	static std::vector<int> player_building_counter;
	if( player_building_counter.empty() )
		player_building_counter.resize( BWAPI::Broodwar->getPlayers().size() );
	else
	{
		for( auto & count : player_building_counter )
			count = 0;
	}

	for( Unit building : m_buildings )
	{
		++player_building_counter[building->getPlayer()->getID()];
	}

	int invested_players = 0;
	int highest_building_count = 0;
	Player player = nullptr;

	int player_id = 0;
	for( auto count : player_building_counter )
	{
		if( count > 0 )
			++invested_players;

		if( count > highest_building_count )
		{
			highest_building_count = count;
			player = getPlayerTracker().getPlayer( player_id );
		}

		++player_id;
	}

	m_player = player;
	m_is_contested = invested_players > 1;
}

void SkynetBase::add_building( Unit building )
{
	m_buildings.insert( building );
}

void SkynetBase::remove_building( Unit building )
{
	m_buildings.remove( building );
}
