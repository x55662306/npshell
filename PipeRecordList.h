#include "PipeRecord.h"

class PipeRecordList
{
	public:
		PipeRecord  *head, *tail;

		PipeRecordList();
		PipeRecord* getHead();
		void add(PipeRecord* pipeRecord);
		PipeRecord* findWithCnt(int cnt, int owner);
		PipeRecord* findWithSender(int sender, int reciever);
		PipeRecord* findAndDeleteWithOwner(int owner);
		PipeRecord* findAndDeleteWithCnt(int cnt, int owner);
		PipeRecord* findAndDeleteWithUser(int sender, int reciever);
		PipeRecord* findAndDeleteWithFd(int fd);
		void countdown(int owner);
};
