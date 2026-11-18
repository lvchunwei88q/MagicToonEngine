#pragma once

namespace MCT {

	class MagicCodegenTool
	{
	public:
		MagicCodegenTool() {};
		~MagicCodegenTool() {};

		bool readGenerateInfoFile();
		bool Run();
	private:
		bool readHeaderFiles();
	};
}
