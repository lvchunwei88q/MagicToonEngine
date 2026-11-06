#include "../Common/Test.h"

#include <Object.h>

#include <cmath>
#include <stdexcept>
#include <string>

#include <ObjectTest.object.generate.h>

MCLASS(MSERIALIZATION)
class TestObject final : public Core::Object
{
	GENERATE_BODY(TestObject, Unknown);
public:
	MMEMBER();
	int32_t integer = 0;
	MMEMBER();
	float decimal = 0.0f;
	MMEMBER();
	std::string text;
	MMEMBER();
	bool serializable = true;
};

void ObjectSerializationRoundTrip()
{
	TestObject source;
	source.integer = 42;
	source.decimal = 3.25f;
	source.text = "Object serialization test";

	BinaryReader serialized = source.Serialize();
	EXPECT_TRUE(serialized.Size() > 0);

	BinaryWriter input;
	input.Write(serialized.ReadToVector(serialized.Remaining()));

	TestObject restored;
	restored.Deserialize(input);

	EXPECT_EQ(restored.integer, source.integer);
	EXPECT_TRUE(std::fabs(restored.decimal - source.decimal) < 0.0001f);
	EXPECT_EQ(restored.text, source.text);
}

void ObjectRejectsEmptyInput()
{
	TestObject object;
	BinaryWriter emptyInput;
	bool threw = false;

	try {
		object.Deserialize(emptyInput);
	}
	catch (const std::invalid_argument&) {
		threw = true;
	}

	EXPECT_TRUE(threw);
}

void ObjectRejectsSerializationWhenDisabled()
{
	TestObject object;
	object.serializable = false;
	bool threw = false;

	try {
		object.Serialize();
	}
	catch (const std::logic_error&) {
		threw = true;
	}

	EXPECT_TRUE(threw);
}

TEST_MAIN(
	ObjectSerializationRoundTrip,
	ObjectRejectsEmptyInput,
	ObjectRejectsSerializationWhenDisabled
)
