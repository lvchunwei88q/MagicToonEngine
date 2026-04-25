#pragma once
#include <Common/api.h>

namespace Core
{
	class CORE_API Core
	{
	public:
		Core();
		~Core();
		static const char* GetVersion() { return "0.0.0.1"; };
	private:

	};
}