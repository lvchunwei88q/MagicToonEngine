#pragma once
#include <string>

#define MAGICMEMBER()  // 你要序列化的成员
#define MAGICCLASS()   // 你要序列化的类，必须放在类定义的开头
#define MAGICBODY()    // 你要序列化的类的成员函数实现，必须放在类定义的结尾

namespace Core {
	// obejct 基础类，提供基本的序列化功能
	struct ObjectData;

	/*
	* 提供序列化功能也就是说继承这个类的对象可以被序列化到文件中，也可以从文件中被反序列化出来
	* ObjectTags负责你序列化的类型标签，因为我们会通过优化将多个不同的对象序列化到同一个文件中，所以我们需要使用一个标签来告诉系统你想保存到那个文件区块中，
	* 默认的标签是Object
	*/
	class Object
	{
	public:
		virtual ~Object();
		Object();

	private:
		std::string ObjectTags = "Object";
		ObjectData* Data;
	};
}