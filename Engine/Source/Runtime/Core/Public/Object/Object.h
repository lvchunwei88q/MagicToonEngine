#pragma once
#include <string>

enum {
	MREFLECTION,
	MSERIALIZATION,
};

#define MMEMBER(...)				// 你要序列化的成员
#define MCLASS(...)					// 你要序列化的类，必须放在类定义的开头
#define GENERATE_BODY()				// 你要序列化的类的成员函数实现，必须放在类定义的结尾

namespace Core {
	/*
	* 提供序列化功能也就是说继承这个类的对象可以被序列化到文件中，也可以从文件中被反序列化出来
	* 但是所有的类型必须要在Subsystem:Core初始化完成之后构造
	*/
	class Object
	{
	public:
		virtual ~Object();
		Object();

		virtual uint64_t GetClassId() const = 0; // 使用MagicBuildTool生成的类ID
		uint64_t GetInstanceId() const { return instance_id; }; // 为每个对象实例分配一个唯一的ID

		template<class Archive>
		void serialize(Archive& archive) {
			// NOT
		}
	private:
		static uint64_t GetNextId() {
			static uint64_t counter = 0;
			return ++counter;
		}

		uint64_t instance_id;
	};

}