#include "Object.h"
#include "ObjectSystem.h"
#include "CoreLogCapture/CoreLogCapture.h"
#include "IO.h"
#include "Common/Check.h"

namespace Core {
	AUTO_REGISTER(ObjectSystem);

	// ------------------------------------------------------------- ObjectSystem --------------------------------------------------------- //
	IObjectSystem* GetObjectSystem() {
		return &ObjectSystem::Get();
	}

	bool ObjectSystem::Init()
	{
		Objects.reserve(PRE_ALLOCATED_SIZE);		// Init Object Pool Size

		if (!Deserialization()) {
			// Deserialization Error
			__debugbreak();
			return false;
		}

		return true;
	}

	void ObjectSystem::Uninstall()
	{
		Read_FileBytes.clear();				// delete read FileBytes

		if (!Serialization()) {
			// Serialization Error
			__debugbreak();
			return;
		}
	}

	size_t ObjectSystem::GetObjectNum() {
		return Objects.size();
	}

	ObjectSystemHandle ObjectSystem::RegisterObject(Object* ptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		ObjectSystemHandle Hanedle{ ptr->GetInstanceId(), Objects.size(), ptr->GetClassType()};

		Objects.push_back({ ptr,Hanedle });
		return Hanedle; // return current index for Object, use Handle Wrapper
	}

	void ObjectSystem::RemoveObject(ObjectSystemHandle Handle)
	{
		size_t index = Handle.GetIndex();
		std::lock_guard<std::mutex> lock(m_mutex);
		if (Objects.empty() || index >= Objects.size()) {
			return;  // Invalid index
		}
		if (index < Objects.size() - 1) {
			std::swap(Objects[index], Objects.back());  // Swap to the tail
		}
		Objects.pop_back();  // delete tail
	}

	// ------------------------------------------------------------- ObjectSystem --------------------------------------------------------- //
}