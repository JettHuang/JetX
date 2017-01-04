// \brief
//		implementation for Helper

#include "UtilityHelper.h"


std::string FUtilityHelper::ReadTextFile(const char* InFileName)
{
	std::string TextString;
	std::ifstream File;

	File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream StrStream;

		File.open(InFileName);
		StrStream << File.rdbuf();
		TextString = StrStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR: FUtilityHelper::ReadTextFile: " << InFileName << " FAILED" << ", REASON: " << e.what() << std::endl;
	}

	return TextString;
}
