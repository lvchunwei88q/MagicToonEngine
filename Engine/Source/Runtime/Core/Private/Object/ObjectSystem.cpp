#include "Object.h"
#include "ObjectSystem.h"
#include "IO.h"
#include "Tools/Check.h"

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

	ObjectSerializationDescriptor ObjectSystem::GetObjectSerializationData(ObjectSystemHandle Handle)
	{
		ObjectSerializationDescriptor ObjectData;
		if (Handle.GetType() == ObjectType::Unknown || Handle.GetIndex() > Objects.size())
		{
			return ObjectSerializationDescriptor();
		}

		FindObjectByteFile_Const(Read_FileBytes, Handle,
			[&](const ObjectByteFile& FileByte, size_t ClassHas, const ObjectSystemHandle&) -> bool {
			for (size_t y = 0; y < FileByte.data.size(); y++)
			{
				const ObjectByte& Byte = FileByte.data[y];
				if (Byte.handle == Handle) {
					ObjectSerializationDescriptor ObjectData;
					ObjectData.DataStart = Byte.data.data();
					ObjectData.Length = Byte.data.size();
					return true;
				}
			}
			return false;
		});
		return ObjectData;
	}

	void ObjectSystem::SaveObjectSerializationData(ObjectSerializationData Descriptor)
	{
		ObjectSystemHandle& Handle = Descriptor.handle;
		if (Handle.GetType() == ObjectType::Unknown || Handle.GetIndex() > Objects.size())
		{
			return;
		}

		bool IsFind = FindObjectByteFile(Written_FileBytes, Handle, 
			[&](ObjectByteFile& FileByte, size_t ClassHas, ObjectSystemHandle& Handle) -> bool {
				// If we find it, then we'll add it here
#ifndef _DEBUG
				for (size_t i = 0; i < FileByte.data.size(); i++)
				{
					const auto& FileByte_data = FileByte.data[i];
					if (FileByte_data.handle == Handle) {
						ThrowErrorMessage("Error: ObjectSystem already has the same Object");
					}
				}
#endif
				ObjectByte ByteFrag;
				ByteFrag.handle = Handle;
				ByteFrag.data = Descriptor.data;

				FileByte.data.push_back(ByteFrag);
				return true;
		});

		if (!IsFind) {
			// It says there's no such area or the file for this area, so now we need to create this file.
			ObjectRef Ptr = Objects[Handle.GetIndex()].Object;
			size_t ClassHas = Ptr->GetClassHas();

			ObjectByte ByteFrag;
			ByteFrag.handle = Handle;
			ByteFrag.data = Descriptor.data;

			ObjectByteFile ByteFile;
			ByteFile.FileHas = ClassHas;
			ByteFile.Type = Handle.GetType();
			ByteFile.data.push_back(ByteFrag);

			Written_FileBytes.push_back(ByteFile);
		}
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

#define FIND_OBJECT_BYTE_FILE_FUNCTION(...)															\
	ObjectRef Ptr = Objects[Handle.GetIndex()].Object;												\
	size_t ClassHas = Ptr->GetClassHas();															\
	for (size_t i = 0; i < FileBytes.size(); i++)													\
	{																								\
		__VA_ARGS__ ObjectByteFile& FileByte = FileBytes[i];										\
		if (Handle.GetType() == FileByte.GetStorageArea() && ClassHas == FileByte.FileHas) {		\
			if (Func) return Func(FileByte, ClassHas, Handle);										\
			return true;																			\
		}																							\
	}

	bool ObjectSystem::FindObjectByteFile(std::vector<ObjectByteFile>& FileBytes,ObjectSystemHandle& Handle, FindObjectLambda Func)
	{
		FIND_OBJECT_BYTE_FILE_FUNCTION();
		return false;
	}

	bool ObjectSystem::FindObjectByteFile_Const(const std::vector<ObjectByteFile>& FileBytes, const ObjectSystemHandle& Handle, FindObjectLambda_Const Func)
	{
		FIND_OBJECT_BYTE_FILE_FUNCTION(const);
		return false;
	}
}