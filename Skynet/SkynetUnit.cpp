#include "SkynetUnit.h"

#include "SkynetUnitTracker.h"
#include "PlayerTracker.h"

#include "MapUtil.h"
#include "DrawingUtil.h"

SkynetUnit::SkynetUnit( BWAPI::Unit unit, int id, SkynetUnitTracker & unit_tracker, PlayerTrackerInterface & player_tracker )
	: m_unit( unit )
	, m_id( id )
{
	bool useable = exists();
	if( useable )
	{
		m_completed = unit->isCompleted();
		m_morphing = unit->isMorphing();
	}

	update( unit_tracker, player_tracker );

	if( useable )
	{
		if( !isCompleted() )
			m_time_till_completed = unit->getType().buildTime();
		else if( isMorphing() )
			m_time_till_completed = unit->getBuildType().buildTime();
	}
}

SkynetUnit::SkynetUnit( int id, Player player, Position pos, UnitType type, int time_till_start )
	: m_id( id )
	, m_position( pos )
	, m_target_position( pos )
	, m_type( type )
	, m_player( player )
	, m_last_player( player )
	, m_updated_time( BWAPI::Broodwar->getFrameCount() )
	, m_time_till_completed( time_till_start + type.buildTime() )
	, m_exists_time( time_till_start )
{
}

TilePosition SkynetUnit::getTilePosition() const
{
	if( exists() )
	{
		if( m_unit->isLoaded() )
			return m_unit->getTransport()->getTilePosition();
		else
			return m_unit->getTilePosition();
	}

	const Position &pos = getPosition();
	UnitType type = getType();

	return TilePosition( (pos.x - (type.tileWidth() * 16)) / 32, (pos.y - (type.tileHeight() * 16)) / 32 );
}

TilePosition SkynetUnit::getTilePosition( int in_frames_time ) const
{
	const Position &pos = getPosition( in_frames_time );
	UnitType type = getType();

	return TilePosition( (pos.x - (type.tileWidth() * 16)) / 32, (pos.y - (type.tileHeight() * 16)) / 32 );
}

WalkPosition SkynetUnit::getWalkPosition() const
{
	const Position &pos = getPosition();
	UnitType type = getType();

	return WalkPosition( (pos.x - type.dimensionLeft()) / 8, (pos.y - type.dimensionUp()) / 8 );
}

WalkPosition SkynetUnit::getWalkPosition( int in_frames_time ) const
{
	const Position &pos = getPosition( in_frames_time );
	UnitType type = getType();

	return WalkPosition( (pos.x - type.dimensionLeft()) / 8, (pos.y - type.dimensionUp()) / 8 );
}

Position SkynetUnit::getPosition() const
{
	if( exists() )
	{
		if( m_unit->isLoaded() )
			return m_unit->getTransport()->getPosition();
		else
			return m_unit->getPosition();
	}

	const BWAPI::UnitType &type = getType();

	if( type.topSpeed() != 0.0 )
	{
		int travelTime = int( m_position.getApproxDistance( m_target_position ) / type.topSpeed() );
		int timeDifference = BWAPI::Broodwar->getFrameCount() - m_updated_time;

		if( timeDifference > travelTime )
			return m_target_position;

		if( travelTime != 0 )
		{
			float traveled = float( timeDifference ) / float( travelTime );

			PositionFloat currentPosition = m_target_position - m_position;

			currentPosition *= traveled;
			currentPosition += m_position;

			return currentPosition;
		}
	}

	return m_position;
}

Position SkynetUnit::getPosition( int in_frames_time ) const
{
	const Position &currentPosition = getPosition();
	const UnitType &type = getType();

	if( type.topSpeed() != 0.0 )
	{
		const Position &targetPosition = getTargetPosition();

		int travelTime = int( currentPosition.getApproxDistance( targetPosition ) / type.topSpeed() );

		if( in_frames_time > travelTime )
			return targetPosition;

		if( travelTime != 0 )
		{
			float traveled = float( in_frames_time ) / float( travelTime );

			PositionFloat direction = targetPosition - currentPosition;

			direction *= traveled;
			direction += m_position;

			return direction;
		}
	}

	return currentPosition;
}

Position SkynetUnit::getTargetPosition() const
{
	if( exists() )
	{
		const UnitType &type = getType();
		if( type.canMove() || type.isFlyingBuilding() )
		{
			if( m_unit->isLoaded() )
				return m_unit->getTransport()->getTargetPosition();
			else
				return m_unit->getTargetPosition();
		}
		else
			return m_unit->getPosition();
	}

	return m_target_position;
}

UnitType SkynetUnit::getType() const
{
	if( exists() )
		return m_unit->getType();

	return m_type;
}

UnitType SkynetUnit::getLastType() const
{
	return m_type;
}

Player SkynetUnit::getPlayer() const
{
	if( exists() )
		return m_player;

	return m_last_player;
}

Player SkynetUnit::getLastPlayer() const
{
	return m_last_player;
}

Unit SkynetUnit::getTarget() const
{
	if( exists() )
	{
		if( m_unit->getOrderTarget() )
			return m_order_target;
		else
			return m_target;
	}

	return nullptr;
}

BWAPI::Unit SkynetUnit::getBWAPIUnit() const
{
	return m_unit;
}

bool SkynetUnit::exists() const
{
	return m_unit && m_unit->exists();
}

int SkynetUnit::getTimeTillExists() const
{
	if( m_unit && m_unit->exists() )
		return 0;

	return m_exists_time;
}

bool SkynetUnit::isMorphing() const
{
	if( exists() )
	{
		if( !m_unit->isMorphing() )
			return false;

		if( m_unit->getPlayer() == BWAPI::Broodwar->self() )
			return m_unit->getRemainingBuildTime() > 0;

		return true;
	}

	if( !m_morphing )
		return false;

	return m_time_till_completed > 0;
}

bool SkynetUnit::isCompleted() const
{
	if( exists() )
	{
		if( m_unit->isCompleted() )
			return true;

		if( m_unit->getPlayer() == BWAPI::Broodwar->self() )
			return m_unit->getRemainingBuildTime() == 0;

		return false;
	}

	if( accessibility() == UnitAccessType::Prediction )
		return false;

	if( m_completed )
		return true;

	return m_time_till_completed <= 0;
}

int SkynetUnit::getTimeTillCompleted() const
{
	if( isCompleted() )
		return 0;

	if( exists() )
	{
		if( m_unit->getPlayer() == BWAPI::Broodwar->self() )
			return m_unit->getRemainingBuildTime();
	}
	else
	{
		const int existsTime = getTimeTillExists();
		const int completeTime = existsTime + getType().buildTime();

		if( completeTime < existsTime )
			return max_time;
		else
			return completeTime;
	}

	return std::max( m_time_till_completed, 1 );
}

bool SkynetUnit::isConstructing() const
{
	if( exists() )
		return m_unit->isConstructing();

	return false;
}

bool SkynetUnit::isBeingConstructed() const
{
	if( exists() )
		return m_unit->isBeingConstructed();

	return false;
}

bool SkynetUnit::isTraining() const
{
	if( exists() )
		return m_unit->isTraining();

	return false;
}

bool SkynetUnit::isResearching() const
{
	if( exists() )
		return m_unit->isResearching();

	return false;
}

bool SkynetUnit::isUpgrading() const
{
	if( exists() )
		return m_unit->isUpgrading();

	return false;
}

int SkynetUnit::getRemainingTrainTime() const
{
	if( exists() && m_unit->getPlayer() == BWAPI::Broodwar->self() )
		return m_unit->getRemainingTrainTime();

	return 0;
}

int SkynetUnit::getRemainingUpgradeTime() const
{
	if( exists() && m_unit->getPlayer() == BWAPI::Broodwar->self() )
		return m_unit->getRemainingUpgradeTime();

	return 0;
}

int SkynetUnit::getRemainingResearchTime() const
{
	if( exists() && m_unit->getPlayer() == BWAPI::Broodwar->self() )
		return m_unit->getRemainingResearchTime();

	return 0;
}

int SkynetUnit::getResources() const
{
	if( exists() )
		return m_unit->getResources();

	return m_resources;
}

UpgradeType SkynetUnit::getUpgrade() const
{
	if( exists() )
		return m_unit->getUpgrade();

	return UpgradeTypes::None;
}

TechType SkynetUnit::getTech() const
{
	if( exists() )
		return m_unit->getTech();

	return TechTypes::None;
}

UnitType SkynetUnit::getBuildType() const
{
	if( m_unit )
	{
		if( m_unit->exists() )
			return m_unit->getBuildType();
		else
			return UnitTypes::Unknown;
	}

	return UnitTypes::None;
}

int SkynetUnit::getLastSeenTime() const
{
	if( exists() )
		return BWAPI::Broodwar->getFrameCount();

	return m_updated_time;
}

UnitAccessType SkynetUnit::accessibility() const
{
	if( m_unit )
	{
		if( m_unit->exists() )
		{
			if( m_unit->getPlayer() == BWAPI::Broodwar->self() || (!m_unit->isCloaked() && !m_unit->isBurrowed()) || m_unit->isDetected() )
				return UnitAccessType::Full;
			else
				return UnitAccessType::Partial;
		}

		// If it doesn't exist but its ours it must be dead, is also possible to check if the player has left the game
		// but in some game modes they don't disappear when player->leftGame() returns true, noticed in protoss champaign
		if( getPlayer()->isLocalPlayer() )
			return UnitAccessType::Dead;

		const UnitType &type = getType();
		if( MapUtil::isAnyVisible( getTilePosition(), type ) )
		{
			if( type.canMove() || type.isFlyingBuilding() || type == UnitTypes::Terran_Siege_Tank_Siege_Mode )
				return UnitAccessType::Lost;
			else
				return UnitAccessType::Dead;
		}
		else if( m_access_type != UnitAccessType::Lost )
			return UnitAccessType::Guess;
	}

	return m_access_type;
}

void SkynetUnit::drawUnitPosition() const
{
	const Position &pos = getPosition();
	const UnitType &type = getType();
	Player player = getPlayer();

	const int bar_height = 4;

	if( (!isCompleted() || isMorphing()) && accessibility() != UnitAccessType::Prediction )
	{
		float progress = static_cast<float>(getTimeTillCompleted());

		if( isMorphing() )
			progress /= getBuildType().buildTime();
		else
			progress /= type.buildTime();

		progress = 1.0f - progress;

		Color bar_colour = isMorphing() ? Colors::Red : Colors::Purple;

		Position bottom_left( pos.x - type.dimensionLeft(), pos.y + type.dimensionDown() + bar_height - 1 );

		DrawingUtil::drawProgressBar( bottom_left, type.dimensionLeft() + type.dimensionRight(), bar_height, progress, bar_colour, player->getColor() );
	}

	if( type.maxShields() > 0 )
	{
		float progress = static_cast<float>(getShield());
		progress /= type.maxShields();

		Position bottom_left( pos.x - type.dimensionLeft(), pos.y - type.dimensionUp() - bar_height + 2 );

		DrawingUtil::drawProgressBar( bottom_left, type.dimensionLeft() + type.dimensionRight(), bar_height, progress, Colors::Blue, player->getColor() );
	}

	if( type.maxHitPoints() > 0 )
	{
		float progress = static_cast<float>(getHealth());
		progress /= type.maxHitPoints();

		Position bottom_left( pos.x - type.dimensionLeft(), pos.y - type.dimensionUp() + 1 );

		DrawingUtil::drawProgressBar( bottom_left, type.dimensionLeft() + type.dimensionRight(), bar_height, progress, Colors::Green, player->getColor() );
	}

	BWAPI::Broodwar->drawBoxMap( pos.x - type.dimensionLeft(), pos.y - type.dimensionUp(), pos.x + type.dimensionRight(), pos.y + type.dimensionDown(), player->getColor() );

	BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y - 10, "%i : %s", m_id, type.getName().c_str() );
	BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y, "%s", player->getName().c_str() );

	UnitAccessType access = accessibility();
	BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y + 10, "%s", toString( access ).c_str() );

	int existTime = getTimeTillExists();
	int completeTime = getTimeTillCompleted() - existTime;
	BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y + 20, "%d : %d", existTime, completeTime );

	if( isMorphing() )
		BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y + 30, "Morphing" );
	else if( isCompleted() )
		BWAPI::Broodwar->drawTextMap( pos.x + type.dimensionRight(), pos.y + 30, "Completed" );

	Position target = getTargetPosition();
	BWAPI::Broodwar->drawLineMap( pos, target, player->getColor() );
}

void SkynetUnit::drawUnitTilePosition() const
{
	TilePosition tile = getTilePosition();
	UnitType type = getType();
	Player player = getPlayer();

	BWAPI::Broodwar->drawBoxMap( Position( tile ), Position( tile + type.tileSize() ), player->getColor() );
}

Order SkynetUnit::getOrder() const
{
	if( exists() )
		return m_unit->getOrder();

	return Orders::None;
}

Order SkynetUnit::getSecondaryOrder() const
{
	if( exists() )
		return m_unit->getSecondaryOrder();

	return Orders::None;
}

bool SkynetUnit::isLifted() const
{
	if( exists() )
		return m_unit->isLifted();

	return m_is_lifted;
}

bool SkynetUnit::isSieged() const
{
	if( exists() )
		return m_unit->isSieged();

	return m_is_sieged;
}

bool SkynetUnit::isCarryingGas() const
{
	if( exists() )
		return m_unit->isCarryingGas();

	return false;
}

bool SkynetUnit::isCarryingMinerals() const
{
	if( exists() )
		return m_unit->isCarryingMinerals();

	return false;
}

bool SkynetUnit::isLockedDown() const
{
	if( exists() )
		return m_unit->isLockedDown();

	return false;
}

bool SkynetUnit::isStasised()
{
	if( exists() && m_unit->isStasised() )
		return true;

	return false; //mAccess.getLatencyTracker().isStasisInRange( shared_from_this() );
}

bool SkynetUnit::isCloaked() const
{
	if( exists() )
		return m_unit->isCloaked();

	return false;
}

bool SkynetUnit::isBurrowed() const
{
	if( exists() )
		return m_unit->isBurrowed();

	return false;
}

bool SkynetUnit::isDetected() const
{
	if( exists() )
		return m_unit->isDetected();

	return false;
}

bool SkynetUnit::isStuck() const
{
	if( exists() )
		return m_unit->isStuck();

	return false;
}

bool SkynetUnit::isUnderStorm()
{
	if( exists() )
		if( m_unit->isUnderStorm() )
			return true;

	return false; //mAccess.getLatencyTracker().isStormInRange( shared_from_this() );
}

bool SkynetUnit::isUnderDarkSwarm() const
{
	if( exists() )
		if( m_unit->isUnderDarkSwarm() )
			return true;

	return false;
}

bool SkynetUnit::isLoaded() const
{
	if( exists() )
		return m_unit->isLoaded();

	return false;
}

bool SkynetUnit::hasAddon() const
{
	if( exists() )
		return m_unit->getAddon() != nullptr;

	return false;
}

bool SkynetUnit::canAttackNow( Unit unit ) const
{
	if( !unit || !exists() || !unit->exists() )
		return false;

	return canAttack( unit );
}

bool SkynetUnit::canAttack( Unit unit ) const
{
	if( !unit )
		return false;

	if( m_unit->isLockedDown() || m_unit->isStasised() || unit->isStasised() )
		return false;

	if( (unit->isCloaked() || unit->isBurrowed()) && !unit->isDetected() )
		return false;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return canAttackAir();
	else
		return canAttackGround();
}

bool SkynetUnit::isInRange( Unit unit ) const
{
	int weapon_max_range = getWeaponMaxRange( unit );

	if( weapon_max_range <= 0 )
		return false;

	int distance = getDistance( unit );
	int weapon_min_range = getWeaponMinRange( unit );

	if( weapon_min_range != 0 && distance <= weapon_min_range )
		return false;

	return weapon_min_range >= distance;
}

bool SkynetUnit::isAttackFrame() const
{
	if( exists() )
		return m_unit->isAttackFrame();

	return false;
}

int SkynetUnit::getGroundWeaponMaxRange() const
{
	if( getType() == UnitTypes::Protoss_Reaver )
		return 256;
	else
		return getPlayer()->weaponMaxRange( getType().groundWeapon() );
}

int SkynetUnit::getAirWeaponMaxRange() const
{
	return getPlayer()->weaponMaxRange( getType().airWeapon() );
}

int SkynetUnit::getWeaponMaxRange( Unit unit ) const
{
	if( !unit )
		return 0;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return getAirWeaponMaxRange();
	else
		return getGroundWeaponMaxRange();
}

int SkynetUnit::getWeaponMinRange( Unit unit ) const
{
	if( !unit )
		return 0;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return getType().airWeapon().minRange();
	else
		return getType().groundWeapon().minRange();
}

int SkynetUnit::getWeaponCooldown( Unit unit ) const
{
	if( !unit )
		return 0;

	if( getType() == UnitTypes::Protoss_Reaver )
		return 60;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return getType().airWeapon().damageCooldown();
	else
		return getType().groundWeapon().damageCooldown();
}

int SkynetUnit::getRemainingCooldown( Unit unit ) const
{
	if( !unit )
		return 0;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return getRemainingAirCooldown();
	else
		return getRemainingGroundCooldown();
}

int SkynetUnit::getRemainingGroundCooldown() const
{
	if( exists() )
		return m_unit->getGroundWeaponCooldown();

	return 0;
}

int SkynetUnit::getRemainingAirCooldown() const
{
	if( exists() )
		return m_unit->getAirWeaponCooldown();

	return 0;
}

int SkynetUnit::getSpellCooldown() const
{
	if( exists() )
		return m_unit->getSpellCooldown();

	return 0;
}

int SkynetUnit::getTimeToKill( Unit unit ) const
{
	int health = unit->totalHitPoints();

	BWAPI::WeaponType weapon = getWeapon( unit );

	int weapon_damage = weapon.damageAmount() + (weapon.damageFactor() * getPlayer()->getUpgradeLevel( weapon.upgradeType() ));
	if( weapon_damage == 0 )
		return 5000;

	int this_number_of_shots = health / weapon_damage;

	if( weapon.damageType() == DamageTypes::Concussive )
	{
		if( unit->getType().size() == UnitSizeTypes::Large )
			this_number_of_shots *= 4;
		else if( unit->getType().size() == UnitSizeTypes::Medium )
			this_number_of_shots += this_number_of_shots;
	}
	else if( weapon.damageType() == DamageTypes::Explosive )
	{
		if( unit->getType().size() == UnitSizeTypes::Small )
			this_number_of_shots += this_number_of_shots;
		else if( unit->getType().size() == UnitSizeTypes::Medium )
			this_number_of_shots += this_number_of_shots / 2;
	}

	if( (BWAPI::Broodwar->getGroundHeight( getTilePosition() ) - BWAPI::Broodwar->getGroundHeight( unit->getTilePosition() )) < 0 )
		this_number_of_shots += this_number_of_shots;

	return this_number_of_shots * weapon.damageCooldown();
}

WeaponType SkynetUnit::getWeapon( Unit unit ) const
{
	if( !unit )
		return WeaponTypes::None;

	if( unit->getType().isFlyer() || unit->isLifted() )
		return getType().airWeapon();
	else
	{
		if( getType() == UnitTypes::Protoss_Reaver )
			return WeaponTypes::Scarab;
		else
			return getType().groundWeapon();
	}
}

bool SkynetUnit::canAttackGround() const
{
	const UnitType &type = getType();
	if( type == UnitTypes::Protoss_Reaver )
		return getScarabCount() != 0;

	if( type == UnitTypes::Terran_Vulture_Spider_Mine )
		return true;

	return type.groundWeapon() != WeaponTypes::None;
}

bool SkynetUnit::canAttackAir() const
{
	return getType().airWeapon() != WeaponTypes::None;
}

int SkynetUnit::getScarabCount() const
{
	if( exists() )
		return m_unit->getScarabCount();

	return 0;
}

double SkynetUnit::totalHitPointFraction() const
{
	return double( totalHitPoints() ) / double( getType().maxHitPoints() + getType().maxShields() );
}

int SkynetUnit::totalHitPoints() const
{
	return getHealth() + getShield() + getDefenseMatrixPoints();
}

int SkynetUnit::getHealth() const
{
	if( exists() )
		return m_unit->getHitPoints();

	return m_health;
}

int SkynetUnit::getShield() const
{
	if( exists() )
		return m_unit->getShields();

	return m_shield;
}

int SkynetUnit::getDefenseMatrixPoints() const
{
	if( exists() )
		return m_unit->getDefenseMatrixPoints();

	return 0;
}

bool SkynetUnit::isRepairing() const
{
	if( exists() )
		return m_unit->isRepairing();

	return false;
}

bool SkynetUnit::isHealing() const
{
	if( exists() )
		return m_unit->getOrder() == Orders::MedicHeal;

	return false;
}

bool SkynetUnit::isBeingHealed() const
{
	if( exists() )
		return m_unit->isBeingHealed();

	return false;
}

bool SkynetUnit::isBeingRepaired() const
{
	return false;
}

int SkynetUnit::getEnergy() const
{
	if( exists() )
		return m_unit->getEnergy();

	return 0;
}

UnitType::list SkynetUnit::getTrainingQueue() const
{
	if( exists() )
		return m_unit->getTrainingQueue();

	return UnitType::list();
}

int SkynetUnit::getDistance( UnitType targ_type, Position position ) const
{
	return MapUtil::getDistance( getPosition(), getType(), position, targ_type );
}

int SkynetUnit::getDistance( UnitType targ_type, Position position, int inFramesTime ) const
{
	return MapUtil::getDistance( getPosition( inFramesTime ), getType(), position, targ_type );
}

int SkynetUnit::getDistance( Unit unit ) const
{
	if( !unit )
		return 0;

	return getDistance( unit->getType(), unit->getPosition() );
}

int SkynetUnit::getDistance( Unit unit, int inFramesTime ) const
{
	if( !unit )
		return 0;

	return getDistance( unit->getType(), unit->getPosition( inFramesTime ) );
}

int SkynetUnit::getDistance( Position position ) const
{
	return MapUtil::getDistance( getPosition(), getType(), position );
}

int SkynetUnit::getDistance( Position position, int inFramesTime ) const
{
	return MapUtil::getDistance( getPosition( inFramesTime ), getType(), position );
}

bool SkynetUnit::isSelected() const
{
	if( exists() )
		return m_unit->isSelected();

	return false;
}

bool SkynetUnit::hasPath( Position position ) const
{
	if( exists() )
		return m_unit->hasPath( position );

	return BWAPI::Broodwar->hasPath( getPosition(), position );
}

UnitCommand SkynetUnit::getLastCommand() const
{
	if( exists() )
		return m_unit->getLastCommand();

	return BWAPI::UnitCommand();
}

bool SkynetUnit::hasOrder( Order order ) const
{
	if( getOrder() == order )
		return true;
	else if( getSecondaryOrder() == order )
		return true;
	else
		return false;
}

Unit SkynetUnit::getBuildUnit() const
{
	if( exists() )
		return m_build_unit;

	return nullptr;
}

void SkynetUnit::train( UnitType type )
{
	if( !exists() )
		return;

	if( m_unit->getSecondaryOrder() == Orders::Train )
	{
		const auto &queue = m_unit->getTrainingQueue();
		if( queue.size() > 1 || (queue.size() == 1 && *queue.begin() != type) )
		{
			cancel();
			return;
		}

		return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Train && m_unit->getLastCommand().getUnitType() == type )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->train( type ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::gather( Unit unit )
{
	if( !exists() || !unit )
		return;

	if( !unit->exists() )
	{
		move( unit->getPosition() );
		return;
	}

	if( unit->getType() == UnitTypes::Resource_Mineral_Field )
	{
		if( m_unit->getOrder() == Orders::MoveToMinerals || m_unit->getOrder() == Orders::WaitForMinerals || m_unit->getOrder() == Orders::MiningMinerals )
		{
			if( m_unit->getOrderTarget() == unit->getBWAPIUnit() )
				return;
		}
	}
	else if( unit->getType().isRefinery() )
	{
		if( m_unit->getOrder() == Orders::Harvest1 )
			return;
		else if( m_unit->getOrder() == Orders::MoveToGas || m_unit->getOrder() == Orders::WaitForGas || m_unit->getOrder() == Orders::HarvestGas )
		{
			if( m_unit->getOrderTarget() == unit->getBWAPIUnit() )
				return;
		}
	}
	else
		return;

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Gather && m_unit->getLastCommand().getTarget() == unit->getBWAPIUnit() )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->gather( unit->getBWAPIUnit() ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::returnCargo()
{
	if( !exists() )
		return;

	if( m_unit->getOrder() == Orders::ReturnGas || m_unit->getOrder() == Orders::ReturnMinerals )
		return;

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Return_Cargo )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->returnCargo() )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::returnCargo( Unit unit )
{
	if( !exists() || !unit )
		return;

	if( !unit->exists() || !unit->isCompleted() )
	{
		move( unit->getPosition() );
		return;
	}

	if( m_unit->getOrder() == Orders::ResetCollision )
		return;

	if( m_unit->getOrder() == Orders::ReturnGas || m_unit->getOrder() == Orders::ReturnMinerals )
	{
		if( m_unit->getOrderTarget() == unit->getBWAPIUnit() )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Right_Click_Unit && m_unit->getLastCommand().getTarget() == unit->getBWAPIUnit() )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->rightClick( unit->getBWAPIUnit() ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::build( UnitType type, TilePosition target )
{
	if( !exists() )
		return;

	const Position targetPosition( target.x * 32 + type.tileWidth() * 16, target.y * 32 + type.tileHeight() * 16 );
	if( getDistance( type, targetPosition ) > 48 || !MapUtil::isAllVisible( target, type ) )
	{
		move( targetPosition, 0 );
		return;
	}

	if( m_unit->getOrder() == Orders::PlaceBuilding )
	{
		if( m_unit->getBuildType() == type && m_unit->getOrderTargetPosition() == targetPosition )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Build && m_unit->getLastCommand().getUnitType() == type && m_unit->getLastCommand().getTargetTilePosition() == target )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->build( type, target ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
	else
		move( targetPosition, 0 );
}

void SkynetUnit::build( Unit unit )
{
	if( !exists() || !unit || !unit->exists() )
		return;

	m_unit->rightClick( unit->getBWAPIUnit() );
	unit->getBWAPIUnit()->rightClick( m_unit );
}

void SkynetUnit::morph( UnitType type )
{
	if( !exists() )
		return;

	m_unit->morph( type );
}

void SkynetUnit::stop()
{
	if( !exists() )
		return;

	if( m_unit->getOrder() == Orders::Stop )
		return;

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Stop )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->stop() )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::cancel( int slot )
{
	if( !exists() )
		return;

	if( m_unit->isResearching() )
		m_unit->cancelResearch();
	else if( m_unit->isUpgrading() )
		m_unit->cancelUpgrade();
	else if( m_unit->isTraining() )
		m_unit->cancelTrain( slot );
	else if( m_unit->isMorphing() )
		m_unit->cancelMorph();
	else if( m_unit->isBeingConstructed() || !m_unit->isCompleted() )
		m_unit->cancelConstruction();
	else if( m_unit->isConstructing() )
		m_unit->haltConstruction();
	else
		return;

	return;
}

void SkynetUnit::upgrade( UpgradeType type )
{
	if( !exists() )
		return;

	m_unit->upgrade( type );
}

void SkynetUnit::research( TechType mType )
{
	if( !exists() )
		return;

	m_unit->research( mType );
}

void SkynetUnit::move( Position target, int accuracy )
{
	if( !exists() )
		return;

	if( m_unit->getOrder() == Orders::Move )
	{
		if( m_unit->getOrderTargetPosition().getApproxDistance( target ) <= accuracy )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Move && m_unit->getLastCommand().getTargetPosition().getApproxDistance( target ) <= accuracy )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
		else if( (m_unit->getOrder() == Orders::Guard || m_unit->getOrder() == Orders::PlayerGuard) && getPosition().getApproxDistance( target ) <= accuracy * 4 )
			return;
	}

	if( m_unit->move( target ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::attack( Unit unit )
{
	if( !exists() || !unit )
		return;

	if( !unit->exists() )
	{
		move( unit->getPosition() );
		return;
	}

	if( m_unit->getOrder() == Orders::AttackUnit )
	{
		if( m_unit->getOrderTarget() == unit->getBWAPIUnit() )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Attack_Unit && m_unit->getLastCommand().getTarget() == unit->getBWAPIUnit() )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->attack( unit->getBWAPIUnit() ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::attack( Position target, int accuracy )
{
	if( !exists() )
		return;

	if( m_unit->getOrder() == Orders::AttackMove )
	{
		if( m_unit->getOrderTargetPosition().getApproxDistance( target ) <= accuracy )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Attack_Move && m_unit->getLastCommand().getTargetPosition().getApproxDistance( target ) <= accuracy )
	{
		if( m_unit->getOrder() == Orders::Guard || m_unit->getOrder() == Orders::PlayerGuard )
			return;
		else if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->attack( target ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::useTech( TechType tech, Position target )
{
	if( !exists() )
		return;

	if( m_unit->getOrder() == tech.getOrder() )
	{
		if( m_unit->getOrderTargetPosition() == target )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Use_Tech_Position && m_unit->getLastCommand().getTechType() == tech && m_unit->getLastCommand().getTargetPosition() == target )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->useTech( tech, target ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::useTech( TechType tech, Unit target )
{
	if( !exists() || !target )
		return;

	if( !target->exists() )
	{
		move( target->getPosition() );
		return;
	}

	if( m_unit->getOrder() == tech.getOrder() )
	{
		if( m_unit->getOrderTarget() == target->getBWAPIUnit() )
			return;
	}

	if( m_unit->getLastCommand().getType() == UnitCommandTypes::Use_Tech_Unit && m_unit->getLastCommand().getTechType() == tech && m_unit->getLastCommand().getTarget() == target->getBWAPIUnit() )
	{
		if( m_last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
			return;
	}

	if( m_unit->useTech( tech, target->getBWAPIUnit() ) )
		m_last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
}

void SkynetUnit::promote( BWAPI::Unit unit )
{
	m_unit = unit;
	m_access_type = UnitAccessType::Full;
}

void SkynetUnit::setBuildTime( int time_till_start )
{
	m_exists_time = time_till_start;
	m_time_till_completed = time_till_start + getType().buildTime();
}

void SkynetUnit::setPosition( Position position )
{
	m_position = position;
	m_target_position = position;
}

void SkynetUnit::update( SkynetUnitTracker & unit_tracker, PlayerTrackerInterface & player_tracker )
{
	if( m_exists_time > 1 )
		--m_exists_time;
	if( m_time_till_completed > 0 )
		--m_time_till_completed;

	m_player = m_unit ? player_tracker.getPlayer( m_unit->getPlayer() ) : m_last_player;

	if( !exists() )
		return;

	m_access_type = accessibility();

	m_updated_time = BWAPI::Broodwar->getFrameCount();

	m_position = m_unit->getPosition();
	m_target_position = m_unit->getTargetPosition();
	m_type = m_unit->getType();
	m_last_player = player_tracker.getPlayer( m_unit->getPlayer() );

	m_resources = m_unit->getResources();
	m_health = m_unit->getHitPoints();
	m_shield = m_unit->getShields();

	m_is_lifted = m_unit->isLifted();
	m_is_sieged = m_unit->isSieged();

	if( m_completed && !m_unit->isCompleted() )
		m_time_till_completed = m_unit->getType().buildTime();
	m_completed = m_unit->isCompleted();

	if( !m_morphing && m_unit->isMorphing() )
		m_time_till_completed = m_unit->getBuildType().buildTime();
	m_morphing = m_unit->isMorphing();

	if( m_unit->getPlayer() == BWAPI::Broodwar->self() )
		m_time_till_completed = m_unit->getRemainingBuildTime();
	else if( m_unit->isCompleted() && !m_unit->isMorphing() )
		m_time_till_completed = 0;

	auto order_target = m_unit->getOrderTarget();
	m_order_target = order_target && order_target->exists() ? unit_tracker.getUnit( order_target ) : nullptr;

	auto target = m_unit->getTarget();
	m_target = target && target->exists() ? unit_tracker.getUnit( target ) : nullptr;

	auto build_unit = m_unit->getBuildUnit();
	m_build_unit = build_unit && build_unit->exists() ? unit_tracker.getUnit( build_unit ) : nullptr;
}

void SkynetUnit::markDead()
{
	m_access_type = UnitAccessType::Dead;
	m_unit = nullptr;
}