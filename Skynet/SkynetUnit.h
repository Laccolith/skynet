#pragma once

#include "Unit.h"

class PlayerTrackerInterface;
class SkynetUnitTracker;
class SkynetUnit : public UnitInterface
{
public:
	SkynetUnit( BWAPI::Unit unit, int id, SkynetUnitTracker & unit_tracker, PlayerTrackerInterface & player_tracker );
	SkynetUnit( int id, Player player, Position pos, UnitType type, int time_till_start );

	int getID() const override { return m_id; }

	TilePosition getTilePosition() const override;
	TilePosition getTilePosition( int in_frames_time ) const override;
	WalkPosition getWalkPosition() const override;
	WalkPosition getWalkPosition( int in_frames_time ) const override;
	Position getPosition() const override;
	Position getPosition( int in_frames_time ) const override;

	Position getTargetPosition() const override;

	UnitType getType() const override;
	UnitType getLastType() const;

	Player getPlayer() const override;
	Player getLastPlayer() const;

	Unit getTarget() const override;

	BWAPI::Unit getBWAPIUnit() const override;
	bool exists() const override;
	int getTimeTillExists() const override;

	bool isMorphing() const override;
	bool isCompleted() const override;
	int getTimeTillCompleted() const override;

	bool isConstructing() const override;
	bool isBeingConstructed() const override;
	bool isTraining() const override;
	bool isResearching() const override;
	bool isUpgrading() const override;

	int getRemainingTrainTime() const override;
	int getRemainingUpgradeTime() const override;
	int getRemainingResearchTime() const override;

	int getResources() const override;

	UpgradeType getUpgrade() const override;
	TechType getTech() const override;
	UnitType getBuildType() const override;

	int getLastSeenTime() const override;

	UnitAccessType accessibility() const override;

	void drawUnitPosition() const override;
	void drawUnitTilePosition() const override;

	Order getOrder() const override;
	Order getSecondaryOrder() const override;

	bool isLifted() const override;
	bool isSieged() const override;

	bool isCarryingGas() const override;
	bool isCarryingMinerals() const override;

	bool isLockedDown() const override;
	bool isStasised() override;
	bool isCloaked() const override;
	bool isBurrowed() const override;
	bool isDetected() const override;
	bool isStuck() const override;
	bool isUnderStorm() override;
	bool isUnderDarkSwarm() const override;
	bool isLoaded() const override;

	bool hasAddon() const override;

	bool canAttackNow( Unit unit ) const override;
	bool canAttack( Unit unit ) const override;
	bool isInRange( Unit unit ) const override;
	bool isAttackFrame() const override;

	int getGroundWeaponMaxRange() const override;
	int getAirWeaponMaxRange() const override;
	int getWeaponMaxRange( Unit unit ) const override;
	int getWeaponMinRange( Unit unit ) const override;
	int getWeaponCooldown( Unit unit ) const override;
	int getRemainingCooldown( Unit unit ) const override;
	int getRemainingGroundCooldown() const override;
	int getRemainingAirCooldown() const override;
	int getSpellCooldown() const override;

	int getTimeToKill( Unit unit ) const override;
	WeaponType getWeapon( Unit unit ) const override;

	bool canAttackGround() const override;
	bool canAttackAir() const override;

	int getScarabCount() const override;

	double totalHitPointFraction() const override;
	int totalHitPoints() const override;
	int getHealth() const override;
	int getShield() const override;
	int getDefenseMatrixPoints() const override;

	bool isRepairing() const override;
	bool isHealing() const override;
	bool isBeingHealed() const override;
	bool isBeingRepaired() const override;

	int getEnergy() const override;
	UnitType::list getTrainingQueue() const override;

	int getDistance( UnitType targ_type, Position position ) const override;
	int getDistance( UnitType targ_type, Position position, int inFramesTime ) const override;
	int getDistance( Unit unit ) const override;
	int getDistance( Unit unit, int inFramesTime ) const override;
	int getDistance( Position position ) const override;
	int getDistance( Position position, int inFramesTime ) const override;

	bool isSelected() const override;

	bool hasPath( Position position ) const override;

	UnitCommand getLastCommand() const override;

	bool hasOrder( Order order ) const override;

	Unit getBuildUnit() const override;

	void train( UnitType type ) override;
	void gather( Unit unit ) override;
	void returnCargo() override;
	void returnCargo( Unit unit ) override;
	void build( UnitType type, TilePosition target ) override;
	void build( Unit unit ) override;
	void morph( UnitType type ) override;
	void stop() override;
	void cancel( int slot = -2 ) override;
	void upgrade( UpgradeType type ) override;
	void research( TechType mType ) override;
	void move( Position target, int accuracy = WALKPOSITION_SCALE ) override;
	void attack( Unit unit ) override;
	void attack( Position target, int accuracy = WALKPOSITION_SCALE ) override;
	void useTech( TechType tech, Position target ) override;
	void useTech( TechType tech, Unit target ) override;

	void promote( BWAPI::Unit unit );
	void setBuildTime( int time_till_start );
	void setPosition( Position position );

	void update( SkynetUnitTracker & unit_tracker, PlayerTrackerInterface & player_tracker, bool first_update );
	void markDead();

private:
	int m_id;
	BWAPI::Unit m_unit = nullptr;

	UnitAccessType m_access_type = UnitAccessType::Prediction;

	int m_updated_time = 0;

	Position m_position = Positions::Unknown;
	Position m_target_position = Positions::None;
	UnitType m_type = UnitTypes::Unknown;
	Player m_player = nullptr;
	Player m_last_player = nullptr;

	int m_resources = 0;
	int m_health = 0;
	int m_shield = 0;

	bool m_is_lifted = false;
	bool m_is_sieged = false;

	bool m_completed = false;
	bool m_morphing = false;
	int m_time_till_completed = 0;

	int m_exists_time = 0;
	int m_delete_timer = 0;

	int m_last_order_execute_time = 0;

	Unit m_order_target = nullptr;
	Unit m_target = nullptr;
	Unit m_build_unit = nullptr;
};