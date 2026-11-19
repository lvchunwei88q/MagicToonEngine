#pragma once

#include <array>
#include <string_view>

namespace MCT {
	namespace Codegen {
		/**
		 * @brief One member the generator adds to every MCLASS class, in both renderings.
		 * @details The same interface is written down twice: BuildParseSource() fakes a
		 * declaration so libclang can analyse a header whose generated include does not
		 * exist yet, and GenerateClassBodyFunction() writes the real definitions into the
		 * generated header. Both used to be independent hand-written string blobs, so
		 * renaming a member (for example GetClassHas -> GetClassHash) needed an edit in
		 * each of them, and a missed edit only surfaced as a clang error pointing at a
		 * line inside the user's header - a line the tool itself had replaced.
		 *
		 * Both renderings now iterate kClassOverrideMethods, so the two sides cannot drift:
		 * a member is added, renamed or retyped here once.
		 *
		 * Signature stops before the body; Body stops before the braces. The generated
		 * file text may contain macros (CLASS_NAME and the GET_/GENERATE_ pair it expands
		 * to), which is why Body is spelled out as source text rather than as C++ code.
		 */
		struct ClassOverrideMethod
		{
			/** Declaration text, e.g. "virtual uint64_t GetClassId() const override". */
			std::string_view Signature;

			/** Function body, e.g. "return GET_CLASS_ID_##CLASS_NAME;". */
			std::string_view Body;
		};

		inline constexpr std::string_view kGetClassNameSignature =
			"virtual std::string GetClassName() const override";
		inline constexpr std::string_view kGetClassIdSignature =
			"virtual uint64_t GetClassId() const override";
		inline constexpr std::string_view kGetClassHasSignature =
			"virtual uint64_t GetClassHas() const override";
		inline constexpr std::string_view kGetClassTypeSignature =
			"virtual Core::ObjectType GetClassType() const override";
		inline constexpr std::string_view kGetClassSwitchSignature =
			"virtual Core::ObjectSwitch GetClassSwitch() const override";

		/**
		 * @brief The virtual overrides of Core::Object that every generated class provides.
		 * @details The base declarations live in Core/Public/Object.h; this table only
		 * describes how the generator spells them.
		 */
		inline constexpr std::array<ClassOverrideMethod, 5> kClassOverrideMethods = { {
			{ kGetClassNameSignature,   "return #CLASS_NAME;" },
			{ kGetClassIdSignature,     "return GET_CLASS_ID_##CLASS_NAME;" },
			{ kGetClassHasSignature,    "return GENERATE_CLASS_HAS_##CLASS_NAME;" },
			{ kGetClassTypeSignature,   "return Core::ObjectType::OBJECT_TYPE;" },
			{ kGetClassSwitchSignature, "return GENERATE_OBJECT_SWITCH_##CLASS_NAME;" },
		} };

		/** Non-override member the generated class also exposes. */
		inline constexpr std::string_view kGenerateObjectInitFuncSignature =
			"void generate_object_init_func_()";
	}
}
