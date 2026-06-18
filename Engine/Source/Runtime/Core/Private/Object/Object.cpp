#include "Object.h"
#include "CoreLogCapture/CoreLogCapture.h"
//////////////////// Serialization library
#include <cereal/cereal.hpp>

namespace Core {
	void Object::ObjectInit() {
		try {
			Handle = GetObjectSystem()->RegisterObject(this); // Register This Object

			if (EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
				Deserialization();
			}
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object Constructor: " + std::string(e.what()));
		}
	}

	void Object::ObjectUninit() {
		try {
			if (EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization) && Mode == DestructionMode::SaveAndDestroy) {
				Serialization();
			}

			GetObjectSystem()->RemoveObject(Handle); // delete this object for objectSystem
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object Constructor: " + std::string(e.what()));
		}
	}

	// -------------------------------- serialization
	void Object::Serialization()
	{
		std::vector<uint8_t> BinaryData;
		std::ostringstream oss;
		// Use the subclass's serialization function
		serialized_data_generation(oss);

		const std::string& str = oss.str();
		BinaryData.assign(str.begin(), str.end());

		ObjectSerializationData Data = { std::move(BinaryData), Handle };
		GetObjectSystem()->SaveObjectSerializationData(Data);
	}

	void Object::Deserialization()
	{
		ObjectSerializationDescriptor ObjectData = GetObjectSystem()->GetObjectSerializationData(Handle);	// Get ObjectData

		if (ObjectData.Length <= 0) return; // No data
		std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);
		ss.write(reinterpret_cast<const char*>(ObjectData.DataStart), ObjectData.Length);
		ss.seekg(0, std::ios::beg);

		// Use the subclass's serialization function
		deserialization_data_generation(ss);
	}
	// -------------------------------- 
}