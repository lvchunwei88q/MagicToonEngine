#pragma once
#include "Common/CORE_API.h"

////////////////////////////// Engine Base Include
#include "Common/Check.h"
//////////////////////////////

// --------------------------------------- Core Macro ------------------------------------------ // 

// Enumerates the features you can use
enum {
	MSERIALIZATION,
};

#define MMEMBER(...)				// 你要序列化的成员
#define MCLASS(...)					// 你要序列化的类，必须放在类定义的开头
#define GENERATE_BODY()				// 你要序列化的类的成员函数实现，必须放在类定义的结尾

// Visual Studio IntelliSense
#if defined(__INTELLISENSE__)
	#define MAGIC_IN_IDE 1  
	// JetBrains ReSharper C++
#elif defined(__RESHARPER__) || defined(__JETBRAINS_IDE__)
	#define MAGIC_IN_IDE 1
	// Clang tools
#elif defined(__clang_analyzer__)
	#define MAGIC_IN_IDE 1
	// Other static analysis tools
#elif defined(__cppcheck__) || defined(__clang_tidy__)
	#define MAGIC_IN_IDE 1
#else
	#define MAGIC_IN_IDE 0
#endif

// --------------------------------------- Core Macro End ------------------------------------------ // 

namespace Core
{
	class CORE_API Core
	{
	public:
		Core();
		~Core();
		static const char* GetVersion();
	private:

	};
}