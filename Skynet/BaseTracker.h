#pragma once

#include "CoreModule.h"

#include "Messaging.h"
#include "Base.h"

struct BasesRecreated
{
};

class BaseTrackerInterface : public CoreModule, public MessageReporter<BasesRecreated>
{
public:
	BaseTrackerInterface( Core & core ) : CoreModule( core, "BaseTracker" ) {}

	virtual const std::vector<Base> &getAllBases() const = 0;

	virtual Base getBase( TilePosition possition ) const = 0;
};