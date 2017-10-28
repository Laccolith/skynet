#include "SkynetBuildLocationManager.h"

#include "SkynetBuildLocation.h"
#include "BaseTracker.h"
#include "PlayerTracker.h"
#include "BaseLocation.h"
#include "Region.h"
#include "MapUtil.h"

SkynetBuildLocationManager::SkynetBuildLocationManager( Core & core )
	: BuildLocationManagerInterface( core )
{
	core.registerUpdateProcess( 3.0f, [this]() { preUpdate(); } );
}

void SkynetBuildLocationManager::preUpdate()
{
	m_calculated_positions_normal.clear();
	m_calculated_positions_power.clear();

	m_base_order_normal = getBaseTracker().getAllBases( getPlayerTracker().getLocalPlayer() );

	//TODO: Sort based on importance
}

std::pair<int, TilePosition> SkynetBuildLocationManager::choosePosition( int time, UnitType unit_type ) const
{
	std::pair<int, TilePosition> best_position( max_time, TilePositions::None );

	for( Base base : m_base_order_normal )
	{
		int ready_time = 0;
		TilePosition chosen_position = MapUtil::spiralSearch( base->getBuildPosition(), [this, &unit_type, &ready_time, base]( TilePosition tile_position ) -> bool
		{
			if( getBaseTracker().getBase( tile_position ) != base )
				return false;

			for( int x = tile_position.x; x < tile_position.x + unit_type.tileWidth(); ++x )
			{
				for( int y = tile_position.y; y < tile_position.y + unit_type.tileHeight(); ++y )
				{
					if( !BWAPI::Broodwar->isBuildable( x, y, true ) )
						return false;
				}
			}

			// TODO: Use own system
			//if( BWAPI::Broodwar->canBuildHere( tile_position, unit_type ) )
			//	return true;

			return true;
		}, std::max( BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight() ) ); // TODO: Add a bound attribute to the region and pass the max dimension here

		if( chosen_position != TilePositions::None && ready_time <= best_position.first )
		{
			best_position.first = std::max( ready_time, time );
			best_position.second = chosen_position;
		}

		if( best_position.first == time )
			return best_position;
	}

	return best_position;
}

void SkynetBuildLocationManager::reservePosition( int time, TilePosition position, UnitType unit_type )
{
	if( unit_type == UnitTypes::Protoss_Pylon )
		m_calculated_positions_power.emplace_back( CalculatedPosition{ time, position, unit_type } );
	else
		m_calculated_positions_normal.emplace_back( CalculatedPosition{ time, position, unit_type } );
}

std::unique_ptr<BuildLocation> SkynetBuildLocationManager::createBuildLocation( UnitType unit_type )
{
	return std::make_unique<SkynetBuildLocation>( *this, unit_type );
}
