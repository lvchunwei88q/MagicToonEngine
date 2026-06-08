#include <Subsystem/SubsystemTemplate.h>
#include <Object/Object.h>
#include <synchapi.h>

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
};