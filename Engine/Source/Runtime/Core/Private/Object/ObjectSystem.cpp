#include "Object.h"
#include "ObjectSystem.h"
#include "IO.h"

namespace Core {
	AUTO_REGISTER(ObjectSystem);

	IObjectSystem* GetObjectSystem() {
		return &ObjectSystem::Get();
	}

	bool ObjectSystem::Init()
	{
		Objects.reserve(1024);		// Init Object Pool Size is 1024
		return true;
	}

	void ObjectSystem::Uninstall()
	{
	}

	size_t ObjectSystem::GetObjectNum() {
		return Objects.size();
	}

	size_t ObjectSystem::RegisterObject(Object* ptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		Objects.push_back(ptr);
		return Objects.size() - 1; // return current index
	}

	void ObjectSystem::RemoveObject(size_t index)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (Objects.empty() || index >= Objects.size()) {
			return;  // Invalid index
		}
		if (index < Objects.size() - 1) {
			std::swap(Objects[index], Objects.back());  // Swap to the tail
		}
		Objects.pop_back();  // delete tail
	}
}