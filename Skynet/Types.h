#pragma once

#include <BWAPI.h>

using BWAPI::TilePosition;
namespace TilePositions = BWAPI::TilePositions;
using BWAPI::WalkPosition;
namespace WalkPositions = BWAPI::WalkPositions;
using BWAPI::Position;
namespace Positions = BWAPI::Positions;

typedef BWAPI::Point<float, BWAPI::TILEPOSITION_SCALE> TilePositionFloat;
typedef BWAPI::Point<float, BWAPI::WALKPOSITION_SCALE> WalkPositionFloat;
typedef BWAPI::Point<float, BWAPI::POSITION_SCALE> PositionFloat;

using BWAPI::Race;
namespace Races = BWAPI::Races;
using BWAPI::UnitType;
namespace UnitTypes = BWAPI::UnitTypes;
using BWAPI::UpgradeType;
namespace UpgradeTypes = BWAPI::UpgradeTypes;
using BWAPI::TechType;
namespace TechTypes = BWAPI::TechTypes;
using BWAPI::WeaponType;
namespace WeaponTypes = BWAPI::WeaponTypes;
using BWAPI::DamageType;
namespace DamageTypes = BWAPI::DamageTypes;
using BWAPI::UnitSizeType;
namespace UnitSizeTypes = BWAPI::UnitSizeTypes;
using BWAPI::UnitCommandType;
namespace UnitCommandTypes = BWAPI::UnitCommandTypes;
using BWAPI::Color;
namespace Colors = BWAPI::Colors;
using BWAPI::Event;
namespace Events = BWAPI::EventType;

namespace CoordinateType = BWAPI::CoordinateType;

using BWAPI::Order;
namespace Orders = BWAPI::Orders;
using BWAPI::UnitCommand;

using BWAPI::TILEPOSITION_SCALE;
using BWAPI::WALKPOSITION_SCALE;
using BWAPI::POSITION_SCALE;

class PlayerInterface;
typedef PlayerInterface *Player;

class UnitInterface;
typedef UnitInterface *Unit;

class RegionInterface;
typedef RegionInterface *Region;

class ChokepointInterface;
typedef ChokepointInterface *Chokepoint;

class BaseLocationInterface;
typedef BaseLocationInterface *BaseLocation;

class BaseInterface;
typedef BaseInterface *Base;

const int max_time = std::numeric_limits<int>::max();
const int max_distance = std::numeric_limits<int>::max();

template <int Scale>
inline float dotProduct( BWAPI::Point<float, Scale> v1, BWAPI::Point<float, Scale> v2 )
{
	return v1.x * v2.x + v1.y * v2.y;
}

template <int Scale>
inline float normalise( BWAPI::Point<float, Scale> & v )
{
	float length = sqrt( v.x * v.x + v.y * v.y );

	if( length > 0 )
	{
		v.x /= length;
		v.y /= length;
	}

	return length;
}

template <typename T, int Scale>
bool operator>=( BWAPI::Point<T, Scale> lhs, BWAPI::Point<T, Scale> rhs )
{
	return std::tie( lhs.x, lhs.y ) >= std::tie( rhs.x, rhs.y );
}