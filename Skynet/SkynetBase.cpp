#include "SkynetBase.h"

#include "BaseLocation.h"
#include "Unit.h"

template <typename ...ARGS>
inline void drawLabel( Position pos, int &yOffset, const ARGS &... args )
{
	BWAPI::Broodwar->drawTextMap( pos.x + 60, pos.y - yOffset, args... );
	yOffset -= 10;
}

void SkynetBase::draw() const
{
	BWAPI::Broodwar->drawCircleMap( m_center_position, 78, (m_player != nullptr ? m_player : BWAPI::Broodwar->neutral())->getColor(), false );

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