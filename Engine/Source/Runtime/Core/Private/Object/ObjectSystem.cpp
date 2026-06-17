#include "Object.h"
#include "ObjectSystem.h"
#include "IO.h"

namespace Core {
	AUTO_REGISTER(ObjectSystem);

	BinaryBuffer ObjectByteFile::GenerateFileData()
	{
		// File layout -> [ index , data ]
		size_t CurrentFileObjectSize = sizeof(size_t);
		for (size_t i = 0; i < data.size(); i++)
		{
			CurrentFileObjectSize += data[i].GetAllSize();
		}

		BinaryBuffer FileData(CurrentFileObjectSize);

		// Write the first one
		FileData.Write(GetIndex());

		for (size_t i = 0; i < data.size(); i++)
		{
			ObjectByte& Byte = data[i];
			std::vector<uint8_t> Handle;
			Handle.resize(sizeof(ObjectSystemHandle));
			memcpy(Handle.data(), &Byte.handle, sizeof(ObjectSystemHandle));

			// Byte layout -> [ size , handle , data ]
			FileData.Write(Byte.data.size());
			FileData.Write(Handle);
			FileData.Write(Byte.data);
		}

		return FileData;
	}

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
		ObjectSystemHandle Hanedle{ Objects.size(),ptr->GetClassType() };
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

	ObjectSerializationDescriptor ObjectSystem::GetObjectSerializationData(ObjectSystemHandle Handle)
	{
		if (Handle.GetType() == ObjectType::Unknown || Handle.GetIndex() > Objects.size())
		{
			return ObjectSerializationDescriptor();
		}

		ObjectRef Ptr = Objects[Handle.GetIndex()].Object;
		size_t ClassHas = Ptr->GetClassHas();

		for (size_t i = 0; i < Read_FileBytes.size(); i++)
		{
			ObjectByteFile& FileByte = Read_FileBytes[i];
			if (Handle.GetType() == FileByte.GetStorageArea() && ClassHas == FileByte.FileHas) {
				for (size_t y = 0; y < FileByte.data.size(); y++)
				{
					ObjectByte& Byte = FileByte.data[y];
					if (Byte.handle == Handle) {
						ObjectSerializationDescriptor ObjectData;
						ObjectData.DataStart = Byte.data.data();
						ObjectData.Length = Byte.data.size();
						return ObjectData;
					}
				}
			}
		}
		return ObjectSerializationDescriptor();
	}

	void ObjectSystem::SaveObjectSerializationData(ObjectSerializationData Descriptor)
	{
		// 实现文件存储
	}

	bool ObjectSystem::Serialization()
	{
		for (size_t i = 0; i < Written_FileBytes.size(); i++)
		{
			ObjectByteFile& FileByte = Written_FileBytes[i];
			const BinaryBuffer& Buffer = FileByte.GenerateFileData();

			ObjectType CurrentStorageArea = FileByte.GetStorageArea();
			std::wstring CurrentFileSavePath = CurrentStorageArea == ObjectType::ENGINE ? Data_.EngineSerializedDataDir :
				CurrentStorageArea == ObjectType::PROJECT ? Data_.ProjectSerializedDataDir : L"ERROR";

			CurrentFileSavePath += FileByte.GetFileName() + L".ObjectByte";
			if (!IO::Exists(CurrentFileSavePath)) {
				IO::MakeFile(CurrentFileSavePath);
			}

			IO::WriteAllBytes(CurrentFileSavePath, Buffer);
		}
		return true;
	}

	bool ObjectSystem::Deserialization()
	{
		// Create the corresponding folder if it doesn't exist
		std::wstring EngineSerializedDataDir = IO::AbsolutePath().Get().GetExecutableDirectory() + L"\\" CACHE L"SerializedData\\";
		Data_.EngineSerializedDataDir = EngineSerializedDataDir;
		std::wstring ProjectSerializedDataDir = IO::AbsolutePath().Get().GetCurrentWorkingDirectory() + L"\\" CACHE L"SerializedData\\";
		Data_.ProjectSerializedDataDir = ProjectSerializedDataDir;

		if (!IO::Exists(Data_.EngineSerializedDataDir)) {
			IO::MakeDirectory(Data_.EngineSerializedDataDir);
		}
		if (!IO::Exists(Data_.ProjectSerializedDataDir)) {
			IO::MakeDirectory(Data_.ProjectSerializedDataDir);
		}

		std::vector<std::string> EngineSerializedFiles = IO::GetFilesInDirectory(EngineSerializedDataDir);
		std::vector<std::string> ProjectSerializedFiles = IO::GetFilesInDirectory(ProjectSerializedDataDir);

		// TODO 实现文件读取

		return true;
	}
}