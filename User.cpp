#include <stddef.h>

#include "User.h"

User::User(int f, std::string i, int p)
{
	id = 1;
	fd = f;
	ip = i;
	port = p;
	name = "(no name)";
	next = NULL;
}

void User::addEnv(std::string e, std::string p)
{
	for(int i = 0; i < env.size(); i++) 
	{
        if(env[i] == e)
        {
        	path[i] = p;
        	return;
        }
    }

    env.push_back(e);
    path.push_back(p);

	return;
}

void User::setUserEnv()
{
	for(int i = 0; i < env.size(); i++) 
	{
        setenv(env[i].c_str(), path[i].c_str(), 1);
    }	

    return;
}