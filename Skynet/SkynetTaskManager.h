#pragma once

#include "TaskManager.h"

class SkynetTaskManager : public TaskManagerInterface
{
public:
	SkynetTaskManager( Access & access );

private:
	DEFINE_DEBUGGING_INTERFACE( Default );
};