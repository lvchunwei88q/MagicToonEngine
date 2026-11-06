#pragma once

/*						Entity Component System
*  The 'capabilities' this object provides only allow single inheritance,
*  but you don't need to worry about extending functionality because in our architecture,
*  objects inherit classes responsible for implementing module functions.
*  Later, we have multiple functions, and now we can freely combine these classes.

						  System
┌───────────────────────────────────────────────────────────────┐
│  Object (Capability provider)									│
│  背后有 ObjectSystem 管理:										│
│  - 序列化/反序列化												│
│  - 对象生命周期管理												│
│  - Handle/ID 管理												│
└───────────────────────────────────────────────────────────────┘
                            │
                            ▼
						  Component
┌───────────────────────────────────────────────────────────────┐
│  Ability Achiever (Implements Abilities)						│
│  - NetworkSyncAbility (网络同步能力)							│
│  - MovementAbility (移动能力)									│
│  - CombatAbility (战斗能力)										│
│  - HealthAbility (生命值能力)									│
│  - InventoryAbility (背包能力)									│
└───────────────────────────────────────────────────────────────┘
                            ▲
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
						  Entity
┌───────────────────────────────────────────────────────────────┐
│  工作类 (Worker Classes) - 像搭积木一样拼接						│
│																│
│  class Player {												│
│      MovementAbility Move;      ← 能力1						│
│      CombatAbility Combat;      ← 能力2						│
│      HealthAbility Health;      ← 能力3						│
│  };															│
│																│
│  class Enemy {												│
│      MovementAbility Move;      ← 复用能力1						│
│      HealthAbility Health;      ← 复用能力3						│
│      AIControl Ability AI;      ← 能力4						│
│  };															│
│																│
│  class NetworkPlayer {										│
│      NetworkSyncAbility Net;   ← 能力5							│
│      MovementAbility Move;      ← 复用能力1						│
│  };															│
└───────────────────────────────────────────────────────────────┘
*/

#include "CoreMinimal.h"
#include <string>
#include <vector>
#include <memory>
#include <cereal/archives/binary.hpp>

#include "Tools/EnumClassFlags.h"
#include "Tools/BinaryRW.h"

///////////////////////////////// From Windows 
#undef GetClassName

//////////////////////////////// We set up corresponding serialization template functions for these mathematical properties
template<class Archive, typename T>
void serialize(Archive& archive, Vec4<T>& v) {
	archive(v.x, v.y, v.z, v.w);
}

template<class Archive, typename T>
void serialize(Archive& archive, Vec3<T>& v) {
	archive(v.x, v.y, v.z);
}

template<class Archive, typename T>
void serialize(Archive& archive, Vec2<T>& v) {
	archive(v.x, v.y);
}
////////////////////////////////

// --------------------------------------- Object Macro ------------------------------------------ // 

// Enumerates the features you can use
enum {
	MSERIALIZATION,
};

#define MCLASS(...)					// The class you want to serialize must be placed at the beginning of the class definition
#define GENERATE_BODY()				// The member function implementation of the class to be serialized must be placed at the end of the class definition.
#define MMEMBER(...)				// Member to serialize

// --------------------------------------- Object Macro End ------------------------------------------ // 

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

	template<typename T, typename... Args>
	static std::unique_ptr<T> CreateUnique(Args&&... args) {
		auto obj = std::make_unique<T>(std::forward<Args>(args)...);
		obj->generate_object_init_func_();  // this func from MagicHeaderTool
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
			return index == other.index && Type == other.Type && id == other.id;
		}

		const size_t GetId()		const { return id;		}
		const size_t GetIndex()		const { return index;	}
		const ObjectType GetType()	const { return Type;	}

	private:
		// this object in objectSystem postion
		size_t id;
		size_t index;
		ObjectType Type = ObjectType::Unknown;
	};

	/*
	* The base class for MagicHeaderTool services. Any class that wants to use advanced C features should inherit from this class.
	*/
	class CORE_API Object
	{
	public:
		virtual ~Object() = default;
		Object() : instance_id(GetNextId()) {};

		virtual std::string GetClassName() const = 0;				// Class Name generated using MagicHeaderTool
		virtual uint64_t GetClassId() const = 0;					// Class ID generated using MagicHeaderTool
		virtual uint64_t GetClassHas() const = 0;					// Class Has generated using MagicHeaderTool
		virtual ObjectType GetClassType() const = 0;				// Class type generated using MagicHeaderTool
		virtual ObjectSwitch GetClassSwitch() const = 0;			// Class Switch generated using MagicHeaderTool
		uint64_t GetInstanceId() const { return instance_id; };		// Assign a unique ID to each object instance

		// Serialization handles object data only; callers own the storage medium and location.
		BinaryReader Serialize();
		void Deserialize(const BinaryWriter& writer);

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

		virtual void SerializeData(std::ostringstream& stream) = 0;
		virtual void DeserializeData(std::stringstream& stream) = 0;

		uint64_t	instance_id;						// The id of each instance
		ObjectSystemHandle Handle = {};					// Object Handle
	};

	class CORE_API IObjectSystem {
	public:
		virtual size_t GetObjectNum() = 0;								// Get Current ObjectNum
		virtual ObjectSystemHandle RegisterObject(Object* ptr) = 0;		// Register This Object For ObjectSystem Return Handle
		virtual void RemoveObject(ObjectSystemHandle Handle) = 0;		// Delete This Object Input this Handle

	};

	CORE_API IObjectSystem* GetObjectSystem();
}

// We use a nickname to make it more recognizable
using MObject = Core::Object;

DISABLE_DLL_WARNINGS_POP;