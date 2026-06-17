#pragma once
#include "Common/Core_API.h"
#include <Common/Compiler.h>
#include <string>
#include <vector>
#include "Tools/Check.h"
#include "Tools/EnumClassFlags.h"

// Enumerates the features you can use
enum {
	MREFLECTION,
	MSERIALIZATION,
};

#define MMEMBER(...)				// 你要序列化的成员
#define MCLASS(...)					// 你要序列化的类，必须放在类定义的开头
#define GENERATE_BODY()				// 你要序列化的类的成员函数实现，必须放在类定义的结尾

FORBIDDEN_METHOD_CONCEPT(Object, GetClassId);
FORBIDDEN_METHOD_CONCEPT(Object, GetInstanceId);
FORBIDDEN_METHOD_CONCEPT(Object, GetNextId);

template<typename T>
concept COMBINE_FORBIDDEN_NAME(Object) = COMBINE_FORBIDDEN_METHODS(Object, GetClassId) ||
			COMBINE_FORBIDDEN_METHODS(Object, GetInstanceId) || COMBINE_FORBIDDEN_METHODS(Object, GetNextId);

DISABLE_DLL_WARNINGS_PUSH;
namespace Core {
	enum class ObjectSwitch : uint8_t {
		Unknown			= 0,
		Reflection		= 1 << 0,
		Serialization	= 1 << 1
	};
	ENUM_CLASS_FLAGS(ObjectSwitch);

	enum class ObjectType : uint8_t {
		Unknown,
		ENGINE,
		PROJECT
	};

	class IObjectSystem;
	class Object;

	using ObjectRef = Object*;

	/*
	* Object Handle in System
	*/
	class CORE_API ObjectSystemHandle {
	public:
		ObjectSystemHandle() = default;
		ObjectSystemHandle(size_t index) 
		: index(index) {}

		const size_t GetIndex() const { return index; }

	private:
		// this object in objectsystem postion
		size_t index;
	};

	// ---------------------------------------------------------------------------------- MSERIALIZATION
	/*
	* Object Serialization Descriptor
	*/
	struct ObjectSerializationDescriptor {
		uint8_t* DataStart;
		size_t Length;
	};
	struct ObjectSerializationData {
		std::vector<uint8_t> data;
	};
	// ---------------------------------------------------------------------------------- MSERIALIZATION END

	/*
	* The base class for MagicHeaderTool services. Any class that wants to use advanced C features should inherit from this class.
	* Note that all Objects will be automatically serialized, but they can only be initialized after the SubsystemCore has completed initialization.
	*/
	class CORE_API Object
	{
	public:
		virtual ~Object() = default;
		Object() : instance_id(GetNextId()) {};

		virtual uint64_t GetClassId() const = 0;					// Class ID generated using MagicHeaderTool
		virtual uint64_t GetClassHas() const = 0;					// Class Has generated using MagicHeaderTool
		virtual ObjectType GetClassType() const = 0;				// Class type generated using MagicHeaderTool
		virtual ObjectSwitch GetClassSwitch() const = 0;			// Class Switch generated using MagicHeaderTool
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

	protected:
		// We need to use helper functions during initialization after the subclass is constructed or before it's destructed.
		void ObjectInit();
		void ObjectUninit();

	private:
		// We're implementing the functionality of Object here
		// -------------------------------- serialization
		void Serialization();
		void Deserialization();
		// -------------------------------- 

	protected:
		uint64_t	instance_id;						// The id of each instance
		ObjectSystemHandle Handle = {};					// Object Handle
	};

	class CORE_API IObjectSystem {
	public:
		virtual size_t GetObjectNum() = 0;								// Get Current ObjectNum
		virtual ObjectSystemHandle RegisterObject(Object* ptr) = 0;		// Register This Object For ObjectSystem Return Handle
		virtual void RemoveObject(ObjectSystemHandle Handle) = 0;		// Delete This Object Input this Handle

		// Functions used by the object class
	private:
		friend class Object;
		// ---------------------------------------------------------------------------------- MSERIALIZATION
		virtual ObjectSerializationDescriptor GetObjectSerializationData(ObjectSystemHandle Handle) = 0;
		virtual void SaveObjectSerializationData(ObjectSerializationData Descriptor) = 0;
		// ---------------------------------------------------------------------------------- MSERIALIZATION END
	};

	CORE_API IObjectSystem* GetObjectSystem();

}
DISABLE_DLL_WARNINGS_POP;