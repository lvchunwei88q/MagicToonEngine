#pragma once
#include "Common/Core_API.h"
#include <string>

// Enumerates the features you can use
enum {
	MREFLECTION,
	MSERIALIZATION,
};

#define MMEMBER(...)				// 你要序列化的成员
#define MCLASS(...)					// 你要序列化的类，必须放在类定义的开头
#define GENERATE_BODY()				// 你要序列化的类的成员函数实现，必须放在类定义的结尾

namespace Core {
	/*
	* The base class for MagicHeaderTool services. Any class that wants to use advanced C features should inherit from this class.
	*/
	class CORE_API Object
	{
	public:
		virtual ~Object();
		Object();

		virtual uint64_t GetClassId() const = 0;					// Class ID generated using MagicHeaderTool
		uint64_t GetInstanceId() const { return instance_id; };		// Assign a unique ID to each object instance

		template<class Archive>
		void serialize(Archive& archive) {
			// NOT
		}
	private:
		static uint64_t GetNextId() {
			static uint64_t counter = 0;
			return ++counter;
		}

		uint64_t instance_id;
	};

	class CORE_API IObjectSystem {
	public:
		virtual size_t GetObjectNum() = 0;
	};

	CORE_API IObjectSystem* GetObjectSystem();
}