#pragma once

/*
*  The 'capabilities' this object provides only allow single inheritance,
*  but you don't need to worry about extending functionality because in our architecture,
*  objects inherit classes responsible for implementing module functions.
*  Later, we have multiple functions, and now we can freely combine these classes.

┌───────────────────────────────────────────────────────────────┐
│  Object (Capability provider)									│
│  背后有 ObjectSystem 管理:										│
│  - 序列化/反序列化												│
│  - 对象生命周期管理												│
│  - Handle/ID 管理												│
└───────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────────────────┐
│  Ability Achiever (Implements Abilities)						│
│  - NetworkSyncAbility (网络同步能力)							│
│  - MovementAbility (移动能力)									│
│  - CombatAbility (战斗能力)										│
│  - HealthAbility (生命值能力)									│
│  - InventoryAbility (背包能力)									│
└───────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌───────────────────────────────────────────────────────────────┐
│  工作类 (Worker Classes) - 像搭积木一样拼接						│
│																│
│  class Player {												│
│      MovementAbility Move;      ← 能力1						│
│      CombatAbility Combat;      ← 能力2						│
│      HealthAbility Health;      ← 能力3						│
│  };															│
│																│
│  class Enemy : public Object {								│
│      MovementAbility Move;      ← 复用能力1						│
│      HealthAbility Health;      ← 复用能力3						│
│      AIControl Ability AI;      ← 能力4						│
│  };															│
│																│
│  class NetworkPlayer : public Object {						│
│      NetworkSyncAbility Net;   ← 能力5							│
│      MovementAbility Move;      ← 复用能力1						│
│  };															│
└───────────────────────────────────────────────────────────────┘
*/

#include "Common/Core_API.h"
#include "Core.h"
#include <Common/Check.h>
#include <string>
#include <vector>
#include <memory>
#include <cereal/archives/binary.hpp>

#include "Tools/EnumClassFlags.h"

FORBIDDEN_METHOD_CONCEPT(Object, GetClassId);
FORBIDDEN_METHOD_CONCEPT(Object, GetInstanceId);
FORBIDDEN_METHOD_CONCEPT(Object, GetNextId);

template<typename T>
concept COMBINE_FORBIDDEN_NAME(Object) = COMBINE_FORBIDDEN_METHODS(Object, GetClassId) ||
			COMBINE_FORBIDDEN_METHODS(Object, GetInstanceId) || COMBINE_FORBIDDEN_METHODS(Object, GetNextId);

DISABLE_DLL_WARNINGS_PUSH;

// This class helps users build objects
class ObjectFactory {
public:
	template<typename T, typename... Args>
	static T* Create(Args&&... args) {
		// Construct object
		T* obj = new T(std::forward<Args>(args)...);
		obj->generate_object_init_func_();			// this func from MagicHeaderTool

		return obj;
	}

	template<typename T, typename... Args>
	static std::shared_ptr<T> CreateShared(Args&&... args) {
		auto obj = std::make_shared<T>(std::forward<Args>(args)...);
		obj->generate_object_init_func_();			// this func from MagicHeaderTool
		return obj;
	}
};

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

	enum class DestructionMode : uint8_t {
		SaveAndDestroy,
		DestroyOnly,
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
		ObjectSystemHandle(size_t id,size_t index, ObjectType Type)
		: id(id), index(index), Type(Type) {}

		bool operator==(const ObjectSystemHandle& other) const {
			return index == other.index && Type == other.Type;
		}

		const size_t GetId()		const { return id;		}
		const size_t GetIndex()		const { return index;	}
		const ObjectType GetType()	const { return Type;	}

	private:
		// this object in objectsystem postion
		size_t id;
		size_t index;
		ObjectType Type = ObjectType::Unknown;
	};

	// ---------------------------------------------------------------------------------- MSERIALIZATION
	/*
	* Object Serialization Descriptor
	*/
	struct ObjectSerializationDescriptor {
		size_t Length = 0;
		const uint8_t* DataStart = nullptr;
	};
	struct ObjectSerializationData {
		std::vector<uint8_t> data;
		ObjectSystemHandle handle;
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

		// If we don't need to serialize this object when we destroy it at the end, we can set the destruction mode to prevent its serialization.
		void SetDestructionMode(DestructionMode NewMode) { Mode = NewMode; }
		//template<class Archive>
		//void serialize(Archive& archive,const std::uint32_t version)
		//{
			// NOT
		//}
	private:
		static uint64_t GetNextId() {
			static uint64_t counter = 0;
			return ++counter;
		}

	protected:
		// We need to use helper functions during initialization after the subclass is constructed or before it's destructed.
		void ObjectInit();
		void ObjectUninit();

		// The serialization function that subclasses must implement
		virtual void serialized_data_generation(std::ostringstream& stream) = 0;
		virtual void deserialization_data_generation(std::stringstream& stream) = 0;
	private:
		// We're implementing the functionality of Object here
		// -------------------------------- serialization
		void Serialization();
		void Deserialization();
		// -------------------------------- 

	protected:
		uint64_t	instance_id;						// The id of each instance
		ObjectSystemHandle Handle = {};					// Object Handle
	private:
		DestructionMode Mode = DestructionMode::SaveAndDestroy;		// Serialization is required by default
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