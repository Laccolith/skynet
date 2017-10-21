#pragma once

#include "SkynetInterface.h"

class TaskManagerInterface : public SkynetInterface
{
public:
	TaskManagerInterface( Access &access ) : SkynetInterface( access, "TaskManager" ) {}
};