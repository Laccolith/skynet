#pragma once

#include "Types.h"

class EnemyTerritory;
class EnemyOutline
{
public:
	virtual const EnemyTerritory & getTerritory() const = 0;
	virtual const std::vector<WalkPosition> & getPositions() const = 0;

	virtual bool isOnOutline( WalkPosition walk_position ) const = 0;

	virtual ~EnemyOutline() = default;
};