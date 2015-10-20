#include "SkynetBaseTracker.h"

#include "Skynet.h"
#include "BaseLocation.h"
#include "Unit.h"
#include "Region.h"

SkynetBaseTracker::SkynetBaseTracker( Access & access )
	: BaseTrackerInterface( access )
	, MessageListener<TerrainAnalysed>( getTerrainAnalyser() )
{
	getSkynet().registerUpdateProcess( 1.0f, [this]() { update(); } );
	setDebugging( Debug::Default, true );
}

void SkynetBaseTracker::update()
{
	if( isDebugging( Debug::Default ) )
	{
		for( auto base : m_bases )
			base->draw();
	}
}

void SkynetBaseTracker::notify( const TerrainAnalysed & message )
{
	std::map<Region, std::vector<Base>> region_to_base;

	for( auto base_location : getTerrainAnalyser().getBaseLocations() )
	{
		m_base_storage.emplace_back( std::make_unique<SkynetBase>() );
		SkynetBase *current_base = m_base_storage.back().get();
		m_bases.push_back( current_base );

		current_base->m_center_position = base_location->getCenterPosition();
		current_base->m_base_location = base_location;
		current_base->m_region = base_location->getRegion();

		region_to_base[base_location->getRegion()].push_back( current_base );

		for( auto mineral : base_location->getStaticMinerals() )
		{
			if( mineral->accessibility() != UnitAccessType::Dead )
				current_base->m_minerals.insert( mineral );
		}

		for( auto geyser : base_location->getStaticGeysers() )
		{
			if( geyser->getResources() <= 0 )
				continue;

			if( geyser->getType() == UnitTypes::Resource_Vespene_Geyser )
				current_base->m_geysers.insert( geyser );
			else
				current_base->m_refineries.insert( geyser );
		}
	}

	WalkPosition map_size( BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight() );
	m_tile_to_base.resize( map_size.x, map_size.y, nullptr );

	for( int x = 0; x < map_size.x; ++x )
	{
		for( int y = 0; y < map_size.y; ++y )
		{
			TilePosition pos( x, y );

			if( !BWAPI::Broodwar->isBuildable( pos ) )
				continue;

			Region region = getTerrainAnalyser().getRegion( WalkPosition( pos ) );
			if( !region )
				continue;

			std::vector<Base> & region_bases = region_to_base[region];
			if( region_bases.size() > 1 ) // Multiple possible bases, use closest
			{
				Base base_to_use = nullptr;
				int distance = std::numeric_limits<int>::max();

				for( auto base : region_bases )
				{
					int this_distance = base->getCenterPosition().getApproxDistance( Position( pos ) );
					if( this_distance < distance )
					{
						distance = this_distance;
						base_to_use = base;
					}
				}

				if( base_to_use )
					m_tile_to_base[pos] = base_to_use;
			}
			else if( region_bases.empty() ) // No bases, create new one for region
			{
				m_base_storage.emplace_back( std::make_unique<SkynetBase>() );
				SkynetBase *new_base = m_base_storage.back().get();
				m_bases.push_back( new_base );

				new_base->m_center_position = Position( region->getCenter() );
				new_base->m_region = region;

				region_bases.push_back( new_base );
				m_tile_to_base[pos] = new_base;
			}
			else // Only one base, easy
				m_tile_to_base[pos] = region_bases.back();
		}
	}

	for( TilePosition start_location : BWAPI::Broodwar->getStartLocations() )
		static_cast<SkynetBase*>( m_tile_to_base[start_location] )->m_is_start_location = true;
}