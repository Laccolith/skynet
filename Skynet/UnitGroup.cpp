#include "UnitGroup.h"

#include "Unit.h"

UnitGroup::UnitGroup() = default;

UnitGroup::UnitGroup( const UnitGroup & other )
	: m_units( other.m_units )
{
}

UnitGroup::UnitGroup( UnitGroup && other )
	: m_units( std::move( other.m_units ) )
{
}

UnitGroup &UnitGroup::operator=( const UnitGroup &other )
{
	m_units = other.m_units;
	return *this;
}

UnitGroup &UnitGroup::operator=( UnitGroup &&other )
{
	m_units = std::move( other.m_units );
	return *this;
}

UnitGroup & UnitGroup::operator+=( const UnitGroup &other )
{
	for( auto other_group_unit : other )
		insert( other_group_unit );

	return *this;
}

UnitGroup UnitGroup::operator+( const UnitGroup &other ) const
{
	return UnitGroup( *this ) += other;
}

UnitGroup::SizeType UnitGroup::countCompletedBy( int time ) const
{
	const int &timeNow = BWAPI::Broodwar->getFrameCount();

	int total = 0;
	for( auto unit : m_units )
	{
		if( unit->isCompleted() || unit->getCompletedTime() <= time )
			++total;
	}
	return total;
}

Position UnitGroup::getCenter() const
{
	if( m_units.empty() )
		return Positions::None;

	if( m_units.size() == 1 )
		return m_units[0]->getPosition();

	PositionFloat position;

	for( auto group_unit : m_units )
		position += group_unit->getPosition();

	return position / float(m_units.size());
}

UnitGroup UnitGroup::getBestFittingToCircle( int circle_size, int in_frames_time ) const
{
	UnitGroup circle_units( *this );
	while( true )
	{
		Position center = circle_units.getCenter();

		ConstIterator furthest_it = circle_units.end();
		int furthest_distance = circle_size;

		for( auto it = circle_units.begin(); it != circle_units.end(); ++it )
		{
			auto unit = *it;
			int current_distance = in_frames_time == 0 ? unit->getDistance( center ) : unit->getDistance( center, in_frames_time );

			if( current_distance > furthest_distance )
			{
				furthest_it = it;
				furthest_distance = current_distance;
			}
		}

		if( furthest_it == circle_units.end() )
			break;
		
		circle_units.erase( furthest_it );
	}

	return circle_units;
}

UnitGroup UnitGroup::inRadius( int radius, Position position ) const
{
	UnitGroup result;
	for( auto group_unit : m_units )
	{
		if( group_unit->getDistance( position ) < radius )
			result.insert( group_unit );
	}
	return result;
}

Unit UnitGroup::getClosestUnit( Unit unit ) const
{
	Unit closest_unit = nullptr;
	int closest_distance = std::numeric_limits<int>::max();

	for( auto group_unit : m_units )
	{
		int this_distance = group_unit->getDistance( unit );
		if( this_distance < closest_distance )
		{
			closest_distance = this_distance;
			closest_unit = group_unit;
		}
	}

	return closest_unit;
}

Unit UnitGroup::getClosestUnit( Position position ) const
{
	Unit closest_unit = nullptr;
	int closest_distance = std::numeric_limits<int>::max();

	for( auto group_unit : m_units )
	{
		int this_distance = group_unit->getDistance( position );
		if( this_distance < closest_distance )
		{
			closest_distance = this_distance;
			closest_unit = group_unit;
		}
	}

	return closest_unit;
}

bool UnitGroup::isAnyInRange( Unit unit ) const
{
	for( auto group_unit : m_units )
	{
		if( group_unit->isInRange( unit ) )
			return true;
	}

	return false;
}

bool UnitGroup::isAnyInRange( const UnitGroup &other ) const
{
	for( auto group_unit : m_units )
	{
		for( auto other_group_unit : other )
		{
			if( group_unit->isInRange( other_group_unit ) )
				return true;
		}
	}

	return false;
}

int UnitGroup::minDistanceBetween( const UnitGroup &other ) const
{
	int min_distance = std::numeric_limits<int>::max();

	for( auto group_unit : m_units )
	{
		for( auto other_group_unit : other )
		{
			int this_distance = other_group_unit->getDistance( group_unit );
			if( this_distance < min_distance )
				min_distance = this_distance;
		}
	}
	return min_distance;
}

int UnitGroup::getBuildScore() const
{
	int rating = 0;
	for( auto group_unit : m_units )
	{
		rating += group_unit->getType().buildScore();
	}

	return rating;
}

int UnitGroup::getAverageTerrainHeight() const
{
	int height = 0;
	for( auto group_unit : m_units )
		height += BWAPI::Broodwar->getGroundHeight( group_unit->getTilePosition() );

	return height / size();
}

float UnitGroup::getAverageSpeed() const
{
	float averageSpeed = 0;
	for( auto group_unit : m_units )
		averageSpeed += static_cast<float>(group_unit->getType().topSpeed());

	return averageSpeed / size();
}

bool UnitGroup::canAttackAir() const
{
	for( auto group_unit : m_units )
	{
		if( group_unit->canAttackAir() )
			return true;
	}

	return false;
}

bool UnitGroup::canAttackGround() const
{
	for( auto group_unit : m_units )
	{
		if( group_unit->canAttackGround() )
			return true;
	}

	return false;
}

float UnitGroup::majorityThatCanAttack( const UnitGroup &other ) const
{
	if( empty() || other.empty() )
		return false;

	float attack_count = 0.0;
	for( auto group_unit : m_units )
	{
		for( auto other_group_unit : other )
		{
			if( group_unit->canAttack( other_group_unit ) )
				++attack_count;
		}
	}

	attack_count /= other.size();
	attack_count /= size();

	return attack_count;
}

UnitGroup::SizeType UnitGroup::getDetectionCount() const
{
	UnitGroup::SizeType count = 0;
	for( auto group_unit : m_units )
	{
		if( group_unit->getType().isDetector() )
			++count;
	}

	return count;
}

UnitGroup::SizeType UnitGroup::getCloakedCount() const
{
	UnitGroup::SizeType count = 0;
	for( auto group_unit : m_units )
	{
		if( group_unit->getType().hasPermanentCloak() || group_unit->isCloaked() )
			++count;
	}

	return count;
}

UnitGroup::SizeType UnitGroup::getFlyingCount() const
{
	UnitGroup::SizeType count = 0;
	for( auto group_unit : m_units )
	{
		if( group_unit->getType().isFlyer() || group_unit->isLifted() )
			++count;
	}

	return count;
}

void UnitGroup::insert( Unit unit )
{
	if( std::find( m_units.begin(), m_units.end(), unit ) == m_units.end() )
		m_units.push_back( unit );
}