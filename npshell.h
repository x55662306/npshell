#include <string>
#include <cstring>

#include "UserList.h"
#include "PipeRecordList.h"

class npshell
{
	public:
		PipeRecordList pipeRecordList, userPipeList;

		npshell();
		std::string getNextToken(std::string& s, std::string delimiter);
		std::string getNextOp(std::string s);
		std::string getNextUserPipe(std::string s);
		std::string trim(const std::string& str, const std::string& whitespace );
		std::string CheckUserPipe(std::string s, int clientFd, UserList userList, int *fdFromUserPipe, int *fdToUserPipe, int sp[], int rp[]);
		void parseCmd(std::string s, int clientFd, UserList userList);
		void getInput(std::string input, int clientFd, UserList userList);
};