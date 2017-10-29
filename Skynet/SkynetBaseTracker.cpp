#include "SkynetBaseTracker.h"

#include "BaseLocation.h"
#include "Unit.h"
#include "Region.h"
#include "Player.h"

SkynetBaseTracker::SkynetBaseTracker( Core & core )
	: BaseTrackerInterface( core )
	, MessageListener<TerrainAnalysed>( getTerrainAnalyser() )
	, MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>( getUnitTracker() )
{
	core.registerUpdateProcess( 1.5f, [this]() { update(); } );

	m_player_bases.resize( BWAPI::Broodwar->getPlayers().size() );
}

const std::vector<Base> &SkynetBaseTracker::getAllBases( Player player ) const
{
	return m_player_bases[player->getID()];
}

void SkynetBaseTracker::update()
{
	for( auto & bases : m_player_bases )
		bases.clear();

	for( auto& base : m_base_storage )
	{
		base->update();

		if( base->getPlayer() )
			m_player_bases[base->getPlayer()->getID()].push_back( base.get() );
	}

	if( isDebugging( Debug::Default ) )
	{
		for( auto base : m_bases )
			base->draw();
	}
}

void SkynetBaseTracker::notify( const TerrainAnalysed & message )
{
	m_bases.clear();
	m_base_storage.clear();

	for( auto bases : m_player_bases )
		bases.clear();

	std::map<Region, std::vector<SkynetBase*>> region_to_base;

	for( auto base_location : getTerrainAnalyser().getBaseLocations() )
	{
		m_base_storage.emplace_back( std::make_unique<SkynetBase>( *this, base_location->getCenterPosition(), base_location->getRegion(), base_location ) );
		SkynetBase *current_base = m_base_storage.back().get();
		m_bases.push_back( current_base );

		region_to_base[base_location->getRegion()].push_back( current_base );
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

			std::vector<SkynetBase*> & region_bases = region_to_base[region];
			if( region_bases.size() > 1 ) // Multiple possible bases, use closest
			{
				SkynetBase* base_to_use = nullptr;
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
				m_base_storage.emplace_back( std::make_unique<SkynetBase>( *this, region ) );
				SkynetBase *new_base = m_base_storage.back().get();
				m_bases.push_back( new_base );

				region_bases.push_back( new_base );
				m_tile_to_base[pos] = new_base;
			}
			else // Only one base, easy
				m_tile_to_base[pos] = region_bases.back();
		}
	}

	for( TilePosition start_location : BWAPI::Broodwar->getStartLocations() )
		m_tile_to_base[start_location]->mark_as_start_location();

	for( Unit unit : getUnitTracker().getAllUnits() )
		notify( UnitDiscover{ unit } );

	postMessage( BasesRecreated{} );
}

void SkynetBaseTracker::notify( const UnitDiscover & message )
{
	if( message.unit->getType().isBuilding() && !message.unit->getPlayer()->isNeutral() && !m_bases.empty() )
	{
		m_tile_to_base[message.unit->getTilePosition()]->add_building( message.unit );
	}
}

void SkynetBaseTracker::notify( const UnitMorphRenegade & message )
{
	if( message.unit->getType().isBuilding() && !message.unit->getPlayer()->isNeutral() && !m_bases.empty() )
	{
		m_tile_to_base[message.unit->getTilePosition()]->add_building( message.unit );
	}
}

void SkynetBaseTracker::notify( const UnitDestroy & message )
{
	if( message.unit->getType().isBuilding() && !message.unit->getPlayer()->isNeutral() && !m_bases.empty() )
	{
		m_tile_to_base[message.unit->getTilePosition()]->remove_building( message.unit );
	}
}