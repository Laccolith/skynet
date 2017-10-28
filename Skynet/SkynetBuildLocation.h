#pragma once

#include "BuildLocation.h"

class SkynetBuildLocationManager;
class SkynetBuildLocation : public BuildLocation
{
public:
	SkynetBuildLocation( SkynetBuildLocationManager & manager, UnitType unit_type );

	int calculatePosition( int earliest_time ) override;
	void reservePosition( int time ) override;

	Position getPosition() const override;
	TilePosition getTilePosition() const override;

private:
	SkynetBuildLocationManager & m_manager;

	UnitType m_unit_type;

	TilePosition m_chosen_position = TilePositions::None;
};
