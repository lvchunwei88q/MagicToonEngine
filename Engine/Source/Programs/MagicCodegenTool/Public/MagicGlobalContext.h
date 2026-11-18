#pragma once

#include <Utilities/Singleton.h>
#include <IO.h>
#include <vector>
#include <string>

#define GENERATE_GET(name, type)                            \
    type Get##name() const { return m_##name; }

#define GENERATE_GET_REF(name, type)                        \
    const type& GetRef##name() const { return m_##name; }   \
    type& GetRef##name() { return m_##name; }

#define GENERATE_SET(name, type)                            \
    void Set##name(const type& val) { m_##name = val; }

#define GENERATE_PROPERTY(name, type)	                    \
    GENERATE_GET(name, type)			                    \
	GENERATE_GET_REF(name, type)		                    \
    GENERATE_SET(name, type)

namespace Context
{
    enum GenerationState {
        Pending,       // Data to be generated
        Completed      // No more data to generate
    };

    struct TaskInfo
    {
        // current task index
        size_t CurrentTaskIndex = 0;
        // all task num
        size_t TaskNum = 0;
    };

    struct HeaderDescriptorData {
        std::vector<std::wstring > ArrayOfHeaderFilePaths;
        std::vector<std::string> HeaderFileComesFromModuleArray;
    };
}

class MagicGlobalContext : public Singleton<MagicGlobalContext>
{
public:
    MagicGlobalContext() = default;
    ~MagicGlobalContext() = default;

    // MagicCodegenTool will read the generateInfoFile and set the TaskInfo and HeaderDescriptorData
    IO::path m_generateDir;
	IO::path m_generateInfoFilePath;

    // Latest file modification time cache
    std::string m_LastModifiedTimeOfLatestFile;

    // GenerationState
    Context::GenerationState m_GenerationStat = Context::GenerationState::Pending;;

    // TaskInfo
    Context::TaskInfo m_Info;
    // Each header file describes data
    std::vector<Context::HeaderDescriptorData> m_HDDs;

    // Utilitie Functions
    GENERATE_PROPERTY(generateDir,IO::path);
    GENERATE_PROPERTY(generateInfoFilePath,IO::path);
    GENERATE_PROPERTY(LastModifiedTimeOfLatestFile,std::string);
    bool LastModifiedTimeOfLatestFile_isValid() const { return !m_LastModifiedTimeOfLatestFile.empty(); }
    
    GENERATE_PROPERTY(Info,Context::TaskInfo);
    const size_t GetCurrentShowTaskIndex() const { return m_Info.CurrentTaskIndex + 1; }
    GENERATE_PROPERTY(HDDs,std::vector<Context::HeaderDescriptorData>);
    [[nodiscard]]Context::HeaderDescriptorData& GetCurrentTaskHeaderDescriptorData() {
        return m_HDDs[m_Info.CurrentTaskIndex];
    }

	bool IsNeedGenerate() { return m_GenerationStat == Context::GenerationState::Pending; }
	void SetGenerateCompleted() { m_GenerationStat = Context::GenerationState::Completed; }
	void SetGeneratePending() { m_GenerationStat = Context::GenerationState::Pending; }
};

// set using
using MGC = decltype(MagicGlobalContext::Get());
