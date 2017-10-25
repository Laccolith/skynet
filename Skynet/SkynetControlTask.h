#pragma once

#include "ControlTask.h"

class SkynetControlTaskFactory;
class CoreAccess;
class SkynetControlTask : public ControlTask
{
public:
	SkynetControlTask( SkynetControlTaskFactory & skynet_control_task_factory );

	virtual ~SkynetControlTask();

	virtual void preUpdate() = 0;
	virtual void postUpdate() = 0;

protected:
	SkynetControlTaskFactory & getFactory() { return m_skynet_control_task_factory; }
	const SkynetControlTaskFactory & getFactory() const { return m_skynet_control_task_factory; }

	CoreAccess & getAccess();
	const CoreAccess & getAccess() const;

private:
	SkynetControlTaskFactory & m_skynet_control_task_factory;
};