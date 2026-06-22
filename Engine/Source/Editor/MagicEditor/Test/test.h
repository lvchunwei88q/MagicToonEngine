#include <Subsystem/SubsystemTemplate.h>
#include <Object.h>
#include <synchapi.h>
#define XXX_API

#include <test.object.generate.h>

class CLASS_A : public SubsystemTemplate<CLASS_A, Core::Priority::Low>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

	template<class Archive>
	void serialize(Archive& archive) {
	}
};

MCLASS(MSERIALIZATION);
class CLASS_B : public MObject
{
	GENERATE_BODY(CLASS_B, ENGINE, "TEST");

public:

	MMEMBER();
	int x = 1;

	MMEMBER();
	float f = 0.0f;
};

MCLASS(MSERIALIZATION);
class XXX_API CLASS_X : public MObject
{
	GENERATE_BODY(CLASS_X, ENGINE, "TEST");

protected:
	Core::DestructionMode GetDestructionMode() override { return Core::DestructionMode::CustomSaveAndDestroy; }
	std::wstring GetObjectCustomSerializedPath() override { return L"D:\\Test\\MTE\\TEST\\ObjectTest.masset"; }
public:

	MMEMBER();
	float o;

	static const CLASS_A Test;
	const CLASS_A TestA = {};
	const CLASS_A TestB{};

	MMEMBER();
	CLASS_A TestC{};
};