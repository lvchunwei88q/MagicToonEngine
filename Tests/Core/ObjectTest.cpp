#include "../Common/Test.h"

#include <Object.h>

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

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
};

MCLASS(MSERIALIZATION)
class NonSerializableTestObject final : public MObject
{
	GENERATE_BODY(NonSerializableTestObject, Unknown);
};

void ObjectSerializationRoundTrip()
{
	auto source = ObjectFactory::CreateUnique<SerializableTestObject>();
	source->integer = 42;
	source->decimal = 3.25f;
	source->text = "Object serialization test";

	BinaryReader serialized = source->Serialize();
	EXPECT_TRUE(serialized.Size() > 0);

	BinaryWriter input;
	input.Write(serialized.ReadToVector(serialized.Remaining()));

	auto restored = ObjectFactory::CreateUnique<SerializableTestObject>();
	restored->Deserialize(input);

	EXPECT_EQ(restored->integer, source->integer);
	EXPECT_TRUE(std::fabs(restored->decimal - source->decimal) < 0.0001f);
	EXPECT_EQ(restored->text, source->text);
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
