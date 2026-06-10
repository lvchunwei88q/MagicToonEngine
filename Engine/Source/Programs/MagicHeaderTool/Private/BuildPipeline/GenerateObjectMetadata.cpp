#include "BuildPipeline.h"
#include <map>
#include <vector>
#include <unordered_set>

namespace MHT {
	namespace {
        /**
         * 生成序列化函数
         */
        std::string GenerateSerializeFunctionFormatted(
            const std::map<std::string, std::vector<MemberVariable>>& classParamsMap,
            const std::string& templateArg = "Archive"
        ) {
            std::ostringstream code;

            code << "template<class " << templateArg << ">\n";
            code << "void serialize(" << templateArg << "& archive) {\n";
            code << "    using ThisType = std::decay_t<decltype(*this)>;\n";
            code << "    \n";

            bool firstClass = true;
            for (const auto& [className, params] : classParamsMap) {
                if (!firstClass) {
                    code << "    } else ";
                }
                code << "if constexpr (std::is_same_v<ThisType, " << className << ">) {\n";
                code << "        archive(";
                for (size_t i = 0; i < params.size(); ++i) {
                    const MemberVariable& Variable = params[i];
                    code << Variable.name;
                    if (i < params.size() - 1) {
                        code << ", ";
                    }
                }
                code << ");\n";
                firstClass = false;
            }

            if (!classParamsMap.empty()) {
                code << "    }\n";
            }
            code << "}\n";
            return code.str();
        }
	}

	namespace Pipeline {
        struct MetaData {
            std::string Metadata;
        };

		bool GenerateObjectMetadata() {
            const auto& MagicEngineClasss = MagicBuildData::Get().GetRefMagicEngineClasss();
            auto& MagicObjectMetadatas = MagicBuildData::Get().GetRefMagicObjectMetadatas();
            std::unordered_set<std::string> AllArea;
            std::vector<MetaData> MetaDatas;

            // 获取所有不重复的区域
            for (size_t i = 0; i < MagicEngineClasss.size(); i++)
            {
                auto& MagicEngineClass = MagicEngineClasss[i];
                AllArea.insert(MagicEngineClass.headerName);
            }

            // 为每个文件生成对应的生成信息 之后生成元数据
            for (const auto& Area : AllArea) 
            {
                std::map<std::string, std::vector<MemberVariable>> GenerateInformationMap;
                for (size_t y = 0; y < MagicEngineClasss.size(); y++)
                {
                    auto& MagicEngineClass = MagicEngineClasss[y];
                    if (Area == MagicEngineClass.headerName) {
                        GenerateInformationMap[MagicEngineClass.className] = MagicEngineClass.members;
                    }
                }
                // 生成元数据
                std::string MetaData = GenerateSerializeFunctionFormatted(GenerateInformationMap);
                MetaDatas.push_back({ MetaData });
            }

			return true;
		}
	}
}