#pragma once

#include "Types.h"

class EnemyOutline;
class EnemyTerritory
{
public:
	virtual WalkPosition getCenter() const = 0;
	virtual const std::vector<EnemyOutline *> getOutlines() const = 0;

	virtual bool isInside( WalkPosition walk_position ) const = 0;

	virtual ~EnemyTerritory() = default;
};