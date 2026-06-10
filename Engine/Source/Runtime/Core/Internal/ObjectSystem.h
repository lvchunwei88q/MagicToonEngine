#pragma once
#include "Subsystem/SubsystemTemplate.h"
#include "Object.h"
#include <vector>
#include <string>

namespace Core {

	// 对象系统，负责管理所有的对象实例
	class ObjectSystem : public SubsystemTemplate<ObjectSystem,Priority::Core> , public IObjectSystem
	{
	public:
		virtual bool Init() override;
		virtual void Uninstall() override;

		virtual size_t GetObjectNum() override;
	private:
		// 我们需要在这里管理Object的序列化数据
		std::vector<std::string> ObjectTags;
	};
}