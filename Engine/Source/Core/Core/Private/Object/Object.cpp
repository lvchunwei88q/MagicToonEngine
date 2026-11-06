#include "Object.h"
#include "CoreMinimal.h"
//////////////////// Serialization library
#include <cereal/cereal.hpp>
#include <stdexcept>

namespace Core {
	namespace {
	}

	void Object::ObjectInit() {
		try {
			Handle = GetObjectSystem()->RegisterObject(this); // Register This Object
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object - " + GetClassName() + " Init: " + std::string(e.what()));
		}
	}

	void Object::ObjectUninit() {
		try {
			GetObjectSystem()->RemoveObject(Handle); // delete this object for objectSystem
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object - " + GetClassName() + " Uninit: " + std::string(e.what()));
		}
	}

	void Object::Save()
	{
		if (!EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
			throw std::logic_error("Object does not support serialization: " + GetClassName());
		}

		const std::wstring path = GetObjectCustomSerializedPath();
		if (path.empty()) {
			throw std::logic_error("Object serialization path is empty: " + GetClassName());
		}

		std::ostringstream oss;
		serialized_data_generation(oss);

		const std::string& str = oss.str();
		ObjectCustomSerializationData data;
		data.handle = Handle;
		data.data.assign(str.begin(), str.end());
		data.ObjectSerializationPath = path;
		GetObjectSystem()->SaveCustomObjectSerializationData(std::move(data));
	}

	bool Object::Load()
	{
		if (!EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
			throw std::logic_error("Object does not support serialization: " + GetClassName());
		}

		const std::wstring path = GetObjectCustomSerializedPath();
		if (path.empty()) {
			throw std::logic_error("Object serialization path is empty: " + GetClassName());
		}

		std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);

		ObjectCustomSerializationDescriptor customData =
			GetObjectSystem()->GetCustomObjectSerializationData(path);
		if (!customData.data.empty()) {
			ss.write(reinterpret_cast<const char*>(customData.data.data()), customData.data.size());
		}
		else {
			// Read legacy ObjectSystem-managed data once so existing settings survive the path migration.
			ObjectSerializationDescriptor legacyData =
				GetObjectSystem()->GetObjectSerializationData(Handle);
			if (legacyData.Length == 0) {
				return false;
			}
			ss.write(reinterpret_cast<const char*>(legacyData.DataStart), legacyData.Length);
		}

		ss.seekg(0, std::ios::beg);
		deserialization_data_generation(ss);
		return true;
	}
	// -------------------------------- 
}