#pragma once
#include "Common/RENDERUI_API.h"
#include "RenderMode/RenderMode.h"

#include "RenderSubsystem/RenderUICotext.h"

#include <Tools/Singleton.h>
#include <cstdint>
#include <string>

namespace RenderUI {
	class RSubsystem;

	struct RegisterSubsystemContext
	{
		RSubsystem* Subsystem;
		const char* Name;
	};

	// Interface 
	class RENDERUI_API IRSubsystem
	{
	public:
		virtual void RegisterSubsystem(RegisterSubsystemContext context) = 0;
		// Control
		virtual void Init() = 0;
		virtual void Uninstall() = 0;
		virtual void Tick() = 0;
		virtual void Notification(const char* msg) = 0;

		virtual void* GetSubsystemPublicData(std::string Target,uint8_t Type) = 0;

		virtual [[nodiscard]] const RenderUIContext& GetRenderUIContext() const = 0;
		virtual [[nodiscard]] RenderUIContext& SetRenderUIContext() = 0;
	};
	RENDERUI_API IRSubsystem* GetSubsystem();

	// Main Type
	class RENDERUI_API RSubsystem
	{
	public:
		RSubsystem() = default;

		// api
		virtual void Init() = 0;
		virtual void Uninstall() = 0;
		virtual void Tick() = 0;
		virtual void Notification(const char* msg) {};
		virtual void* PublicData(uint8_t type) = 0;

		template<typename Derived, ModeType type>
		void Register() {
			// Register the subsystem in the system manager
			const char* name = typeid(Derived).name();

			RegisterSubsystemContext context{};
			context.Name = name;
			context.Subsystem = this;
			this->Type = type;

			IRSubsystem* Subsystemcontext = GetSubsystem();
			Subsystemcontext->RegisterSubsystem(context);
		}

	protected:
		ModeType Type;
	public:
		[[nodiscard]] ModeType GetModeType() const {
			return Type;
		}
		virtual [[nodiscard]] int GetPriority() const { // 可选
			return 0;
		}
	};

	// the subsystems template Cannot be used as an export API!!!
	template<typename T, ModeType type>
	class RSubsystemTemplate : public RSubsystem, public Singleton<T>
	{
	public:
		static void RegisterStatic() {
			RSubsystemTemplate::Get().template Register<T, type>();
		}
	};

	#define CAT(a, b) a##b
	#define RENDERUI_REGISTER(T) \
		inline static bool CAT(T, _registered) = []() { \
			T::RegisterStatic(); \
			return true; \
		}(); \
		inline bool CAT(T, _dummy) = CAT(T, _registered);
}