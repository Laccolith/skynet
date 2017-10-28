#pragma once

#include "Types.h"

#include "VectorUnique.h"

class UnitGroup : public VectorUnique<Unit>
{
public:
	SizeType countCompletedIn( int time ) const;

	Position getCenter() const;

	UnitGroup getBestFittingToCircle( int circle_size, int in_frames_time = 0 ) const;
	UnitGroup inRadius( int radius, Position position ) const;

	Unit getClosestUnit( Unit unit ) const;
	Unit getClosestUnit( Position position ) const;

	bool isAnyInRange( Unit unit ) const;
	bool isAnyInRange( const UnitGroup &other ) const;
	int minDistanceBetween( const UnitGroup &other ) const;

	int getBuildScore() const;

	int getAverageTerrainHeight() const;
	float getAverageSpeed() const;
	bool canAttackAir() const;
	bool canAttackGround() const;

	float majorityThatCanAttack( const UnitGroup &other ) const;

	SizeType getDetectionCount() const;
	SizeType getCloakedCount() const;
	SizeType getFlyingCount() const;

	std::vector<UnitGroup> getClusters( int distance, int min_size ) const;

	void sortByDistance( Position position );
};

const UnitGroup empty_unit_group;