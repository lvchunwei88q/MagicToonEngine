#pragma once
#include "Subsystem/SubsystemTemplate.h"
#include "Object.h"
#include "Tools/Buffer.h"
#include <vector>
#include <string>
#include <mutex>

#define PRE_ALLOCATED_SIZE 1024

namespace Core {
	struct ObjectByte{
		std::vector<uint8_t> data;
		ObjectSystemHandle handle;

		// Byte layout -> [ size , handle , data ]
		size_t GetAllSize() const { return sizeof(size_t) + sizeof(ObjectSystemHandle) + data.size(); }
	};

	struct ObjectByteFile {
		std::vector<ObjectByte> data;	// Data, and the handle corresponding to each object
		uint64_t FileHas;				// The file corresponds to Has (the Has that matches the Object will be stored using this file) and we usually use it as the file name
		ObjectType Type;				// Here we use this as a mark for the Engine or the Project

		size_t GetIndex() const { return data.size(); }
		std::wstring GetFileName() const { return std::to_wstring(FileHas); };
		ObjectType GetStorageArea() const { return Type; };
		BinaryWrite GenerateFileData();
	};

	/*				 size = x
	* |-----------------------------------|
	* |				file				  |
	* |-----------------------------------|
	*				  |
	*	 size = 8	 \|/     size = x
	* |-----------------------------------|
	* |	Index |  data					  |
	* |-----------------------------------|
	*				  |
	*	 size = 8	 \|/     size = x
	* |-----------------------------------|
	* |ObjectByte | data				  |
	* |-----------------------------------|
	*/

	struct ObjectContainer {
		ObjectRef Object;
		ObjectSystemHandle Handle;
	};

	struct ObjectSystemTempData : public Singleton<ObjectSystemTempData>
	{
		std::wstring EngineSerializedDataDir;
		std::wstring ProjectSerializedDataDir;
	};

	// 对象系统，负责管理所有的对象实例
	class ObjectSystem : public SubsystemTemplate<ObjectSystem,Priority::Core> , public IObjectSystem
	{
	public:
		virtual bool Init() override;
		virtual void Uninstall() override;

		virtual size_t GetObjectNum() override;
		virtual ObjectSystemHandle RegisterObject(Object* ptr) override;
		virtual void RemoveObject(ObjectSystemHandle Handle) override;
		// ---------------------------------------------------------------------------------- MSERIALIZATION
		virtual ObjectSerializationDescriptor GetObjectSerializationData(ObjectSystemHandle Handle) override;
		virtual void SaveObjectSerializationData(ObjectSerializationData Descriptor) override;
		// ---------------------------------------------------------------------------------- MSERIALIZATION END

	private:
		// functions
		using FindObjectLambda_Const	= std::function<bool(const ObjectByteFile&, size_t, const ObjectSystemHandle&)>;
		using FindObjectLambda			= std::function<bool(ObjectByteFile&, size_t, ObjectSystemHandle&)>;
		bool FindObjectByteFile(std::vector<ObjectByteFile>& FileBytes, ObjectSystemHandle& Handle, FindObjectLambda Func = nullptr);
		bool FindObjectByteFile_Const(const std::vector<ObjectByteFile>& FileBytes, const ObjectSystemHandle& Handle, FindObjectLambda_Const Func = nullptr);
	protected:
		// Here we implement the features we need
		// -------------------------------- serialization
		bool Serialization();
		bool Deserialization();
		// -------------------------------- 
	private:
		// There is absolutely no management of their deletions here.
		std::vector<ObjectContainer> Objects;
		// We use two separate structures for initialization and storage
		std::vector<ObjectByteFile> Read_FileBytes;
		std::vector<ObjectByteFile> Written_FileBytes;

		mutable std::mutex m_mutex;

		ObjectSystemTempData& Data_ = ObjectSystemTempData::Get();
	};
}