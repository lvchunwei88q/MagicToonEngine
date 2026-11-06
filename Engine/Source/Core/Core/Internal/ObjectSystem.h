#pragma once
#include "Subsystem/SubsystemTemplate.h"
#include "Object.h"
#include <vector>
#include <mutex>

#define PRE_ALLOCATED_SIZE 1024

namespace Core {
	struct ObjectContainer {
		ObjectRef Object;
		ObjectSystemHandle Handle;
	};

	class ObjectSystem : public SubsystemTemplate<ObjectSystem, Priority::Core>, public IObjectSystem
	{
	public:
		virtual bool Init() override;
		virtual void Uninstall() override;

		virtual size_t GetObjectNum() override;
		virtual ObjectSystemHandle RegisterObject(Object* ptr) override;
		virtual void RemoveObject(ObjectSystemHandle Handle) override;

	private:
		std::vector<ObjectContainer> Objects;
		mutable std::mutex m_mutex;
	};
}
