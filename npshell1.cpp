#include <iostream>
#include <unistd.h>
#include <stdlib.h>  
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "npshell1.h"

using namespace std;

npshell1::npshell1()
{

}

string npshell1::getNextToken(string& s, string delimiter)
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

string npshell1::getNextOp(string s)
{
	if(s.find("|") != string::npos)
		return "|";
	else if(s.find(">") != string::npos)
		return ">";
	else if(s.find("!") != string::npos)
		return "!";
	else 
		return "\n";
}

string npshell1::trim(const string& str, const string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void npshell1::parseCmd(string s)
{
	string cmd, token, cmdName, args, currOp = "", nextOp = "|";
	int p[2][2] = {{0, 0}, {0, 0}}, status = 0, numberOfPipe;
	pid_t prePid = -1, currPid = -1;
	bool lastCmd = false, numberPipe = false;

	//Check if any numberpipe input
	PipeRecord *tmpRecord = NULL;
	tmpRecord = pipeRecordList.findAndDeleteWithCnt(0);
	if(tmpRecord != NULL)
	{	
		if(pipe(p[1])<0)
           	cout << "create pipe1 error" << endl;

		while((currPid = fork()) < 0)
		{
			waitpid(-1, &status, 0);
		}

       	if(currPid == 0)	//Child
       	{
       		close(tmpRecord->pip[1]);
       		close(p[1][0]);
       		dup2(p[1][1], STDOUT_FILENO);
       		close(p[1][1]);

       		int charNum = 0;
			char buffer[100], part[100];

			while( (charNum = read(tmpRecord->pip[0], &buffer, 100)) > 0)
			{
				memset(part, 0, 100);
				memcpy(part, buffer, charNum);
				cout << part;
			}
			close(tmpRecord->pip[0]);
       		delete(tmpRecord);
       		exit(0);
       	}
       	else	//Parent
       	{
       		close(tmpRecord->pip[1]);
       		close(tmpRecord->pip[0]);
	       	delete(tmpRecord);
	       	prePid = currPid;
		}
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
		if(nextOp == "|" && s[0] != ' ')
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
		
		//create pipe
		if(!lastCmd)
		{
			if(pipe(p[1])<0)
           		cout << "create pipe1 error" << endl;
		}
		
		//NumberPipe
		PipeRecord *pipeRecord = NULL;
		if(numberPipe)
		{
			pipeRecord = pipeRecordList.find(numberOfPipe);
			
			if(pipeRecord==NULL)
			{
				pipeRecord = new PipeRecord(numberOfPipe, p[1]);
				pipeRecordList.add(pipeRecord);
			}
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
					dup2(p[1][1], 2);
				}

				if(prePid != -1)	//Not first cmd
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
			if(prePid != -1)
			{
				close(p[0][0]);
				close(p[0][1]);
			}

			//Wait for previous pipe finish or wait for last cmd 
			if(lastCmd)
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

void npshell1::getInput(string input)
{
	//setenv("PATH", "bin:.", 1);

	if(input.find("printenv") != string::npos)
	{
		string cmd, envName;
		cmd = getNextToken(input, " ");
		envName = getNextToken(input, "\n");
		char *result = getenv(envName.c_str());
		if(result)
			cout << result << endl;
		pipeRecordList.countdown();
	}
	else if(input.find("setenv") != string::npos)
	{
		string cmd, envName, newPath;
		cmd = getNextToken(input, " ");
		envName = getNextToken(input, " ");
		newPath = getNextToken(input, "\n");
		setenv(envName.c_str(), newPath.c_str(), 1);
		pipeRecordList.countdown();
	}
	else if(input != "")
	{				
		parseCmd(input);
		pipeRecordList.countdown();
	}

	return;
}