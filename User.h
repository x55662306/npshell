#include <string>
#include <vector>

class User
{
	public:
		int id, fd, port;
		std::string ip, name;
		std::vector<std::string> env = {"PATH"}, path = {"bin:."};
		User* next;

		User(int f, std::string i, int p);
		void addEnv(std::string e, std::string p);
		void setUserEnv();
};