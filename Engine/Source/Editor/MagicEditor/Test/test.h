#include <Subsystem/SubsystemTemplate.h>
#include <Object/Object.h>
#include <synchapi.h>
#define XXX_API

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

MCLASS();
class CLASS_B : public Core::Object
{
	GENERATE_BODY();
public:

	MMEMBER();
	int x;

	MMEMBER();
	float f;
};

MCLASS();
class XXX_API CLASS_X : public Core::Object
{
	GENERATE_BODY();
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