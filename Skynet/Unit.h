#pragma once

#include "Types.h"

#include "SmartEnum.h"

SMART_ENUM( UnitAccessType, int,
	Dead,
	Lost,
	Guess,
	Prediction,
	Partial,
	Full
	);

class UnitInterface;
typedef UnitInterface *Unit;

#include "UnitGroup.h"

class UnitInterface
{
public:
	virtual TilePosition getTilePosition() const = 0;
	virtual TilePosition getTilePosition( int in_frames_time ) const = 0;
	virtual WalkPosition getWalkPosition() const = 0;
	virtual WalkPosition getWalkPosition( int in_frames_time ) const = 0;
	virtual Position getPosition() const = 0;
	virtual Position getPosition( int in_frames_time ) const = 0;

	virtual Position getTargetPosition() const = 0;

	virtual UnitType getType() const = 0;

	virtual Player getPlayer() const = 0;

	virtual Unit getTarget() const = 0;

	virtual BWAPI::Unit getBWAPIUnit() const = 0;
	virtual bool exists() const = 0;
	virtual int getExistTime() const = 0;

	virtual bool isMorphing() const = 0;
	virtual bool isCompleted() const = 0;
	virtual int getCompletedTime() const = 0;

	virtual bool isConstructing() const = 0;
	virtual bool isBeingConstructed() const = 0;
	virtual bool isTraining() const = 0;
	virtual bool isResearching() const = 0;
	virtual bool isUpgrading() const = 0;

	virtual int getRemainingTrainTime() const = 0;
	virtual int getRemainingUpgradeTime() const = 0;
	virtual int getRemainingResearchTime() const = 0;

	virtual int getResources() const = 0;

	virtual UpgradeType getUpgrade() const = 0;
	virtual TechType getTech() const = 0;
	virtual UnitType getBuildType() const = 0;

	virtual int getLastSeenTime() const = 0;

	virtual UnitAccessType accessibility() const = 0;

	virtual void drawUnitPosition() const = 0;
	virtual void drawUnitTilePosition() const = 0;

	virtual Order getOrder() const = 0;
	virtual Order getSecondaryOrder() const = 0;

	virtual bool isLifted() const = 0;
	virtual bool isSieged() const = 0;

	virtual bool isCarryingGas() const = 0;
	virtual bool isCarryingMinerals() const = 0;

	virtual bool isLockedDown() const = 0;
	virtual bool isStasised() = 0;
	virtual bool isCloaked() const = 0;
	virtual bool isBurrowed() const = 0;
	virtual bool isDetected() const = 0;
	virtual bool isStuck() const = 0;
	virtual bool isUnderStorm() = 0;
	virtual bool isUnderDarkSwarm() const = 0;
	virtual bool isLoaded() const = 0;

	virtual bool hasAddon() const = 0;

	virtual bool canAttackNow( Unit unit ) const = 0;
	virtual bool canAttack( Unit unit ) const = 0;
	virtual bool isInRange( Unit unit ) const = 0;
	virtual bool isAttackFrame() const = 0;

	virtual int getGroundWeaponMaxRange() const = 0;
	virtual int getAirWeaponMaxRange() const = 0;
	virtual int getWeaponMaxRange( Unit unit ) const = 0;
	virtual int getWeaponMinRange( Unit unit ) const = 0;
	virtual int getWeaponCooldown( Unit unit ) const = 0;
	virtual int getRemainingCooldown( Unit unit ) const = 0;
	virtual int getRemainingGroundCooldown() const = 0;
	virtual int getRemainingAirCooldown() const = 0;
	virtual int getSpellCooldown() const = 0;

	virtual int getTimeToKill( Unit unit ) const = 0;
	virtual WeaponType getWeapon( Unit unit ) const = 0;

	virtual bool canAttackGround() const = 0;
	virtual bool canAttackAir() const = 0;

	virtual int getScarabCount() const = 0;

	virtual double totalHitPointFraction() const = 0;
	virtual int totalHitPoints() const = 0;
	virtual int getHealth() const = 0;
	virtual int getShield() const = 0;
	virtual int getDefenseMatrixPoints() const = 0;

	virtual bool isRepairing() const = 0;
	virtual bool isHealing() const = 0;
	virtual bool isBeingHealed() const = 0;
	virtual bool isBeingRepaired() const = 0;

	virtual int getEnergy() const = 0;
	virtual UnitType::list getTrainingQueue() const = 0;

	virtual int getDistance( UnitType targ_type, Position position ) const = 0;
	virtual int getDistance( UnitType targ_type, Position position, int inFramesTime ) const = 0;
	virtual int getDistance( Unit unit ) const = 0;
	virtual int getDistance( Unit unit, int inFramesTime ) const = 0;
	virtual int getDistance( Position position ) const = 0;
	virtual int getDistance( Position position, int inFramesTime ) const = 0;

	virtual bool isSelected() const = 0;

	virtual bool hasPath( Position position ) const = 0;

	virtual UnitCommand getLastCommand() const = 0;

	virtual bool hasOrder( Order order ) const = 0;

	virtual Unit getBuildUnit() const = 0;

	virtual void train( UnitType type ) = 0;
	virtual void gather( Unit unit ) = 0;
	virtual void returnCargo() = 0;
	virtual void returnCargo( Unit unit ) = 0;
	virtual void build( UnitType type, TilePosition target ) = 0;
	virtual void build( Unit unit ) = 0;
	virtual void morph( UnitType type ) = 0;
	virtual void stop() = 0;
	virtual void cancel( int slot = -2 ) = 0;
	virtual void upgrade( UpgradeType type ) = 0;
	virtual void research( TechType mType ) = 0;
	virtual void move( Position target, int accuracy = WALKPOSITION_SCALE ) = 0;
	virtual void attack( Unit unit ) = 0;
	virtual void attack( Position target, int accuracy = WALKPOSITION_SCALE ) = 0;
	virtual void useTech( TechType tech, Position target ) = 0;
	virtual void useTech( TechType tech, Unit target ) = 0;
};