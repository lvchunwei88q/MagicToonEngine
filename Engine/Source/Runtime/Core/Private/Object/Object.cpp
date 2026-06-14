#include "Object.h"
#include "CoreLogCapture/CoreLogCapture.h"
//////////////////// Serialization library
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

namespace Core {

	void Object::ObjectInit() {
		try {
			Handle = GetObjectSystem()->RegisterObject(this); // Register This Object

			std::vector<uint8_t> binaryData = { 1,2,3 };

			std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);
			ss.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
			ss.seekg(0, std::ios::beg);

			cereal::BinaryInputArchive archive(ss);
			archive(*this);
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object Constructor: " + std::string(e.what()));
		}
	}
	void Object::ObjectUninit() {
		try {
			std::vector<uint8_t> BinaryData;
			std::ostringstream oss;
			cereal::BinaryOutputArchive archive(oss);
			archive(*this);

			const std::string& str = oss.str();
			BinaryData.assign(str.begin(), str.end());

			GetObjectSystem()->RemoveObject(Handle); // delete this object for objectSystem
		}
		catch (const std::exception& e) {
			ErrorCapture::Capture("Object Constructor: " + std::string(e.what()));
		}
	}
}