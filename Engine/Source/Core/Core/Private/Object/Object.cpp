#include "Object.h"
#include "CoreMinimal.h"
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

	std::vector<uint8_t> Object::Serialize()
	{
		if (!EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
			throw std::logic_error("Object does not support serialization: " + GetClassName());
		}

		std::ostringstream oss;
		SerializeData(oss);

		const std::string& str = oss.str();
		return std::vector<uint8_t>(str.begin(), str.end());
	}

	void Object::Deserialize(const std::vector<uint8_t>& data)
	{
		if (!EnumHasAnyFlags(GetClassSwitch(), ObjectSwitch::Serialization)) {
			throw std::logic_error("Object does not support serialization: " + GetClassName());
		}

		if (data.empty()) {
			throw std::invalid_argument("Cannot deserialize an empty buffer: " + GetClassName());
		}

		std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);
		ss.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
		ss.seekg(0, std::ios::beg);
		DeserializeData(ss);
	}
	// -------------------------------- 
}