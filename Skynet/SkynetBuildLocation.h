#pragma once

#include "BuildLocation.h"

class SkynetBuildLocationManager;
class SkynetBuildLocation : public BuildLocation
{
public:
	SkynetBuildLocation( SkynetBuildLocationManager & manager, UnitType unit_type );
	~SkynetBuildLocation();

	int calculatePosition( int earliest_time ) override;
	void reservePosition( int time ) override;
	void freeReservation() override;

	TilePosition getTilePosition() const override;
	UnitType getUnitType() const override;

	void drawInfo();

private:
	SkynetBuildLocationManager & m_manager;

	UnitType m_unit_type;

	TilePosition m_chosen_position = TilePositions::None;
};
