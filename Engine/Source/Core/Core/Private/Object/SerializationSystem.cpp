#include "Object.h"
#include "ObjectSystem.h"
#include "CoreMinimal.h"
#include "IO.h"
#include "Common/Check.h"

namespace Core {
	BinaryWrite ObjectByteFile::GenerateFileData()
	{
		// File layout -> [ index , data ]
		size_t CurrentFileObjectSize = sizeof(size_t);
		for (size_t i = 0; i < data.size(); i++)
		{
			CurrentFileObjectSize += data[i].GetAllSize();
		}

		BinaryWrite FileData(CurrentFileObjectSize);

		// Write the first one
		FileData.Write(GetIndex());

		for (size_t i = 0; i < data.size(); i++)
		{
			ObjectByte& Byte = data[i];

			// Byte layout -> [ size , handle , data ]
			FileData.Write(Byte.data.size());
			FileData.Write(Byte.handle);
			FileData.Write(Byte.data);
		}

		return FileData;
	}

	// ------------------------------------------------------------------- Object System ------- //
	ObjectSerializationDescriptor ObjectSystem::GetObjectSerializationData(ObjectSystemHandle Handle)
	{
		ObjectSerializationDescriptor ObjectData;
		if (Handle.GetType() == ObjectType::Unknown || Handle.GetIndex() >= Objects.size())
		{
			return ObjectSerializationDescriptor();
		}

		FindObjectByteFile_Const(Read_FileBytes, Handle,
			[&](const ObjectByteFile& FileByte, size_t ClassHas, const ObjectSystemHandle&) -> bool {
				for (size_t y = 0; y < FileByte.data.size(); y++)
				{
					const ObjectByte& Byte = FileByte.data[y];
					if (Byte.handle == Handle) {
						ObjectData.DataStart = Byte.data.data();
						ObjectData.Length = Byte.data.size();
						return true;
					}
				}
				return false;
			});
		return ObjectData;
	}

	void ObjectSystem::SaveObjectSerializationData(ObjectSerializationData ObjectData)
	{
		ObjectSystemHandle& Handle = ObjectData.handle;
		if (Handle.GetType() == ObjectType::Unknown || Handle.GetIndex() >= Objects.size())
		{
			return;
		}

		bool IsFind = FindObjectByteFile(Written_FileBytes, Handle,
			[&](ObjectByteFile& FileByte, size_t ClassHas, ObjectSystemHandle& Handle) -> bool {
				// If we find it, then we'll add it here
#ifdef _DEBUG
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
				ByteFrag.data = ObjectData.data;

				FileByte.data.push_back(ByteFrag);
				return true;
			});

		if (!IsFind) {
			// It says there's no such area or the file for this area, so now we need to create this file.
			ObjectRef Ptr = Objects[Handle.GetIndex()].Object;
			size_t ClassHas = Ptr->GetClassHas();

			ObjectByte ByteFrag;
			ByteFrag.handle = Handle;
			ByteFrag.data = ObjectData.data;

			ObjectByteFile ByteFile;
			ByteFile.FileHas = ClassHas;
			ByteFile.Type = Handle.GetType();
			ByteFile.data.push_back(ByteFrag);

			Written_FileBytes.push_back(ByteFile);
		}
	}

	bool ObjectSystem::SaveSerializationToByte()
	{
		for (size_t i = 0; i < Written_FileBytes.size(); i++)
		{
			ObjectByteFile& FileByte = Written_FileBytes[i];
			const BinaryWrite& Buffer = FileByte.GenerateFileData();

			ObjectType CurrentStorageArea = FileByte.GetStorageArea();
			std::wstring CurrentFileSavePath = CurrentStorageArea == ObjectType::ENGINE ? Data_.EngineSerializedDataDir :
				CurrentStorageArea == ObjectType::PROJECT ? Data_.ProjectSerializedDataDir : L"ERROR";

			CurrentFileSavePath += FileByte.GetFileName() + L".ObjectByte";
			if (!IO::Exists(CurrentFileSavePath)) {
				IO::CreateFile(CurrentFileSavePath);
			}

			IO::WriteAllBytes(CurrentFileSavePath, Buffer);
		}
		return true;
	}

	bool ObjectSystem::ReadDeserializationToByte()
	{
		// Create the corresponding folder if it doesn't exist
		std::wstring EngineSerializedDataDir = IO::AbsolutePath().Get().GetExecutableDirectory() + L"\\" CACHE L"SerializedData\\";
		Data_.EngineSerializedDataDir = EngineSerializedDataDir;
		std::wstring ProjectSerializedDataDir = IO::AbsolutePath().Get().GetCurrentWorkingDirectory().GetCache() + L"SerializedData\\";
		Data_.ProjectSerializedDataDir = ProjectSerializedDataDir;

		if (!IO::Exists(Data_.EngineSerializedDataDir)) {
			IO::CreateDirectory(Data_.EngineSerializedDataDir);
		}
		if (!IO::Exists(Data_.ProjectSerializedDataDir)) {
			IO::CreateDirectory(Data_.ProjectSerializedDataDir);
		}

		// Get File List
		std::vector<std::string> EngineSerializedFiles = IO::GetFilesInDirectory(EngineSerializedDataDir);
		std::vector<std::string> ProjectSerializedFiles = IO::GetFilesInDirectory(ProjectSerializedDataDir);

		// Reusable Lambda Function
		auto ReadFileToObjectByteFile = [&](const std::vector<std::string>& files, std::vector<ObjectByteFile>& OutByteFile, ObjectType Type) {
			for (size_t i = 0; i < files.size(); i++)
			{
				auto& file = files[i];
				std::wstring fileW = IO::ToWideString(file);
				std::wstring FileName;
				size_t FileHas;
				IO::GetFileName(fileW, FileName);
				try {
					FileHas = static_cast<size_t>(std::stoull(FileName));
				}
				catch (const std::exception&) {
					WarningCapture::Capture("Files not handled by ObjectSystem have been skipped!");
					continue;
				}

				// Read File Name
				std::wstring pathW;
				switch (Type)
				{
				case ObjectType::ENGINE:	pathW = EngineSerializedDataDir;	break;
				case ObjectType::PROJECT:	pathW = ProjectSerializedDataDir;	break;
				}

				pathW += fileW;
				std::vector<uint8_t> Content = IO::ReadAllU8Bytes(pathW);
				BinaryReader ObjectByteBuffer(std::move(Content));
				size_t Index = ObjectByteBuffer.Read<size_t>();

				ObjectByteFile ObjectByte;
				ObjectByte.Type = Type;
				ObjectByte.FileHas = FileHas;
				ObjectByte.data.resize(Index);

				for (size_t y = 0; y < Index; y++)
				{
					size_t Size = ObjectByteBuffer.Read<size_t>();
					ObjectSystemHandle Handle = ObjectByteBuffer.Read<ObjectSystemHandle>();
					std::vector<uint8_t> Data = ObjectByteBuffer.ReadToVector(Size);

					ObjectByte.data[y].data = Data;
					ObjectByte.data[y].handle = Handle;
				}

				OutByteFile.push_back(ObjectByte);
			}
			};

		ReadFileToObjectByteFile(EngineSerializedFiles, Read_FileBytes, ObjectType::ENGINE);
		ReadFileToObjectByteFile(ProjectSerializedFiles, Read_FileBytes, ObjectType::PROJECT);

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

	bool ObjectSystem::FindObjectByteFile(std::vector<ObjectByteFile>& FileBytes, ObjectSystemHandle& Handle, FindObjectLambda Func)
	{
		FIND_OBJECT_BYTE_FILE_FUNCTION();
		return false;
	}

	bool ObjectSystem::FindObjectByteFile_Const(const std::vector<ObjectByteFile>& FileBytes, const ObjectSystemHandle& Handle, FindObjectLambda_Const Func)
	{
		FIND_OBJECT_BYTE_FILE_FUNCTION(const);
		return false;
	}

	/////////////////////////// Simple Serialization

	ObjectCustomSerializationDescriptor ObjectSystem::GetCustomObjectSerializationData(const std::wstring& Path)
	{
		if (IO::Exists(Path)) {
			ObjectCustomSerializationDescriptor ObjectDescriptor;
			std::vector<uint8_t> data = IO::ReadAllU8Bytes(Path);
			BinaryReader Data(data);
			ObjectDescriptor.handle = Data.Read<ObjectSystemHandle>();
			Data.ReadToVector(ObjectDescriptor.data, Data.Remaining());
			return ObjectDescriptor;
		}
		return ObjectCustomSerializationDescriptor();
	}

	void ObjectSystem::SaveCustomObjectSerializationData(ObjectCustomSerializationData ObjectData)
	{
		if (!IO::Exists(ObjectData.ObjectSerializationPath)) {
			IO::CreateFile(ObjectData.ObjectSerializationPath);
		}

		BinaryWrite Data;
		Data.Write(ObjectData.handle);
		Data.Write(ObjectData.data);

		IO::WriteAllBytes(ObjectData.ObjectSerializationPath, Data);
	}

	// ------------------------------------------------------------------- Object System ------- //
}