#pragma once

#include "BuildLocationManager.h"

class SkynetBuildLocationManager : public BuildLocationManagerInterface
{
public:
	SkynetBuildLocationManager( Core & core );

	void preUpdate();

	std::pair<int, TilePosition> choosePosition( int time, UnitType unit_type ) const;
	void reservePosition( int time, TilePosition position , UnitType unit_type );

	std::unique_ptr<BuildLocation> createBuildLocation( UnitType unit_type ) override;

private:
	struct CalculatedPosition
	{
		int time;
		TilePosition position;
		UnitType unit_type;
	};
	std::vector<CalculatedPosition> m_calculated_positions_normal;
	std::vector<CalculatedPosition> m_calculated_positions_power;

	std::vector<Base> m_base_order_normal;
};
