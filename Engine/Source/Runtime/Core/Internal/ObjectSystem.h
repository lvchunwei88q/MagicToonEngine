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
		virtual ObjectSystemHandle RegisterObject(Object* ptr) override;
		virtual void RemoveObject(ObjectSystemHandle Handle) override;
		// ---------------------------------------------------------------------------------- MSERIALIZATION
		virtual ObjectSerializationDescriptor GetObjectSerializationData(ObjectSystemHandle Handle) override;
		virtual void SaveObjectSerializationData(ObjectSerializationData Descriptor) override;
		// ---------------------------------------------------------------------------------- MSERIALIZATION END
	private:
		// There is absolutely no management of their deletions here.
		std::vector<ObjectRef> Objects;

		mutable std::mutex m_mutex;

		std::wstring EngineSerializedDataDir;
		std::wstring ProjectSerializedDataDir;
	};
}