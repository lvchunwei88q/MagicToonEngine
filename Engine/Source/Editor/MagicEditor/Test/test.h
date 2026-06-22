#include <Subsystem/SubsystemTemplate.h>
#include <Object.h>
#include <LuaVM.h>
#include <lua.hpp>
#define XXX_API

#include <test.object.generate.h>

class CLASS_A : public SubsystemTemplate<CLASS_A, Core::Priority::Low>
{
public:
	virtual bool Init() {
		Sleep(2000);
		return true;
	}
	virtual void Uninstall() {}

	template<class Archive>
	void serialize(Archive& archive) {
	}
};

MCLASS(MSERIALIZATION);
class CLASS_B : public MObject
{
	GENERATE_BODY(CLASS_B, ENGINE, "TEST");

public:

	MMEMBER();
	int x = 1;

	MMEMBER();
	float f = 0.0f;
};

MCLASS(MSERIALIZATION);
class XXX_API CLASS_X : public MObject
{
	GENERATE_BODY(CLASS_X, ENGINE, "TEST");

protected:
	Core::DestructionMode GetDestructionMode() override { return Core::DestructionMode::CustomSaveAndDestroy; }
	std::wstring GetObjectCustomSerializedPath() override { return L"D:\\Test\\MTE\\TEST\\ObjectTest.masset"; }
public:

	MMEMBER();
	float o;

	MMEMBER();
	int3 Testint3;
	MMEMBER();
	float2 Testfloat2;
	MMEMBER();
	float3 Testfloat3;

	static const CLASS_A Test;
	const CLASS_A TestA = {};
	const CLASS_A TestB{};

	MMEMBER();
	CLASS_A TestC{};
};

class LuaVMTest {
public:
    /**
     * 运行所有测试
     * @return 所有测试是否通过
     */
    static bool RunAllTests() {
        bool allPassed = true;

        std::cout << "=== LuaVM Test Suite ===" << std::endl;

        allPassed &= TestBasicExecution();
        allPassed &= TestGlobalVariables();
        allPassed &= TestFunctionRegistration();
        allPassed &= TestVectorToTable();
        allPassed &= TestErrorHandling();

        std::cout << "=== All tests " << (allPassed ? "PASSED" : "FAILED") << " ===" << std::endl;
        return allPassed;
    }

private:
    // 测试 1：基本执行
    static bool TestBasicExecution() {
        Core::LuaVM vm;
        std::string error;

        // 执行简单 Lua 代码
        bool result = vm.DoString("print('Hello from Lua!')", &error);

        if (!result) {
            std::cerr << "[FAIL] BasicExecution: " << error << std::endl;
            return false;
        }

        std::cout << "[PASS] BasicExecution" << std::endl;
        return true;
    }

    // 测试 2：设置和获取全局变量
    static bool TestGlobalVariables() {
        Core::LuaVM vm;

        // 设置不同类型的全局变量
        vm.SetGlobal("test_int", 42);
        vm.SetGlobal("test_float", 3.14f);
        vm.SetGlobal("test_bool", true);
        vm.SetGlobal("test_string", "Hello World");

        // 在 Lua 中验证这些变量
        std::string luaCode = R"(
            print("test_int =", test_int)
            print("test_float =", test_float)
            print("test_bool =", test_bool)
            print("test_string = '" .. tostring(test_string) .. "'")
            print("type(test_string) =", type(test_string))
            print("string length =", #test_string)
            print("expected = 'Hello World'")
            assert(test_int == 42, "test_int failed")
            assert(math.abs(test_float - 3.14) < 0.001, "test_float failed")
            assert(test_bool == true, "test_bool failed")
            assert(test_string == "Hello World", "test_string failed")
            print("All global variables verified!")
        )";

        std::string error;
        bool result = vm.DoString(luaCode, &error);

        if (!result) {
            std::cerr << "[FAIL] GlobalVariables: " << error << std::endl;
            return false;
        }

        std::cout << "[PASS] GlobalVariables" << std::endl;
        return true;
    }

    // 测试 3：注册 C++ 函数
    static int AddFunction(lua_State* L) {
        double a = luaL_checknumber(L, 1);  // 第一个参数
        double b = luaL_checknumber(L, 2);  // 第二个参数

        // 2. 计算结果
        double result = a + b;

        // 3. 将结果压入栈
        lua_pushnumber(L, result);
        return 1;
    }

    static bool TestFunctionRegistration() {
        Core::LuaVM vm;

        // 注册 C++ 函数
        vm.RegisterFunction("add", AddFunction);

        // 在 Lua 中调用它
        std::string luaCode = R"(
            local result = add(3, 5)
            assert(result == 8, "add(3,5) should return 8")
            print("add(3,5) = " .. result)
        )";

        std::string error;
        bool result = vm.DoString(luaCode, &error);

        if (!result) {
            std::cerr << "[FAIL] FunctionRegistration: " << error << std::endl;
            return false;
        }

        std::cout << "[PASS] FunctionRegistration" << std::endl;
        return true;
    }

    // 测试 4：C++ vector 转换为 Lua 表
    static bool TestVectorToTable() {
        Core::LuaVM vm;

        // 设置 vector
        std::vector<int> intVec = { 10, 20, 30, 40 };
        std::vector<std::string> strVec = { "apple", "banana", "cherry" };

        vm.SetGlobal("int_vec", intVec);
        vm.SetGlobal("str_vec", strVec);

        // 在 Lua 中验证
        std::string luaCode = R"(
            -- 验证整数表
            assert(#int_vec == 4, "int_vec length should be 4")
            assert(int_vec[1] == 10 and int_vec[2] == 20, "int_vec values incorrect")
            
            -- 验证字符串表
            assert(#str_vec == 3, "str_vec length should be 3")
            assert(str_vec[1] == "apple" and str_vec[2] == "banana", "str_vec values incorrect")
            
            print("Vector to table conversion verified!")
        )";

        std::string error;
        bool result = vm.DoString(luaCode, &error);

        if (!result) {
            std::cerr << "[FAIL] VectorToTable: " << error << std::endl;
            return false;
        }

        std::cout << "[PASS] VectorToTable" << std::endl;
        return true;
    }

    // 测试 5：错误处理
    static bool TestErrorHandling() {
        Core::LuaVM vm;
        std::string error;

        // 执行有错误的 Lua 代码
        bool result = vm.DoString("invalid lua syntax!!!", &error);

        if (result) {
            std::cerr << "[FAIL] ErrorHandling: Expected error but got success" << std::endl;
            return false;
        }

        // 检查错误信息是否包含预期内容
        if (error.find("syntax error") == std::string::npos) {
            std::cerr << "[FAIL] ErrorHandling: Unexpected error message: " << error << std::endl;
            return false;
        }

        std::cout << "[PASS] ErrorHandling (caught: " << error << ")" << std::endl;
        return true;
    }
};
