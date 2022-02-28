#include <string>
#include <cstring>

#include "PipeRecordList.h"

class npshell1
{
	public:
		PipeRecordList pipeRecordList;

		npshell1();
		std::string getNextToken(std::string& s, std::string delimiter);
		std::string getNextOp(std::string s);
		std::string trim(const std::string& str, const std::string& whitespace );
		void parseCmd(std::string s);
		void getInput(std::string input);
};