#include "SkynetBuildLocationManager.h"

#include "SkynetBuildLocation.h"
#include "BaseTracker.h"
#include "PlayerTracker.h"
#include "BaseLocation.h"
#include "Region.h"
#include "Unit.h"
#include "UnitTracker.h"
#include "MapUtil.h"

SkynetBuildLocationManager::SkynetBuildLocationManager( Core & core )
	: BuildLocationManagerInterface( core )
	, MessageListener<BasesRecreated>( getBaseTracker() )
	, MessageListener<UnitDestroy>( getUnitTracker() )
{
	core.registerUpdateProcess( 3.0f, [this]() { preUpdate(); } );
	core.registerUpdateProcess( 5.0f, [this]() { postUpdate(); } );

	m_build_power_times.resize( BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight() );
	m_tile_info.resize( BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight() );
}

void SkynetBuildLocationManager::notify( const BasesRecreated & message )
{
	m_base_to_build_positions.clear();
	m_tile_info.fill();

	for( Base base : getBaseTracker().getAllBases() )
	{
		auto & build_positions = m_base_to_build_positions[base];

		MapUtil::spiralSearch( base->getBuildPosition(), [this, base, &build_positions]( TilePosition tile_position ) -> bool
		{
			if( !BWAPI::Broodwar->isBuildable( tile_position ) )
				return false;

			if( getBaseTracker().getBase( tile_position ) != base )
				return false;

			build_positions.push_back( tile_position );
			return false;
		} );

		if( base->getLocation() )
		{
			UnitType depot_type = getPlayerTracker().getLocalPlayer()->getRace().getResourceDepot();
			TilePosition tile_position = base->getLocation()->getBuildLocation();
			Position center_position = base->getLocation()->getCenterPosition();

			for( int x = tile_position.x; x < tile_position.x + depot_type.tileWidth(); ++x )
				for( int y = tile_position.y; y < tile_position.y + depot_type.tileHeight(); ++y )
					m_tile_info[TilePosition( x, y )].is_permanently_reserved = true;

			auto reserve_resouce_area = [this, base, center_position]( const UnitGroup & resources )
			{
				for( Unit resource : resources )
				{
					int distance_between = (resource->getPosition().getApproxDistance( center_position ) / 32) + 1;

					for( int x = resource->getTilePosition().x - distance_between; x < resource->getTilePosition().x + distance_between + 4; ++x )
					{
						for( int y = resource->getTilePosition().y - distance_between; y < resource->getTilePosition().y + distance_between + 3; ++y )
						{
							if( x < 0 || y < 0 || x >= BWAPI::Broodwar->mapWidth() || y >= BWAPI::Broodwar->mapWidth() )
								continue;

							if( m_tile_info[TilePosition( x, y )].is_permanently_reserved )
								continue;

							int distance_to_resource = Position( x * 32 + 16, y * 32 + 16 ).getApproxDistance( resource->getPosition() ) / 32;
							int distance_to_base = Position( x * 32 + 16, y * 32 + 16 ).getApproxDistance( center_position ) / 32;

							if( distance_to_resource + distance_to_base < distance_between )
							{
								++m_tile_info[TilePosition( x, y )].m_resouce_reserved;

								if( resource->getType().isMineralField() )
									m_resources[resource].push_back( TilePosition( x, y ) );
							}
						}
					}
				}
			};

			reserve_resouce_area( base->getMinerals() );
			reserve_resouce_area( base->getGeysers() );
		}
	}
}

void SkynetBuildLocationManager::notify( const UnitDestroy & message )
{
	if( !message.unit->getType().isMineralField() )
		return;

	for( TilePosition resource_position : m_resources[message.unit] )
		--m_tile_info[resource_position].m_resouce_reserved;

	m_resources.erase( message.unit );
}

void SkynetBuildLocationManager::preUpdate()
{
	m_build_power_times.fill();
	m_earliest_power_time = max_time;

	for( auto & tile_info : m_tile_info )
	{
		tile_info.is_build_planned = false;
	}

	for( Unit pylon : getUnitTracker().getAllUnits( UnitTypes::Protoss_Pylon, getPlayerTracker().getLocalPlayer() ) )
	{
		addToPowerTime( pylon->getTilePosition(), pylon->getTimeTillCompleted() );
	}

	for( auto & reserved_position : m_reserved_positions )
	{
		if( reserved_position.unit_type == UnitTypes::Protoss_Pylon )
			addToPowerTime( reserved_position.position, UnitTypes::Protoss_Pylon.buildTime() );

		for( int x = reserved_position.position.x; x < reserved_position.position.x + reserved_position.unit_type.tileWidth(); ++x )
		{
			for( int y = reserved_position.position.y; y < reserved_position.position.y + reserved_position.unit_type.tileHeight(); ++y )
			{
				m_tile_info[TilePosition( x, y )].is_build_planned = true;
			}
		}
	}

	m_base_order_normal = getBaseTracker().getAllBases( getPlayerTracker().getLocalPlayer() );

	//TODO: Sort based on importance
}

void SkynetBuildLocationManager::postUpdate()
{
	for( auto build_location : m_build_locations )
	{
		build_location->drawInfo();
	}
}

std::pair<int, TilePosition> SkynetBuildLocationManager::choosePosition( int time, UnitType unit_type ) const
{
	std::pair<int, TilePosition> best_position( max_time, TilePositions::None );

	if( unit_type.isRefinery() )
	{
		for( Base base : m_base_order_normal )
		{
			for( Unit geyser : base->getGeysers() )
			{
				if( geyser->getType() != UnitTypes::Resource_Vespene_Geyser )
					continue;

				if( m_tile_info[geyser->getTilePosition()].is_build_planned )
					continue;

				return std::make_pair( time, geyser->getTilePosition() );
			}
		}

		return best_position;
	}

	if( unit_type.requiresPsi() )
	{
		if( m_earliest_power_time == max_time )
			return best_position;

		time = std::max( m_earliest_power_time, time );
	}

	for( Base base : m_base_order_normal )
	{
		auto build_positions_it = m_base_to_build_positions.find( base );
		if( build_positions_it == m_base_to_build_positions.end() )
			continue;

		for( TilePosition tile_position : build_positions_it->second )
		{
			bool is_buildable = true;
			for( int x = tile_position.x; x < tile_position.x + unit_type.tileWidth() && is_buildable; ++x )
			{
				for( int y = tile_position.y; y < tile_position.y + unit_type.tileHeight() && is_buildable; ++y )
				{
					if( !BWAPI::Broodwar->isBuildable( x, y, true ) )
						is_buildable = false;

					auto tile_info = m_tile_info[TilePosition( x, y )];

					if( tile_info.is_build_planned || tile_info.is_permanently_reserved || tile_info.m_resouce_reserved > 0 )
						is_buildable = false;
				}
			}

			if( !is_buildable )
				continue;

			int ready_time = 0;

			if( unit_type.requiresPsi() )
			{
				if( unit_type.tileHeight() == 2 && unit_type.tileWidth() == 2 )
				{
					int power_available_time = m_build_power_times[tile_position].small_building_time;
					if( power_available_time > ready_time )
						ready_time = power_available_time;
				}
				else if( unit_type.tileHeight() == 2 && unit_type.tileWidth() == 3 )
				{
					int power_available_time = m_build_power_times[tile_position].medium_building_time;
					if( power_available_time > ready_time )
						ready_time = power_available_time;
				}
				else if( unit_type.tileHeight() == 3 && unit_type.tileWidth() == 4 )
				{
					int power_available_time = m_build_power_times[tile_position].large_building_time;
					if( power_available_time > ready_time )
						ready_time = power_available_time;
				}
			}

			int earliest_time = std::max( ready_time, time );
			if( earliest_time < best_position.first )
			{
				best_position.first = earliest_time;
				best_position.second = tile_position;

				if( best_position.first <= time )
				{
					return best_position;
				}
			}
		}
	}

	return best_position;
}

void SkynetBuildLocationManager::reservePosition( int time, TilePosition position, UnitType unit_type )
{
	if( unit_type == UnitTypes::Protoss_Pylon )
	{
		addToPowerTime( position, time );
	}

	for( int x = position.x; x < position.x + unit_type.tileWidth(); ++x )
	{
		for( int y = position.y; y < position.y + unit_type.tileHeight(); ++y )
		{
			m_tile_info[TilePosition(x, y)].is_build_planned = true;
		}
	}

	if( time <= 0 )
		m_reserved_positions.emplace_back( ReservedPosition{ position, unit_type } );
}

void SkynetBuildLocationManager::freeReservation( TilePosition position, UnitType unit_type )
{
	for( auto & reserved_position : m_reserved_positions )
	{
		if( reserved_position.position == position && reserved_position.unit_type == unit_type )
		{
			reserved_position = m_reserved_positions.back();
			break;
		}
	}

	m_reserved_positions.pop_back();
}

std::unique_ptr<BuildLocation> SkynetBuildLocationManager::createBuildLocation( UnitType unit_type )
{
	auto build_location = std::make_unique<SkynetBuildLocation>( *this, unit_type );
	m_build_locations.insert( build_location.get(), true );
	return build_location;
}

void SkynetBuildLocationManager::onBuildLocationDestroyed( SkynetBuildLocation * build_location )
{
	m_build_locations.remove( build_location );
}

void SkynetBuildLocationManager::addToPowerTime( TilePosition tile_position, int time )
{
	if( time < m_earliest_power_time )
		m_earliest_power_time = time;

	for( int x = 0; x <= 15; ++x )
	{
		for( int y = 0; y <= 9; ++y )
		{
			bool in_range_small = false;
			bool in_range_medium = false;
			bool in_range_large = false;

			switch( y )
			{
			case 0:
				if( x >= 4 && x <= 9 )
					in_range_large = true;
				break;
			case 1:
			case 8:
				if( x >= 2 && x <= 13 )
				{
					in_range_small = true;
					in_range_medium = true;
				}
				if( x >= 1 && x <= 12 )
					in_range_large = true;
				break;
			case 2:
			case 7:
				if( x >= 1 && x <= 14 )
				{
					in_range_small = true;
					in_range_medium = true;
				}
				if( x <= 13 )
					in_range_large = true;
				break;
			case 3:
			case 4:
			case 5:
			case 6:
				if( x >= 1 )
					in_range_small = true;
				in_range_medium = true;
				if( x <= 14 )
					in_range_large = true;
				break;
			case 9:
				if( x >= 5 && x <= 10 )
				{
					in_range_small = true;
					in_range_medium = true;
				}
				if( x >= 4 && x <= 9 )
					in_range_large = true;
				break;
			}

			const TilePosition tile( tile_position.x + x - 8, tile_position.y + y - 5 );
			if( tile.x < 0 || tile.y < 0 || tile.x >= BWAPI::Broodwar->mapWidth() || tile.y >= BWAPI::Broodwar->mapWidth() )
				continue;

			PowerTimes & power_time = m_build_power_times[tile];

			if( in_range_small )
			{
				if( time < power_time.small_building_time )
					power_time.small_building_time = time;
			}

			if( in_range_medium )
			{
				if( time < power_time.medium_building_time )
					power_time.medium_building_time = time;
			}

			if( in_range_large )
			{
				if( time < power_time.large_building_time )
					power_time.large_building_time = time;
			}
		}
	}
}
