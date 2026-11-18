#pragma once

namespace MHT {

	class MagicHeaderTool
	{
	public:
		MagicHeaderTool() {};
		~MagicHeaderTool() {};

		bool readGenerateInfoFile();
		bool Run();
	private:
		bool readHeaderFiles();
	};
}
