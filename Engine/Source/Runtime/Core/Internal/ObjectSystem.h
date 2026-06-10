#pragma once
#include "Subsystem/SubsystemTemplate.h"
#include "Object.h"
#include <vector>
#include <string>
#include <mutex>

namespace Core {

	// 对象系统，负责管理所有的对象实例
	class ObjectSystem : public SubsystemTemplate<ObjectSystem,Priority::Core> , public IObjectSystem
	{
	public:
		virtual bool Init() override;
		virtual void Uninstall() override;

		virtual size_t GetObjectNum() override;
		virtual size_t RegisterObject(Object* ptr) override;
		virtual void RemoveObject(size_t index) override;
	private:
		// There is absolutely no management of their deletions here.
		std::vector<Object*> Objects;

		mutable std::mutex m_mutex;
	};
}