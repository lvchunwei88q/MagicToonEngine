#pragma once
#include "Common/Core_API.h"
#include <string>
#include "Tools/Check.h"

// Enumerates the features you can use
enum {
	MREFLECTION,
	MSERIALIZATION,
};

#define MMEMBER(...)				// 你要序列化的成员
#define MCLASS(...)					// 你要序列化的类，必须放在类定义的开头
#define GENERATE_BODY()				// 你要序列化的类的成员函数实现，必须放在类定义的结尾

DEFINE_MEMBER_CHECKER_BEGIN(Object)
	DEFINE_MEMBER_CHECKER_ADD(GetClassId)
	DEFINE_MEMBER_CHECKER_ADD(GetInstanceId)
DEFINE_MEMBER_CHECKER_END(GetNextId)

namespace Core {
	/*
	* The base class for MagicHeaderTool services. Any class that wants to use advanced C features should inherit from this class.
	* Note that all Objects will be automatically serialized, but they can only be initialized after the SubsystemCore has completed initialization.
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
		virtual size_t GetObjectNum() = 0;					// Get Current ObjectNum
		virtual size_t RegisterObject(Object* ptr) = 0;		// Register This Object For ObjectSystem Return Index
		virtual void RemoveObject(size_t index) = 0;		// Delete This Object Input this Index 
	};

	CORE_API IObjectSystem* GetObjectSystem();

}