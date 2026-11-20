#include "../Common/TestFramework.h"

#include <IO.h>

namespace {

void PrintPath(const char* name, const IO::path& path)
{
	std::cout << name << ": " << path.string() << '\n';
	EXPECT_FALSE(path.empty());
}

}

void AbsolutePathOutputsPaths()
{
	IO::AbsolutePath& absolutePath = IO::AbsolutePath::Get();

	PrintPath("Engine path", absolutePath.GetEnginePath());
	PrintPath("Engine directory", absolutePath.GetEngineDirectory());

	const IO::WorkingDirectory workingDirectory =
		absolutePath.GetCurrentWorkingDirectory();
	PrintPath("Working directory", workingDirectory.GetRoot());
	PrintPath("Working content", workingDirectory.GetContent());
	PrintPath("Working config", workingDirectory.GetConfig());
	PrintPath("Working cache", workingDirectory.GetCache());

	const IO::ContentDirectory contentDirectory =
		absolutePath.GetContentDirectory();
	PrintPath("Content directory", contentDirectory.GetRoot());
	PrintPath("Content editor", contentDirectory.GetEditor());
	PrintPath("Content config", contentDirectory.GetConfig());
	PrintPath("Content logo", contentDirectory.GetLogo());
	PrintPath("Content window", contentDirectory.GetWindow());

	PrintPath("Script directory", absolutePath.GetScriptDirectory().GetRoot());

	const IO::EngineOntologyDirectory engineOntologyDirectory =
		absolutePath.GetEngineOntologyDirectory();
	PrintPath("Engine ontology directory", engineOntologyDirectory.GetRoot());
	PrintPath("Engine ontology config", engineOntologyDirectory.GetConfig());
	PrintPath("Engine ontology cache", engineOntologyDirectory.GetCache());
}

TEST_MAIN(AbsolutePathOutputsPaths)
