#include "Object.h"
#include "ObjectSystem.h"
#include "CoreMinimal.h"

namespace Core {
	AUTO_REGISTER(ObjectSystem);

	// ------------------------------------------------------------- ObjectSystem --------------------------------------------------------- //
	IObjectSystem* GetObjectSystem() {
		return &ObjectSystem::Get();
	}

	bool ObjectSystem::Init()
	{
		Objects.reserve(PRE_ALLOCATED_SIZE);		// Init Object Pool Size
		return true;
	}

	void ObjectSystem::Uninstall()
	{
	}

	size_t ObjectSystem::GetObjectNum() {
		return Objects.size();
	}

	ObjectHandle ObjectSystem::RegisterObject(Object* ptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		ObjectHandle Hanedle{ ptr->GetInstanceId(), Objects.size(), ptr->GetClassType()};

		Objects.push_back({ ptr,Hanedle });
		return Hanedle; // return current index for Object, use Handle Wrapper
	}

	void ObjectSystem::RemoveObject(ObjectHandle Handle)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		size_t index = Handle.GetIndex();
		if (index >= Objects.size()) {
			return;  // Invalid index
		}
		// Identity verification: a mismatch constitutes a dirty handle
		if (Objects[index].Handle != Handle) return;

		if (index < Objects.size() - 1) {
			std::swap(Objects[index], Objects.back());  // Swap to the tail
			Objects[index].Handle.UpdateIndex(index);
		}
		Objects.pop_back();  // delete tail
	}

	// ------------------------------------------------------------- ObjectSystem --------------------------------------------------------- //
}