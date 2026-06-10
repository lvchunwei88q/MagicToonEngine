#include "Object.h"
#include "ObjectSystem.h"
#include "IO.h"

namespace Core {
	AUTO_REGISTER(ObjectSystem);

	IObjectSystem* GetObjectSystem() {
		return &ObjectSystem::Get();
	}

	bool ObjectSystem::Init()
	{

		return true;
	}

	void ObjectSystem::Uninstall()
	{
	}

	size_t ObjectSystem::GetObjectNum() {
		return 0;
	}
}