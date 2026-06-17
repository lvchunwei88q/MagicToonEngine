#include "Object.h"
#include "CoreLogCapture/CoreLogCapture.h"
//////////////////// Serialization library
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

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
			if (EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
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
		cereal::BinaryOutputArchive archive(oss);
		archive(*this);

		const std::string& str = oss.str();
		BinaryData.assign(str.begin(), str.end());

		ObjectSerializationData Data = { std::move(BinaryData) };
		GetObjectSystem()->SaveObjectSerializationData(Data);
	}

	void Object::Deserialization()
	{
		ObjectSerializationDescriptor ObjectData = GetObjectSystem()->GetObjectSerializationData(Handle);	// Get ObjectData

		std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);
		ss.write(reinterpret_cast<const char*>(ObjectData.DataStart), ObjectData.Length);
		ss.seekg(0, std::ios::beg);

		cereal::BinaryInputArchive archive(ss);
		archive(*this);
	}
	// -------------------------------- 
}