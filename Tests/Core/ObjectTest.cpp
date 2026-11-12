#include "../Common/TestFramework.h"

#include <Object.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ObjectTest.object.generate.h>

MCLASS(MSERIALIZATION)
class SerializableTestObject final : public MObject
{
	GENERATE_BODY(SerializableTestObject, Unknown);
public:
	MMEMBER();
	int32_t integer = 0;
	MMEMBER();
	float decimal = 0.0f;
	MMEMBER();
	std::string text;
	MMEMBER();
	std::vector<int32_t> scores;
	MMEMBER();
	std::vector<std::string> tags;
	MMEMBER();
	std::vector<std::vector<int32_t>> inventory;
	MMEMBER();
	Vec3<float> position;
};

MCLASS(MSERIALIZATION)
class NonSerializableTestObject final : public MObject
{
	GENERATE_BODY(NonSerializableTestObject, Unknown);
};

namespace {

std::filesystem::path GetObjectTestDataPath()
{
#ifdef _WIN32
	std::array<wchar_t, 32768> executablePath = {};
	const DWORD pathLength = GetModuleFileNameW(
		nullptr,
		executablePath.data(),
		static_cast<DWORD>(executablePath.size()));
	if (pathLength == 0 || pathLength >= executablePath.size())
	{
		throw std::runtime_error("Failed to determine the ObjectTest executable path");
	}

	std::filesystem::path fileName("ObjectTest.cpp");
	fileName.replace_extension(".mdate");
	return std::filesystem::path(executablePath.data()).parent_path()
		/ "data"
		/ fileName;
#else
	throw std::runtime_error("ObjectTest data path is supported on Windows only");
#endif
}

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Failed to open serialized ObjectTest data");
	}

	const std::vector<char> fileContents{
		std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
	if (file.bad())
	{
		throw std::runtime_error("Failed to read serialized ObjectTest data");
	}

	std::vector<uint8_t> bytes;
	bytes.reserve(fileContents.size());
	std::transform(fileContents.begin(), fileContents.end(), std::back_inserter(bytes),
		[](char value) {
			return static_cast<uint8_t>(static_cast<unsigned char>(value));
		});
	return bytes;
}

}

void ObjectSerializationRoundTrip()
{
	auto source = ObjectFactory::CreateUnique<SerializableTestObject>();
	source->integer = 42;
	source->decimal = 3.25f;
	source->text = "Object serialization test: nested data survives disk round-trip";
	source->scores = { 17, 93, -4, 1200 };
	source->tags = { "player", "serialization", "UTF-8-ready" };
	source->inventory = {
		{ 101, 102, 103 },
		{},
		{ -7, 0, 2048, 65535 }
	};
	source->position = Vec3<float>(12.5f, -3.25f, 900.125f);

	BinaryReader serialized = source->Serialize();
	EXPECT_TRUE(serialized.Size() > 0);

	const std::filesystem::path dataPath = GetObjectTestDataPath();
	std::filesystem::create_directories(dataPath.parent_path());

	const std::vector<uint8_t> serializedBytes =
		serialized.ReadToVector(serialized.Remaining());
	{
		std::ofstream file(dataPath, std::ios::binary | std::ios::trunc);
		ASSERT_TRUE(file.is_open());
		file.write(
			reinterpret_cast<const char*>(serializedBytes.data()),
			static_cast<std::streamsize>(serializedBytes.size()));
		file.close();
		ASSERT_TRUE(static_cast<bool>(file));
	}

	const std::vector<uint8_t> loadedBytes = ReadFileBytes(dataPath);
	EXPECT_TRUE(!loadedBytes.empty());
	EXPECT_EQ(loadedBytes.size(), serializedBytes.size());
	EXPECT_TRUE(loadedBytes == serializedBytes);

	auto restored = ObjectFactory::CreateUnique<SerializableTestObject>();
	BinaryWriter input(loadedBytes);
	restored->Deserialize(input);

	EXPECT_EQ(restored->integer, source->integer);
	EXPECT_TRUE(std::fabs(restored->decimal - source->decimal) < 0.0001f);
	EXPECT_EQ(restored->text, source->text);
	EXPECT_TRUE(restored->scores == source->scores);
	EXPECT_TRUE(restored->tags == source->tags);
	EXPECT_TRUE(restored->inventory == source->inventory);
	EXPECT_TRUE(std::fabs(restored->position.x - source->position.x) < 0.0001f);
	EXPECT_TRUE(std::fabs(restored->position.y - source->position.y) < 0.0001f);
	EXPECT_TRUE(std::fabs(restored->position.z - source->position.z) < 0.0001f);
	std::cout << "Serialized ObjectTest data saved and restored from: "
		<< dataPath.string() << '\n';
}

void ObjectRejectsEmptyInput()
{
	auto object = ObjectFactory::CreateUnique<SerializableTestObject>();
	BinaryWriter emptyInput;

	ExpectThrows<std::invalid_argument>([&] {
		object->Deserialize(emptyInput);
	});
}

void ObjectRejectsSerializationWhenDisabled()
{
	auto object = ObjectFactory::CreateUnique<NonSerializableTestObject>();

	ExpectThrows<std::logic_error>([&] {
		object->Serialize();
	});

	BinaryWriter input;
	input.Write(uint8_t{0});
	ExpectThrows<std::logic_error>([&] {
		object->Deserialize(input);
	});
}

TEST_MAIN(
	ObjectSerializationRoundTrip,
	ObjectRejectsEmptyInput,
	ObjectRejectsSerializationWhenDisabled
)
