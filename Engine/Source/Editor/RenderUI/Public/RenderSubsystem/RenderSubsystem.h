#pragma once
#include "Common/RENDERUI_API.h"

#include "RenderSubsystem/RenderUICotext.h"

#include <Tools/Singleton.h>
#include <cstdint>
#include <string>

namespace RenderUI {
	class RSubsystem;
	using Function = std::function<void(void*)>;

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

		virtual void* GetSubsystemPublicData(std::string Target, uint8_t Type) = 0;
		virtual void CallSubsystemFunction(std::string Target, std::string Func, void* data) = 0;

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

		template<typename Derived>
		void Register(const char* name) {
			// Register the subsystem in the system manager

			RegisterSubsystemContext context{};
			context.Name = name;
			context.Subsystem = this;

			this->RegisterFunctions();		// 注册函数指针到FunctionArray
			IRSubsystem* Subsystemcontext = GetSubsystem();
			Subsystemcontext->RegisterSubsystem(context);
		}

	protected:
		// not
	public:
		struct FunctionArray {
			// 注册函数数组
			std::vector<Function> FunctionArray;
			std::vector<std::string> FunctionNames;
			size_t FunctionArraySize = 0;
		};
		FunctionArray* Array = nullptr;
		void SetFunctionArray(FunctionArray* array) {
			Array = array;
			Array->FunctionArraySize = Array->FunctionArray.size();
		}
		virtual void RegisterFunctions() {};
	public:
		virtual [[nodiscard]] int GetPriority() const { // 可选
			return 0;
		}
	};

	// the subsystems template Cannot be used as an export API!!!
	template<typename T>
	class RSubsystemTemplate : public RSubsystem, public Singleton<T>
	{
	public:
		static void RegisterStatic(const char* name) {
			RSubsystemTemplate::Get().template Register<T>(name);
		}
	};

	#define CAT(a, b) a##b
	#define RENDERUI_REGISTER(T) \
		inline static bool CAT(T, _registered) = []() { \
			T::RegisterStatic(#T); \
			return true; \
		}(); \
		inline bool CAT(T, _dummy) = CAT(T, _registered);

	// 注册成员函数为函数指针到FunctionArray，使用宏简化注册过程
	#define RS_REGISTER_METHODS(...)												\
		void RegisterFunctions() override {										\
			FunctionArray* Array = new FunctionArray();							\
			__VA_ARGS__															\
			this->SetFunctionArray(Array);										\
		}

	#define RS_METHOD(MethodName)																\
		Array->FunctionArray.push_back([this](void* data) { this->MethodName(data); });		\
		Array->FunctionNames.push_back(#MethodName);
}