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
		// Use the subclass's serialization function
		serialized_data_generation(oss);

		const std::string& str = oss.str();
		BinaryData.assign(str.begin(), str.end());

		ObjectSerializationData Data = { std::move(BinaryData), Handle };
		switch (Mode)
		{
			case DestructionMode::SaveAndDestroy:
				GetObjectSystem()->SaveObjectSerializationData(Data);
				break;
			case DestructionMode::CustomSaveAndDestroy: {
				ObjectCustomSerializationData SerializationData;
				SerializationData.handle = Data.handle;
				SerializationData.data = Data.data;
				SerializationData.ObjectSerializationPath = L""; // TODO 实现
				GetObjectSystem()->SaveCustomObjectSerializationData(SerializationData);
			}break;
		}
	}

	void Object::Deserialization()
	{
		std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);
		switch (Mode)
		{
			case DestructionMode::SaveAndDestroy: {
				ObjectSerializationDescriptor Descriptor = GetObjectSystem()->GetObjectSerializationData(Handle);	// Get ObjectData -> Descriptor
				if (Descriptor.Length <= 0) return; // No data
				ss.write(reinterpret_cast<const char*>(Descriptor.DataStart), Descriptor.Length);
			}break;
			case DestructionMode::CustomSaveAndDestroy: {
				ObjectCustomSerializationDescriptor Descriptor = GetObjectSystem()->GetCustomObjectSerializationData(L"");// TODO 实现
				if (Descriptor.data.size() <= 0) return; // No data
				ss.write(reinterpret_cast<const char*>(Descriptor.data.data()), Descriptor.data.size());
			}break;
			default: return;
		}

		ss.seekg(0, std::ios::beg);

		// Use the subclass's serialization function
		deserialization_data_generation(ss);
	}
	// -------------------------------- 
}