#include <stddef.h>
#include <string.h>
#include <unistd.h> 
#include <string>
#include <iostream>

#include "UserList.h"

UserList::UserList()
{
	head = NULL;
	tail = NULL;
}

User* UserList::getHead()
{
	return head;
}

User* UserList::findWithFd(int fd)
{
	User* tmp = head;

	while(tmp!= NULL && tmp->fd != fd)
	{
		tmp = tmp->next;
	}
	
	return tmp;
}

User* UserList::findWithId(int id)
{
	User* tmp = head;

	while(tmp!= NULL && tmp->id != id)
	{
		tmp = tmp->next;
	}
	
	return tmp;
}

void UserList::add(User* user)
{	
	if(head == NULL)
	{
		head = user;
		tail = user; 
	}
	else
	{
		int cnt = 1;
		User* tmp = head;

		//Insert first
		if(tmp->id != cnt)
		{
			user->id = cnt;
			user->next = tmp;
			head = user;
			return;
		}

		cnt++;

		//Insert middle
		while(tmp->next != NULL)
		{
			if(tmp->next->id != cnt)
			{
				user->id = cnt;
				user->next = tmp->next;
				tmp->next = user;
				return;
			}
			tmp = tmp->next;
			cnt++;
		}

		//Insert last
		user->id = tmp->id+1;
		tmp->next = user;
		tail = user;
	}
	return;
}

void UserList::remove(int fd)
{
	User *tmp = head;

	//Remove first
	if(head->fd == fd)
	{
		tmp = head->next;

		std::string msg = "*** User '" + head->name + "' left. ***\n";
		broadcast(msg);
		free(head);

		head = tmp;

		return;
	}

	while(tmp->next!= NULL)
	{
		if(tmp->next->fd == fd)
		{
			User *deleteTmp = tmp->next;
			//Remove last
			if(tmp->next == tail)
			{
				tail = tmp;
				tmp->next = NULL;
			}
			else
				tmp->next = tmp->next->next;

			std::string msg = "*** User '" + deleteTmp->name + "' left. ***\n";
			broadcast(msg);
			free(deleteTmp);
			
			return;
		}	
		tmp = tmp->next;
	}	

	return;
}

void UserList::broadcast(std::string msg)
{
	User* tmp = head;

	while(tmp!= NULL)
	{

		write(tmp->fd, msg.c_str(), strlen(msg.c_str()));
		tmp = tmp->next;
	}

	return;
}

void UserList::tellNewUser(int fd, std::string ip, int port)
{
	User* user = findWithFd(fd);

	std::string msg = "*** User '" + user->name + "' entered from " + user->ip + ":" + std::to_string(user->port) + ". ***\n";
	//std::string msg = "*** User '(" + user->name + ")' entered from " + user->ip + ". ***\n";

	broadcast(msg);

	return;
}

void UserList::who(int fd)
{
	User* tmp = head;
	std::string msg = "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";

	while(tmp!= NULL)
	{
		msg = msg + std::to_string(tmp->id) + "\t" + tmp->name + "\t" + tmp->ip + ":" + std::to_string(tmp->port);
		if(tmp->fd == fd)
			msg = msg + "\t<-me\n";
		else
			msg = msg + "\n";
		tmp = tmp->next;
	}

	write(fd, msg.c_str(), strlen(msg.c_str()));

	return;
}

void UserList::tell(int fd, int recieverId, std::string msg)
{
	User *sender = findWithFd(fd), *reciever = findWithId(recieverId);

	msg = "*** " + sender->name + " told you ***: " + msg + "\n";

	
	if(reciever == NULL)
	{
		std::string error = "*** Error: user #" + std::to_string(recieverId) + " does not exist yet. ***\n";
		write(sender->fd, error.c_str(), strlen(error.c_str()));
	}
	else
		write(reciever->fd, msg.c_str(), strlen(msg.c_str()));	

	return;
}

void UserList::yell(int fd, std::string msg)
{
	msg = "*** " + findWithFd(fd)->name + " yelled ***: " + msg + "\n";

	broadcast(msg);

	return;
}

void UserList::changeName(int fd, std::string name)
{
	//Check if exist same name
	User* tmp = head;
	
	while(tmp!= NULL)
	{
		if(tmp->name == name)
		{
			std::string error = "*** User '" + name + "' already exists. ***\n";
			write(fd, error.c_str(), strlen(error.c_str()));
			return;
		}
		tmp = tmp->next;
	}


	User* user = findWithFd(fd);
	user->name = name;

	std::string msg = "*** User from " + user->ip + ":" + std::to_string(user->port) + " is named '" + user->name + "'. ***\n";

	broadcast(msg);

	return;
}