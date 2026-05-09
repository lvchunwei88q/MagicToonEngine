#pragma once
#include <Common/CORE_API.h>

#include <Subsystem/SubsystemContext.h>

#include <typeinfo> 

namespace Core
{
	CORE_API SubsystemContext* GetSubsystemContext();

	// Subsystem base class
	class CORE_API Subsystem
	{
	public:
		Subsystem() = default;

		// api
		virtual bool Init() = 0;
		virtual void Uninstall() = 0;

		template<typename Derived, SubsystemContext::Priority Priority>
		void Register() {
			// Register the subsystem in the system manager
			const char* name = typeid(Derived).name();

			SubsystemContext::Context context;
			context.name = name;
			context.subsystem = this;
			context.priority = Priority; // Default priority, can be customized

			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->RegisterSubsystem(context);
		}
	};

	class CORE_API SubsystemControl
	{
	public:
		static SubsystemContext::SubsystemError Init() {
			// Initialize all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			// Subsystemcontext->Init(); // This would be an API to initialize all subsystems
			return Subsystemcontext->Init();
		}

		static void Register_Init_Callback(Init_Callback func) {
			// Register Init _Callback Function
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->RegisterInitCallbackFunction(func);
		}

		static size_t GetNum() {
			// Get Subsystem Num
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			return Subsystemcontext->Num();
		}

		static void Uninstall() {
			// Uninstall all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->Uninstall();
		}
	};
}