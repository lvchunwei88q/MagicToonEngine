#include "../Common/Test.h"

#include <clang-c/Index.h>

#include <iostream>
#include <set>
#include <string>

namespace {

struct ParsedSymbols
{
	std::set<std::string> Functions;
	std::set<std::string> Classes;
	std::set<std::string> GlobalVariables;
	std::set<std::string> ClassVariables;
};

std::string ToString(CXString value)
{
	const char* text = clang_getCString(value);
	std::string result = text ? text : "";
	clang_disposeString(value);
	return result;
}

enum CXChildVisitResult VisitCursor(CXCursor cursor, CXCursor, CXClientData clientData)
{
	auto& symbols = *static_cast<ParsedSymbols*>(clientData);
	const CXCursorKind kind = clang_getCursorKind(cursor);
	const std::string name = ToString(clang_getCursorSpelling(cursor));
	const CXCursor semanticParent = clang_getCursorSemanticParent(cursor);
	const CXCursorKind parentKind = clang_getCursorKind(semanticParent);

	switch (kind)
	{
	case CXCursor_FunctionDecl:
	case CXCursor_CXXMethod:
	case CXCursor_Constructor:
	case CXCursor_Destructor:
	case CXCursor_ConversionFunction:
	case CXCursor_FunctionTemplate:
		symbols.Functions.insert(name);
		break;
	case CXCursor_ClassDecl:
	case CXCursor_StructDecl:
	case CXCursor_ClassTemplate:
		symbols.Classes.insert(name);
		break;
	case CXCursor_VarDecl:
		if (parentKind == CXCursor_TranslationUnit || parentKind == CXCursor_Namespace)
		{
			symbols.GlobalVariables.insert(name);
		}
		else if (parentKind == CXCursor_ClassDecl ||
			parentKind == CXCursor_StructDecl ||
			parentKind == CXCursor_ClassTemplate)
		{
			symbols.ClassVariables.insert(name);
		}
		break;
	case CXCursor_FieldDecl:
		if (parentKind == CXCursor_ClassDecl ||
			parentKind == CXCursor_StructDecl ||
			parentKind == CXCursor_ClassTemplate)
		{
			symbols.ClassVariables.insert(name);
		}
		break;
	default:
		break;
	}

	return CXChildVisit_Recurse;
}

void PrintSymbols(const char* label, const std::set<std::string>& symbols)
{
	std::cout << label << ":";
	for (const std::string& symbol : symbols)
	{
		std::cout << " " << symbol;
	}
	std::cout << '\n';
}

void LibClangParsesAndIdentifiesDeclarations()
{
	static constexpr char source[] = R"cpp(
		namespace Demo
		{
			int globalCount = 3;
			double globalRatio = 1.5;
			int* globalCountPointer = &globalCount;
			int& globalCountReference = globalCount;
			int globalArray[4] = { 1, 2, 3, 4 };

			template <typename T, unsigned N>
			struct Buffer
			{
				T elements[N];
				unsigned length;

				T& At(unsigned index)
				{
					return elements[index];
				}
			};

			class Entity
			{
			public:
				int id;
				virtual void Tick(float delta) = 0;
			};

			class Transform : public Entity
			{
			public:
				struct Node
				{
					Node* next;
					Transform* owner;
				};

				Buffer<float, 3> position;
				const Entity* target;
				Entity& owner;
				Node* firstNode;
				void (*callback)(int, const Entity&);
				int Entity::* selectedMember;
				static Buffer<int, 2> sharedValues;

				explicit Transform(Entity& entity) : owner(entity) {}
				void Tick(float delta) override;
				void SetTarget(const Entity* newTarget)
				{
					target = newTarget;
				}
			};

			Buffer<int, 2> Transform::sharedValues = {};
			Buffer<int, 5> globalBuffer = {};
			Entity* activeEntity = nullptr;

			int Add(int left, int right)
			{
				return left + right;
			}

			template <typename T>
			T Identity(T value)
			{
				return value;
			}
		}
		)cpp";

	CXIndex index = clang_createIndex(0, 0);
	ASSERT_TRUE(index != nullptr);

	CXUnsavedFile unsavedFile = {
		"LibClangSample.cpp",
		source,
		sizeof(source) - 1
	};
	const char* arguments[] = { "-x", "c++", "-std=c++20" };
	CXTranslationUnit translationUnit = nullptr;
	const CXErrorCode parseResult = clang_parseTranslationUnit2(
		index,
		unsavedFile.Filename,
		arguments,
		static_cast<int>(sizeof(arguments) / sizeof(arguments[0])),
		&unsavedFile,
		1,
		CXTranslationUnit_None,
		&translationUnit);

	ASSERT_EQ(parseResult, CXError_Success);
	ASSERT_TRUE(translationUnit != nullptr);

	bool hasErrors = false;
	const unsigned diagnosticCount = clang_getNumDiagnostics(translationUnit);
	for (unsigned i = 0; i < diagnosticCount; ++i)
	{
		CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, i);
		const CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diagnostic);
		if (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal)
		{
			hasErrors = true;
			std::cerr << ToString(clang_getDiagnosticSpelling(diagnostic)) << '\n';
		}
		clang_disposeDiagnostic(diagnostic);
	}
	EXPECT_FALSE(hasErrors);

	if (!hasErrors)
	{
		ParsedSymbols symbols;
		clang_visitChildren(
			clang_getTranslationUnitCursor(translationUnit),
			VisitCursor,
			&symbols);

		PrintSymbols("Functions", symbols.Functions);
		PrintSymbols("Classes", symbols.Classes);
		PrintSymbols("Global variables", symbols.GlobalVariables);
		PrintSymbols("Class variables", symbols.ClassVariables);

		EXPECT_TRUE(symbols.Functions.count("Add") == 1);
		EXPECT_TRUE(symbols.Functions.count("Identity") == 1);
		EXPECT_TRUE(symbols.Functions.count("Tick") == 1);
		EXPECT_TRUE(symbols.Functions.count("SetTarget") == 1);
		EXPECT_TRUE(symbols.Functions.count("At") == 1);
		EXPECT_TRUE(symbols.Classes.count("Buffer") == 1);
		EXPECT_TRUE(symbols.Classes.count("Entity") == 1);
		EXPECT_TRUE(symbols.Classes.count("Transform") == 1);
		EXPECT_TRUE(symbols.Classes.count("Node") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalCount") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalRatio") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalCountPointer") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalCountReference") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalArray") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("globalBuffer") == 1);
		EXPECT_TRUE(symbols.GlobalVariables.count("activeEntity") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("elements") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("length") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("position") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("target") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("owner") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("firstNode") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("callback") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("selectedMember") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("sharedValues") == 1);
		EXPECT_TRUE(symbols.ClassVariables.count("next") == 1);
	}

	clang_disposeTranslationUnit(translationUnit);
	clang_disposeIndex(index);
}

} // namespace

TEST_MAIN(LibClangParsesAndIdentifiesDeclarations)
