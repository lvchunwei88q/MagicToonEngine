#pragma once
#include <Common/api.h>

class CORE_API Core
{
public:
	Core();
	~Core();
	static const char* GetVersion() { return "0.0.0.1"; };
private:

};