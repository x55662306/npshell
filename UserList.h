#ifndef _USERLIST_H_
#define _USERLIST_H_

#include <string>

#include "User.h"

class UserList
{
	public:
		User *head, *tail;

		UserList();
		User* getHead();
		User* findWithFd(int fd);
		User* findWithId(int id);
		void add(User* User);
		void remove(int fd);
		void broadcast(std::string msg);
		void who(int fd);
		void tell(int fd, int recieverId, std::string msg);
		void yell(int fd, std::string msg);
		void tellNewUser(int fd, std::string ip, int port);
		void changeName(int fd, std::string name);
};

#endif