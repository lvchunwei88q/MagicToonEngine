#pragma once
#include <Subsystem/SubsystemContext.h>
#include <Subsystem/Subsystem.h>
#include <Tools/Singleton.h>

#include <vector>
#include <string>

namespace Core
{
	class SubsystemContextImpl : public SubsystemContext, public Singleton<SubsystemContextImpl>
	{
		struct Context
		{
			std::string name;
			Subsystem* subsystem;
			Priority priority; 
		};
	public:
		SubsystemContextImpl();

		virtual void RegisterSubsystem(SubsystemContext::Context context) override;
		virtual bool Init() override;
		virtual void Uninstall() override;

	private:
		std::vector<Context> Derived;
	};
}