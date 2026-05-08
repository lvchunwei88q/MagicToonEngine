class CLASS_A : public SubsystemTemplate<CLASS_A, Core::SubsystemContext::Priority::Low>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

private:
};
class CLASS_B : public SubsystemTemplate<CLASS_B, Core::SubsystemContext::Priority::Normal>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

private:
};
class CLASS_C : public SubsystemTemplate<CLASS_C, Core::SubsystemContext::Priority::High>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

private:
};
AUTO_REGISTER(CLASS_A)
AUTO_REGISTER(CLASS_B)
AUTO_REGISTER(CLASS_C)