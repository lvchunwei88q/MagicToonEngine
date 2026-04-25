#pragma once
#include <Common/Core_API.h>

#include <Subsystem/SubsystemContext.h>

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
		static bool Init() {
			// Initialize all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			// Subsystemcontext->InitAllSubsystems(); // This would be an API to initialize all subsystems
			return Subsystemcontext->Init();
		}

		static void Uninstall() {
			// Uninstall all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->Uninstall();
		}
	};
}