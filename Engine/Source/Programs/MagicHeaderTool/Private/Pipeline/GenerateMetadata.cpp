#include "Pipeline/GeneralPipeline.h"
#include <algorithm>
#include <map>
#include <vector>
#include <unordered_set>

namespace MHT {
	namespace GeneralPipeline {
	namespace {
        struct LocalSourcesIndex {
            enum class Sort : uint8_t {
                BeforeBody,
                InBody,
                AfterBody
            };

            std::string Source;
            Sort index = Sort::InBody;
        };

        struct LocalValidateParameter {
            std::string Parameter;
            bool consistent = false;
        };

        struct LocalMetadataSource {
            std::string Source;
            bool NeedMacro = true;
        };

        struct LocalMetaData {
            std::string headerName;
            std::vector<LocalMetadataSource> Sources;
            std::string Metadata;

            void SetSources(const std::vector<LocalMetadataSource>& Sources, const std::string& headerName)
            {
                this->Sources = Sources;
                this->headerName  = headerName;
            }
            void SetMetadata(const std::string& Metadata) { this->Metadata = Metadata; }
        };

        std::string GenerateUndefMacros(const std::vector<std::string>& macroNames) {
            std::ostringstream code;

            for (const auto& macroName : macroNames) {
                code << "#undef " << macroName << "\n";
            }

            return code.str();
        }

        // Build the shared class API and leave extension points for optional capabilities.
        std::string GenerateClassBodyFunction() {
            std::ostringstream code;
            code << "    virtual std::string GetClassName() const override {\n";
            code << "        \n";
            code << "        return #CLASS_NAME;\n";
            code << "    }\n";

            code << "    virtual uint64_t GetClassId() const override {\n";
            code << "        \n";
            code << "        return GET_CLASS_ID_##CLASS_NAME;\n";
            code << "    }\n";

            code << "    virtual uint64_t GetClassHas() const override {\n";
            code << "        \n";
            code << "        return GENERATE_CLASS_HAS_##CLASS_NAME;\n";
            code << "    }\n";

            code << "    virtual Core::ObjectType GetClassType() const override {\n";
            code << "        \n";
            code << "        return Core::ObjectType::OBJECT_TYPE;\n";
            code << "    }\n";

            code << "    virtual Core::ObjectSwitch GetClassSwitch() const override  {\n";
            code << "        \n";
            code << "        return GENERATE_OBJECT_SWITCH_##CLASS_NAME;\n";
            code << "    }\n";

            // use macros generate class function
            code << "    GENERATE_SERIALIZE_##CLASS_NAME()\n";
            code << "    GENERATE_DELEGATES_##CLASS_NAME()\n";
            // ...

            // Set constructor parameters
            code << "    CLASS_NAME() : Core::Object() {                            \n";
            code << "       /*Please use a factory class to create this object */   \n";
            code << "       /*this->ObjectInit();*/                                 \n";
            code << "    }                                                          \n";
            code << "    void generate_object_init_func_() {                        \n";
            code << "       this->ObjectInit();                                     \n";
            code << "    }                                                          \n";
            code << "                                                               \n";
            code << "    ~CLASS_NAME() {                                            \n";
            code << "       this->ObjectUninit();                                   \n";
            code << "    }                                                          \n";
            code << "protected:                                                                     \n";
            code << "    GENERATE_SERIALIZE_BODY_##CLASS_NAME()\n";
            return code.str();
        }

        /**
         * 生成 GetClassId 函数
         * @param classIdMap 类名和类ID的映射表
         * @return 宏
        */
        std::string GenerateClassIdSpecificMacros(
            const std::map<std::string, size_t>& classIdMap
        ) {
            std::ostringstream code;

            for (const auto& [className, classId] : classIdMap) {
                // 生成 GET_CLASS_ID_XXX 宏
                code << "#define GET_CLASS_ID_" << className << " " << classId << "ULL\n";
            }

            return code.str();
        }

        std::string GenerateObjectTagSpecificMacros(
            const std::map<std::string, size_t>& ObjectTagMap
        ) {
            std::ostringstream code;

            for (const auto& [className, Tag] : ObjectTagMap) {
                // 生成 GENERATE_CLASS_HAS_XXX 宏
                code << "#define GENERATE_CLASS_HAS_" << className << " " << Tag << "\n";
            }

            return code.str();
        }

        std::string GenerateFileWithCode(
            const std::vector<std::string>& codeLines,
            const std::string& generatorName = "MagicHeaderTool",
            const std::string& additionalInfo = ""
        ) {
            std::ostringstream result;

            // 生成文件头部
            result << "#pragma once\n\n";
            result << "// ============================================================================\n";
            result << "// This file is generated by " << generatorName << ".\n";
            result << "// DO NOT MODIFY THIS FILE MANUALLY!\n";
            result << "// Any manual changes will be overwritten.\n";

            if (!additionalInfo.empty()) {
                result << "// \n";
                result << "// " << additionalInfo << "\n";
            }

            result << "// ============================================================================\n\n";

            // 此时我们不需要Object上的默认宏了
            result << "#ifdef GENERATE_BODY\n";
            result << "#undef GENERATE_BODY\n";
            result << "#endif\n\n";

            // 追加用户代码
            for (const auto& line : codeLines) {
                result << line << "\n";
            }

            return result.str();
        }

        std::string ConvertFunctionToMacro(const std::vector<LocalSourcesIndex>& funcStrings, const std::string& macroName = "GENERATE_BODY", const std::string& macroSignature = "...") {
            std::ostringstream result;
            result << "#define " << macroName << "(" << macroSignature << ")                    \\\n";
            result << "    [[maybe_unused]] static constexpr bool __check = [] {                \\\n";
            result << "        if constexpr (MAGIC_IN_IDE) {                                    \\\n";
            result << "            /* IDE/Compiler analysis mode - perform checks */            \\\n";
            result << "            CHECK_COMBINE_MEMBER(CLASS_NAME,Object);                     \\\n";
            result << "        }                                                                \\\n";
            result << "        return true;                                                     \\\n";
            result << "    }();                                                                 \\\n";
            result << "public: " << " \\\n";  // Default to public

            bool firstLine = true;

            for (const auto& funcString : funcStrings) {
                if (funcString.index != LocalSourcesIndex::Sort::InBody)
                    continue;   // is no InBody continue

                std::stringstream ss(funcString.Source);
                std::string line;

                while (std::getline(ss, line)) {
                    // 去除换行符和首尾空白
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                    size_t start = line.find_first_not_of(" \t");
                    if (start == std::string::npos) continue;

                    line = line.substr(start);

                    if (!firstLine) {
                        result << " \\\n";
                    }

                    result << "    " << line;
                    firstLine = false;
                }
            }

            return result.str();
        }

        bool ValidateParameters(
            const std::vector<LocalValidateParameter>& a,
            const std::vector<std::string>& b
        ) {
            // 检查长度是否一致
            if (a.size() != b.size()) {
                if (a.size() > b.size()) {
                    Utilitie::Log::Error("Parameter count mismatch: expected " +
                        std::to_string(b.size()) + " parameters, but got " +
                        std::to_string(a.size()) + " parameters (too many)");
                }
                else {
                    Utilitie::Log::Error("Parameter count mismatch: expected " +
                        std::to_string(b.size()) + " parameters, but got " +
                        std::to_string(a.size()) + " parameters (too few)");
                }
                return false;
            }

            // 检查每个参数是否一致
            for (size_t i = 0; i < a.size(); ++i) {
                LocalValidateParameter Parameter = a[i];
                if (Parameter.consistent) {
                    if (Parameter.Parameter != b[i]) {
                        Utilitie::Log::Error("Parameter mismatch at index " + std::to_string(i) +
                            ": expected \"" + b[i] + "\", but got \"" + Parameter.Parameter + "\"");
                        return false;
                    }
                }
                else {
                    // 没有绝对比配那么只要有这个参数就可以
                }
            }

            return true;
        }

        std::string MergeCode(const std::string& code1, const std::string& code2) {
            std::ostringstream result;
            result << code1 << "\n" << code2;
            return result.str();
        }

        uint64_t GlobalAutoincrementedValue() {
            static uint64_t value = 0; // 初始化为0
            return value++;
        }
	}

		bool GenerateMetadata() {
            const auto& classes = MagicGeneralPipelinedData::Get().GetRefMagicClasses();
            auto& generatedMetadata = MagicGeneralPipelinedData::Get().GetRefGeneratedHeaders();
            std::unordered_set<std::string> AllArea;
            std::vector<LocalMetaData> MetaDatas;

            // 获取所有不重复的区域
            for (const auto& classInfo : classes)
            {
                AllArea.insert(classInfo.headerName);
            }

            // 为每个文件生成对应的生成信息 之后生成元数据
            for (const auto& Area : AllArea) 
            {
                std::map<std::string, size_t> GenerateInformationMap_GetClassId;
                std::map<std::string, size_t> GenerateInformationMap_ObjectTag;
                // 校验参数是否正确
                for (const auto& classInfo : classes) {
                    if (Area == classInfo.headerName) {
                        std::vector<LocalValidateParameter> TargetParameter = {
                            {classInfo.className,true},      // 第一个参数是输入class的Name
                            {"ObjectType",false},                   // 第二个参数是输入Object的属于谁
                            // more ...
                        };
                        if (!ValidateParameters(TargetParameter, classInfo.generateBodyParams)) // 参数校验
                            return false;

                        GenerateInformationMap_GetClassId[classInfo.className] = GlobalAutoincrementedValue();
                        GenerateInformationMap_ObjectTag[classInfo.className] = std::hash<std::string>{}(classInfo.className); // Use hash of class name as unique tag
                    }
                }

                // 生成元数据
                std::vector<LocalMetadataSource> Sources;
                std::string src_UndefMacros               = GenerateUndefMacros(MagicGeneralPipelinedData::Get().GetRefAllUndefMacros());
                std::string src_ClassBodyFunction         = GenerateClassBodyFunction();
                std::string src_ClassGetClassIdMacros     = GenerateClassIdSpecificMacros(GenerateInformationMap_GetClassId);
                std::string src_ObjectTagMacros           = GenerateObjectTagSpecificMacros(GenerateInformationMap_ObjectTag);

                // 构造原始元数据
                Sources.push_back({ src_UndefMacros ,               false });
                Sources.push_back({ src_ClassBodyFunction ,         true  });
                Sources.push_back({ src_ClassGetClassIdMacros ,     false });
                Sources.push_back({ src_ObjectTagMacros ,           false });
                MetaDatas.emplace_back().SetSources(Sources, Area);
            }

            // 之后将我们生成的元数据使用宏包裹
            for (size_t i = 0; i < MetaDatas.size(); i++)
            {
                auto& MetaData = MetaDatas[i];
                std::vector<LocalSourcesIndex> Sources;
                bool AfterBody = false; // 在第一个ClassBodyMacro出现之后的Source我们认为他就是在ClassBodyMacro之后
                for (size_t y = 0; y < MetaData.Sources.size(); y++)
                {
                    auto& Source = MetaData.Sources[y];
                    if (Source.NeedMacro) AfterBody = true;
                    Sources.push_back({ Source.Source, Source.NeedMacro ? LocalSourcesIndex::Sort::InBody :
                                        AfterBody ? LocalSourcesIndex::Sort::AfterBody : LocalSourcesIndex::Sort::BeforeBody });
                }

                auto AssembleMetadata = [&Sources](std::string& target, LocalSourcesIndex::Sort index) {
                    for (size_t i = 0; i < Sources.size(); i++)
                    {
                        auto& SourceTemp = Sources[i];

                        if (SourceTemp.index == index) {
                            target = MergeCode(target, SourceTemp.Source);
                        }
                    }
                };

                std::string Source_GENERATE_BODY = ConvertFunctionToMacro(Sources, "GENERATE_BODY", "CLASS_NAME,OBJECT_TYPE");
                std::string Source;

                AssembleMetadata(Source, LocalSourcesIndex::Sort::BeforeBody);
                Source = MergeCode(Source, Source_GENERATE_BODY);
                AssembleMetadata(Source, LocalSourcesIndex::Sort::AfterBody);
                
                MetaData.SetMetadata(Source);
            }

            // 之后生成文件头与解释说明
            for (size_t i = 0; i < MetaDatas.size(); i++)
            {
                auto& MetaData = MetaDatas[i];
                std::string MetaDataScript = GenerateFileWithCode({ MetaData.Metadata });
                MetaData.SetMetadata(MetaDataScript);
            }

            // 最后把数据给MagicBuildData
            for (size_t i = 0; i < MetaDatas.size(); i++)
            {
                auto& MetaData = MetaDatas[i];

                std::string ModelName = "NOT";
                for (const auto& classInfo : classes)
                {
                    if (classInfo.headerName == MetaData.headerName) {
                        ModelName = classInfo.moduleName;
                    }
                }

                generatedMetadata.push_back({ MetaData.headerName, MetaData.Metadata, ModelName });
            }

			return true;
		}
	}
}
