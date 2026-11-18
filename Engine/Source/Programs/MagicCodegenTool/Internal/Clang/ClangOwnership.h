#pragma once

#include <clang-c/Index.h>

#include <string>

namespace MCT {
	namespace ClangSupport {
		/**
		 * @brief Move-only owner for a libclang CXString.
		 * @details libclang hands out strings the caller must release with
		 * clang_disposeString. Owning them here means a call site may return early (or
		 * throw) without leaking, instead of pairing every allocation with a manual
		 * dispose on each exit path.
		 */
		class ClangString
		{
		public:
			explicit ClangString(CXString value) noexcept
				: m_Value(value)
			{
			}

			ClangString() noexcept = default;
			ClangString(const ClangString&) = delete;
			ClangString& operator=(const ClangString&) = delete;

			ClangString(ClangString&& other) noexcept
				: m_Value(other.m_Value)
			{
				other.m_Value = CXString{};
			}

			ClangString& operator=(ClangString&& other) noexcept
			{
				if (this != &other)
				{
					Reset();
					m_Value = other.m_Value;
					other.m_Value = CXString{};
				}
				return *this;
			}

			~ClangString()
			{
				Reset();
			}

			/**
			 * @brief Release the libclang storage early.
			 */
			void Reset() noexcept
			{
				// A default constructed CXString owns nothing; handing it to
				// clang_disposeString would pass a null payload libclang cannot classify.
				if (m_Value.data != nullptr)
				{
					clang_disposeString(m_Value);
				}
				m_Value = CXString{};
			}

			CXString Get() const noexcept
			{
				return m_Value;
			}

			/**
			 * @brief Copy the libclang text into std::string.
			 * @return The text, or an empty string when libclang returned none.
			 */
			std::string ToString() const
			{
				const char* text = clang_getCString(m_Value);
				return text != nullptr ? std::string(text) : std::string();
			}

		private:
			CXString m_Value = {};
		};

		/**
		 * @brief Copy a libclang string and release its storage immediately.
		 * @param value The libclang string to copy and dispose.
		 * @return The copied text, or an empty string when there is none.
		 */
		inline std::string ToString(CXString value)
		{
			return ClangString(value).ToString();
		}

		/**
		 * @brief Move-only owner for a CXIndex.
		 * @details The index owns every translation unit created from it, so releasing it
		 * on destruction also covers the error and exception paths of the caller.
		 */
		class ClangIndex
		{
		public:
			explicit ClangIndex(CXIndex handle = nullptr) noexcept
				: m_Handle(handle)
			{
			}

			ClangIndex(const ClangIndex&) = delete;
			ClangIndex& operator=(const ClangIndex&) = delete;

			ClangIndex(ClangIndex&& other) noexcept
				: m_Handle(other.m_Handle)
			{
				other.m_Handle = nullptr;
			}

			ClangIndex& operator=(ClangIndex&& other) noexcept
			{
				if (this != &other)
				{
					Reset();
					m_Handle = other.m_Handle;
					other.m_Handle = nullptr;
				}
				return *this;
			}

			~ClangIndex()
			{
				Reset();
			}

			void Reset() noexcept
			{
				if (m_Handle != nullptr)
				{
					clang_disposeIndex(m_Handle);
					m_Handle = nullptr;
				}
			}

			CXIndex Get() const noexcept
			{
				return m_Handle;
			}

			explicit operator bool() const noexcept
			{
				return m_Handle != nullptr;
			}

		private:
			CXIndex m_Handle = nullptr;
		};

		/**
		 * @brief Move-only owner for a CXTranslationUnit.
		 * @details libclang may return a partially built unit even when
		 * clang_parseTranslationUnit2 reports a failure, so the pointer is taken over
		 * before the result is checked and disposed by this type either way.
		 */
		class ClangTranslationUnit
		{
		public:
			explicit ClangTranslationUnit(CXTranslationUnit handle = nullptr) noexcept
				: m_Handle(handle)
			{
			}

			ClangTranslationUnit(const ClangTranslationUnit&) = delete;
			ClangTranslationUnit& operator=(const ClangTranslationUnit&) = delete;

			ClangTranslationUnit(ClangTranslationUnit&& other) noexcept
				: m_Handle(other.m_Handle)
			{
				other.m_Handle = nullptr;
			}

			ClangTranslationUnit& operator=(ClangTranslationUnit&& other) noexcept
			{
				if (this != &other)
				{
					Reset();
					m_Handle = other.m_Handle;
					other.m_Handle = nullptr;
				}
				return *this;
			}

			~ClangTranslationUnit()
			{
				Reset();
			}

			void Reset() noexcept
			{
				if (m_Handle != nullptr)
				{
					clang_disposeTranslationUnit(m_Handle);
					m_Handle = nullptr;
				}
			}

			CXTranslationUnit Get() const noexcept
			{
				return m_Handle;
			}

			explicit operator bool() const noexcept
			{
				return m_Handle != nullptr;
			}

		private:
			CXTranslationUnit m_Handle = nullptr;
		};

		/**
		 * @brief Move-only owner for a CXDiagnostic.
		 */
		class ClangDiagnostic
		{
		public:
			explicit ClangDiagnostic(CXDiagnostic handle = nullptr) noexcept
				: m_Handle(handle)
			{
			}

			ClangDiagnostic(const ClangDiagnostic&) = delete;
			ClangDiagnostic& operator=(const ClangDiagnostic&) = delete;

			ClangDiagnostic(ClangDiagnostic&& other) noexcept
				: m_Handle(other.m_Handle)
			{
				other.m_Handle = nullptr;
			}

			ClangDiagnostic& operator=(ClangDiagnostic&& other) noexcept
			{
				if (this != &other)
				{
					Reset();
					m_Handle = other.m_Handle;
					other.m_Handle = nullptr;
				}
				return *this;
			}

			~ClangDiagnostic()
			{
				Reset();
			}

			void Reset() noexcept
			{
				if (m_Handle != nullptr)
				{
					clang_disposeDiagnostic(m_Handle);
					m_Handle = nullptr;
				}
			}

			CXDiagnostic Get() const noexcept
			{
				return m_Handle;
			}

			explicit operator bool() const noexcept
			{
				return m_Handle != nullptr;
			}

		private:
			CXDiagnostic m_Handle = nullptr;
		};
	}
}
