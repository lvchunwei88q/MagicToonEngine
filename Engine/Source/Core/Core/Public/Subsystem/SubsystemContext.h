#pragma once
namespace Core
{
	class SubsystemContext
	{
	public:
		enum class Priority
		{
			Low = -1,      // 低优先级
			Normal = 0,    // 普通优先级
			High = 1,      // 高优先级
		};

		struct Context
		{
			const char* name;
			void* subsystem; // Pointer to the subsystem instance
			Priority priority; // priority
		};

		struct SubsystemError {
			bool error;
			const char* target;
		};

		// API
		virtual void RegisterSubsystem(Context context) = 0;
		virtual SubsystemError Init() = 0;
		virtual void Uninstall() = 0;
	};
}