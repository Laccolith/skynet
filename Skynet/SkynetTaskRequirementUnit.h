#pragma once

#include "SkynetTaskRequirement.h"

#include "UnitGroup.h"
#include "BuildLocation.h"

#include <memory>

class SkynetTaskRequirementUnitPositionBase
{
public:
	virtual ~SkynetTaskRequirementUnitPositionBase() {}

	virtual void get( int & time, Position & starting_position, Position & ending_position ) = 0;
	virtual void reserve( int time ) = 0;
	virtual void freeReservation() = 0;

	virtual TilePosition getBuildPosition() const = 0;
};

class SkynetTaskRequirementUnitType : public SkynetTaskRequirementUnit
{
public:
	SkynetTaskRequirementUnitType( UnitType unit_type, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position );
	SkynetTaskRequirementUnitType( UnitType unit_type, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position );

	int getReserveEarliestTime( CoreAccess & access, int current_earliest_time ) override;
	void freeReserved( CoreAccess & access ) override;

	Unit getChosenUnit() const override;
	TilePosition getBuildPosition() const override;

	int getRemainingUnitTime( CoreAccess & access ) const override;
	void requestUnitTimeChange( CoreAccess & access, int time ) override;

private:
	UnitType m_unit_type;
	int m_duration;
	std::unique_ptr<SkynetTaskRequirementUnitPositionBase> m_position;

	Unit m_chosen_unit = nullptr;

	int chooseUnit( CoreAccess & access, int current_earliest_time, const UnitGroup & applicable_units, Position starting_position, Position ending_position );
};

class SkynetTaskRequirementUnitSpecific : public SkynetTaskRequirementUnit
{
public:
	SkynetTaskRequirementUnitSpecific( Unit unit, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position );
	SkynetTaskRequirementUnitSpecific( Unit unit, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position );

	int getReserveEarliestTime( CoreAccess & access, int current_earliest_time ) override;
	void freeReserved( CoreAccess & access ) override;

	Unit getChosenUnit() const override;
	TilePosition getBuildPosition() const override;

	int getRemainingUnitTime( CoreAccess & access ) const override;
	void requestUnitTimeChange( CoreAccess & access, int time ) override;

private:
	Unit m_unit;
	int m_duration;
	std::unique_ptr<SkynetTaskRequirementUnitPositionBase> m_position;

	bool m_is_reserved = false;
};

class SkynetTaskRequirementUnitPosition : public SkynetTaskRequirementUnitPositionBase
{
public:
	SkynetTaskRequirementUnitPosition( Position starting_position = Positions::None, Position ending_position = Positions::None );

	void get( int & time, Position & starting_position, Position & ending_position ) override;
	void reserve( int time ) override;
	void freeReservation() override;

	TilePosition getBuildPosition() const override;

private:
	Position m_starting_position;
	Position m_ending_position;
};

class SkynetTaskRequirementUnitPositionBuildLocation : public SkynetTaskRequirementUnitPositionBase
{
public:
	SkynetTaskRequirementUnitPositionBuildLocation( std::unique_ptr<BuildLocation> build_location );

	void get( int & time, Position & starting_position, Position & ending_position ) override;
	void reserve( int time ) override;
	void freeReservation() override;

	TilePosition getBuildPosition() const override;

private:
	std::unique_ptr<BuildLocation> m_build_location;
};
