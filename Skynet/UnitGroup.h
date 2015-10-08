#pragma once

#include "Unit.h"

class UnitGroup
{
public:
	typedef std::vector<Unit> ContainerType;
	typedef ContainerType::value_type ValueType;
	typedef ContainerType::size_type SizeType;

	typedef ContainerType::const_iterator ConstIterator;

	UnitGroup();
	UnitGroup( const UnitGroup &other );
	UnitGroup( UnitGroup &&other );
	
	UnitGroup &operator=( const UnitGroup &other );
	UnitGroup &operator=( UnitGroup &&other );
	UnitGroup &operator+=( const UnitGroup &other );
	UnitGroup operator+( const UnitGroup &other ) const;

	SizeType countCompletedBy( int time ) const;

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

	SizeType size() const { return m_units.size(); }
	bool empty() const { return m_units.empty(); }

	void insert( Unit unit );

	void reserve( SizeType size ) { m_units.reserve( size ); }

	ConstIterator begin() const { return m_units.begin(); }
	ConstIterator end() const { return m_units.end(); }

	ConstIterator erase( ConstIterator it ) { return m_units.erase( it ); }
	ConstIterator erase( ConstIterator first, ConstIterator second ) { return m_units.erase( first, second ); }

	void clear() { m_units.clear(); }
	void swap( UnitGroup &other ) { m_units.swap( other.m_units ); }

private:
	ContainerType m_units;
};

const UnitGroup empty_unit_group;