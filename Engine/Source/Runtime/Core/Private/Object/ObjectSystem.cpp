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

		// Create the corresponding folder if it doesn't exist
		std::wstring EngineSerializedDataDir = IO::AbsolutePath().Get().GetExecutableDirectory() + L"\\" CACHE L"SerializedData\\";
		this->EngineSerializedDataDir = EngineSerializedDataDir;
		std::wstring ProjectSerializedDataDir = IO::AbsolutePath().Get().GetCurrentWorkingDirectory() + L"\\" CACHE L"SerializedData\\";
		this->ProjectSerializedDataDir = ProjectSerializedDataDir;

		IO::MakeDirectory(this->EngineSerializedDataDir);
		IO::MakeDirectory(this->ProjectSerializedDataDir);

		return true;
	}

	void ObjectSystem::Uninstall()
	{
	}

	size_t ObjectSystem::GetObjectNum() {
		return Objects.size();
	}

	ObjectSystemHandle ObjectSystem::RegisterObject(Object* ptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		Objects.push_back({ ptr,ptr->GetClassType() });
		return ObjectSystemHandle(Objects.size() - 1); // return current index for Object, use Handle Wrapper
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

	ObjectSerializationDescriptor ObjectSystem::GetObjectSerializationData(ObjectSystemHandle Handle)
	{
		return ObjectSerializationDescriptor();
	}

	void ObjectSystem::SaveObjectSerializationData(ObjectSerializationData Descriptor)
	{

	}
}