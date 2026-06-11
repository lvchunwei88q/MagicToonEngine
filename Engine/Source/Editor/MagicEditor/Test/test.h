#include <Subsystem/SubsystemTemplate.h>
#include <Object.h>
#include <synchapi.h>
#define XXX_API

#include <test.generate.h>

class CLASS_A : public SubsystemTemplate<CLASS_A, Core::Priority::Low>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

private:
};

MCLASS(MSERIALIZATION);
class CLASS_B : public Core::Object
{
	GENERATE_BODY(CLASS_B);

public:

	MMEMBER();
	int x;

	MMEMBER();
	float f;
};

MCLASS(MSERIALIZATION);
class XXX_API CLASS_X : public Core::Object
{
	GENERATE_BODY(CLASS_X);
public:

	MMEMBER();
	float o;

	MMEMBER();
	static const CLASS_A Test;

	MMEMBER();
	const CLASS_A TestA = {};

	MMEMBER();
	const CLASS_A TestB{};

	MMEMBER();
	CLASS_A TestC{};
};