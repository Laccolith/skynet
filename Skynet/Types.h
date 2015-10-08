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

using BWAPI::Player;

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

namespace CoordinateType = BWAPI::CoordinateType;

using BWAPI::Order;
namespace Orders = BWAPI::Orders;
using BWAPI::UnitCommand;

using BWAPI::WALKPOSITION_SCALE;
using BWAPI::TILEPOSITION_SCALE;