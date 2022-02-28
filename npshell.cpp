#include <iostream>
#include <unistd.h>
#include <stdlib.h>  
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <climits>

#include "UserList.h"
#include "npshell.h"

using namespace std;

npshell::npshell()
{
	
}

string npshell::getNextToken(string& s, string delimiter)
{
	size_t pos = 0;
	string token;

	if(s == "")
		return "";

	do
	{
		pos = s.find(delimiter);
		if(pos == string::npos)
		{
			token = s;
			s = "";
		}
		else
		{
			token = s.substr(0, pos);
			s.erase(0, pos + delimiter.length());
		}
	}while(token == "");

	return token;
}

string npshell::getNextOp(string s)
{
	int minPos = INT_MAX;
	string op = "\n";
	string allOp= "|<>!";

	for(int i=0; i<allOp.length(); i++)
	{
		int pos = s.find(allOp[i]);
		if(pos != string::npos && pos < minPos)
		{
			minPos = pos;
			op = allOp[i];
		}
	}

	return op;
}

string npshell::getNextUserPipe(string s)
{
	int minPos = INT_MAX;
	string op = "\n";
	string allOp= "<>";

	for(int i=0; i<allOp.length(); i++)
	{
		int pos = s.find(allOp[i]);
		if(pos != string::npos && pos < minPos)
		{
			minPos = pos;
			op = allOp[i];
		}
	}
	return op;
}

string npshell::trim(const string& str, const string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

string npshell::CheckUserPipe(string s, int clientFd, UserList userList, int *fdFromUserPipe, int *fdToUserPipe, int sp[], int rp[])
{
	int pos, id;
	string noUserPipeStr = "", nextOp = "", tmpOp = "", token = "", sCopy = s.substr(0, s.length()-1), toMsg = "", fromMsg = "";
	User *me = userList.findWithFd(clientFd), *other = NULL; 
	while(s != "")
	{
		nextOp = getNextUserPipe(s);
		pos = s.find(nextOp);
		noUserPipeStr = noUserPipeStr + s.substr(0, pos);
		s.erase(0, pos);

		if(nextOp != "\n" && s[1] != ' ')
		{
			s.erase(0, 1);
			tmpOp = getNextOp(s);
			pos = s.find(tmpOp);
			string strId = trim(s.substr(0, pos));
			s.erase(0, pos);
			string::size_type sz ;
			id = stoi(strId,&sz);

			//Check if id exist
			if((other = userList.findWithId(id)) == NULL)
			{
				string error = "*** Error: user #" + strId + " does not exist yet. ***\n";	
				write(clientFd, error.c_str(), strlen(error.c_str()));
				if(nextOp == ">")
					*fdToUserPipe = -2;
				else
					*fdFromUserPipe = -2;
			}
			else if(nextOp == ">")
			{
				*fdToUserPipe =  other->fd;	
				if(userPipeList.findWithSender(clientFd, *fdToUserPipe) != NULL)
				{
					string strSenderId = to_string(me->id);
					string error = "*** Error: the pipe #" + strSenderId + "->#" + to_string(id) + " already exists. ***\n";
					write(clientFd, error.c_str(), strlen(error.c_str()));
					*fdToUserPipe = -2;
				}
				else
					toMsg = "*** " + me->name + " (#" + to_string(me->id) + ") just piped '" + sCopy + "' to " + other->name + " (#" + to_string(other->id) + ") ***\n";
			}
			else if(nextOp == "<")
			{
				*fdFromUserPipe =  other->fd;	
				if(userPipeList.findWithSender(*fdFromUserPipe, clientFd) == NULL)
				{
					string strRecieverId = to_string(me->id);
					string error = "*** Error: the pipe #" + to_string(id) + "->#" + strRecieverId + " does not exist yet. ***\n";
					write(clientFd, error.c_str(), strlen(error.c_str()));	
					*fdFromUserPipe = -2;
				}
				else
					fromMsg = "*** " + me->name + " (#" + to_string(me->id) + ") just received from " + other->name + " (#" + to_string(other->id) + ") by '" + sCopy + "' ***\n";
			}
		}
		else
		{
			noUserPipeStr = noUserPipeStr + s.substr(0, 1);
			s.erase(0, 1);
		}
	}

	if(fromMsg != "")
			userList.broadcast(fromMsg);
	if(toMsg != "")
		userList.broadcast(toMsg);

	return noUserPipeStr;
}

void npshell::parseCmd(string s, int clientFd, UserList userList)
{
	string cmd, token, cmdName, args, currOp = "", nextOp = "|";
	int p[2][2] = {{0, 0}, {0, 0}}, status = 0, numberOfPipe, pipeIn[2] = {0, 0}, pipPid = 0;
	pid_t prePid = -1, currPid = -1;
	bool lastCmd = false, numberPipe = false, isPipeIn = false;
	//For user pipe
	int fdFromUserPipe = -1, fdToUserPipe = -1, sp[2] = {0, 0}, rp[2] = {0, 0};

	//Check user pipe
	s = CheckUserPipe(s, clientFd, userList, &fdFromUserPipe, &fdToUserPipe, sp, rp);
	//Check if any pipe input
	PipeRecord *tmpRecord = NULL;
	tmpRecord = pipeRecordList.findAndDeleteWithCnt(0, clientFd);	//Number  pipe
	if(fdFromUserPipe > -1 && prePid == -1)//User pipe
	{
		tmpRecord = userPipeList.findAndDeleteWithUser(fdFromUserPipe, clientFd);	
	}

	if(tmpRecord != NULL)
	{	
		isPipeIn = true;
		pipeIn[0] = tmpRecord->pip[0];
		pipeIn[1] = tmpRecord->pip[1];
		free(tmpRecord);
	}
	else if(fdFromUserPipe == -2)
	{
		isPipeIn = true;
		pipeIn[0] = open("/dev/null", O_RDONLY);
		pipeIn[1] = -1;
	}
	
	//Delete redundant space
	s = trim(s);
	while(s != "")
	{
		//Get command type like "|", ">" ...
		currOp = nextOp;
		nextOp = getNextOp(s);

		//Get command and parse
		cmd = getNextToken(s, nextOp);
		cmd = trim(cmd);
		cmdName = getNextToken(cmd, " ");
		args = cmd;
		args = trim(args);
	
		//Check nextOp if numberpipe
		if( (nextOp == "|" || nextOp == "!") && s[0] != ' ')
		{
			numberPipe = true;
			string::size_type sz ;
			numberOfPipe = stoi (s,&sz);
			s = "";
		}
		
		//check if the last cmd
		if(numberPipe)
			lastCmd == false;
		else if(nextOp == "\n" || nextOp == ">")
			lastCmd = true;
		else 
			lastCmd = false;

		//Copy p1 to p0 because p0 is done
		p[0][0] = p[1][0];
		p[0][1] = p[1][1];
		
		//Create pipe
		if(!lastCmd || (lastCmd && fdToUserPipe > -1))
		{
			if(pipe(p[1])<0)
           		cout << "create pipe1 error" << endl;
		}
		else if(lastCmd && fdToUserPipe == -2)
		{

			p[1][0] = -1;
			p[1][1] = open("/dev/null", O_WRONLY);
		}
		
		//NumberPipe
		PipeRecord *pipeRecord = NULL;
		PipeRecord *userPipeRecord = NULL;
		if(numberPipe)
		{
			pipeRecord = pipeRecordList.findWithCnt(numberOfPipe, clientFd);
			
			if(pipeRecord==NULL)
			{
				pipeRecord = new PipeRecord(numberOfPipe, p[1], clientFd);
				pipeRecordList.add(pipeRecord);
			}
		}
		else if(lastCmd && fdToUserPipe > -1)	//User pipe
		{
			userPipeRecord = new PipeRecord(p[1], clientFd, fdToUserPipe);
			userPipeList.add(userPipeRecord);
		}
		
		while((currPid = fork()) < 0)
		{
			waitpid(-1, &status, 0);
		}

		if(currPid == 0) 	//child process
		{
			if(currOp == "|" || currOp == "\n")
			{
				if(nextOp == "!")
				{
					dup2(pipeRecord->pip[1], STDERR_FILENO);
				}

				if(prePid == -1)	//First cmd
				{
					if(isPipeIn)
					{
						close(pipeIn[1]);
						dup2(pipeIn[0], STDIN_FILENO);
						close(pipeIn[0]);
					}
				}
				else	//Not first cmd
				{
					close(p[0][1]);
					dup2(p[0][0], STDIN_FILENO);
					close(p[0][0]);
				}

				if(!lastCmd)	//Multiple cmd and not last cmd
				{
					if(numberPipe)
					{
						dup2(pipeRecord->pip[1], STDOUT_FILENO);
						close(p[1][0]);
						close(p[1][1]);
						if(p[1][0] != pipeRecord->pip[0])
							close(pipeRecord->pip[0]);
						if(p[1][1] != pipeRecord->pip[1])
							close(pipeRecord->pip[1]);	
					}
					else
					{
						dup2(p[1][1], STDOUT_FILENO);	
						close(p[1][1]);
						close(p[1][0]);
					}
				}
				else
				{
					if(fdToUserPipe != -1)	//User pipe
					{
						dup2(p[1][1], STDOUT_FILENO);	
						close(p[1][1]);
						if(p[1][0] > 2)
							close(p[1][0]);
					}

					if(nextOp == ">")
					{
						string path = getNextToken(s, "\n");
						path = trim(path);
						int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
						dup2(fd, STDOUT_FILENO);
						close(fd);
					}
				}

				//Execute cmd
				string error = "Unknown command: [" + cmdName + "].";

				int i = 1;
				char* arg[100];
				string tmpArg;
				arg[0] = strdup(cmdName.c_str());
				while((tmpArg = getNextToken(args, " ")) != "")
				{
					arg[i] = strdup(tmpArg.c_str());
					i++;
				}
				arg[i] = NULL;

				if(execvp(arg[0], arg)<0)
				{
					cerr << error.c_str() << endl;  
					exit(1);
				}		
			}
		}
		else //parent process
		{
			//If next op is "">", clear s
			if(nextOp == ">")
			{
				s = "";
			}
			//If not fir cmd, close previous pipe
			
			if(lastCmd && fdToUserPipe == -2)
			{	
				if(prePid != -1)
				{
					close(p[0][0]);
					close(p[0][1]);
				}
				close(p[1][1]);
			}
			else if(prePid == -1)
			{
				if(isPipeIn)
				{
					close(pipeIn[0]);
					close(pipeIn[1]);
				}
			}
			else
			{
				close(p[0][0]);
				close(p[0][1]);
			}

			//Wait for previous pipe finish or wait for last cmd 
			if(lastCmd && fdToUserPipe < 0 && !numberPipe)
			{
				waitpid(currPid, &status, 0);
			}
			else if(prePid != -1)
			{
				waitpid(prePid, &status, 0);
			}
		} 
		prePid = currPid;
	}

	//clear zombie
	while(waitpid(-1, &status, WNOHANG)>0)
	{
		//donothing
	}
}

void npshell::getInput(string input, int clientFd, UserList userList)
{
	User* user =  userList.findWithFd(clientFd);
	user->setUserEnv();

	if(input.find("printenv") != string::npos)
	{
		string cmd, envName;
		cmd = getNextToken(input, " ");
		envName = getNextToken(input, "\n");
		char *result = getenv(envName.c_str());
		if(result)
			cout << result << endl;
		pipeRecordList.countdown(clientFd);
	}
	else if(input.find("setenv") != string::npos)
	{
		string cmd, envName, newPath;
		cmd = getNextToken(input, " ");
		envName = getNextToken(input, " ");
		newPath = getNextToken(input, "\n");
		setenv(envName.c_str(), newPath.c_str(), 1);
		user->addEnv(envName, newPath);
		pipeRecordList.countdown(clientFd);
	}
	else if(input != "")
	{				
		parseCmd(input, clientFd, userList);
		pipeRecordList.countdown(clientFd);
	}

	return;
}