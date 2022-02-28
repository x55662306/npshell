#include <stddef.h>
#include <iostream>
#include "PipeRecordList.h"

PipeRecordList::PipeRecordList()
{
	head = NULL;
	tail = NULL;
}

PipeRecord* PipeRecordList::getHead()
{
	return head;
}

void PipeRecordList::add(PipeRecord* pipeRecord)
{	
	if(head == NULL)
	{
		head = pipeRecord;
		tail = pipeRecord; 
	}
	else
	{
		tail->next = pipeRecord;
		tail = pipeRecord;
	}
	return;
}

PipeRecord* PipeRecordList::findWithCnt(int cnt, int owner)
{
	PipeRecord* tmp = head;

	while(tmp!= NULL && (tmp->cnt != cnt || tmp->owner != owner))
	{
		tmp = tmp->next;
	}
	
	return tmp;
}

PipeRecord* PipeRecordList::findWithSender(int sender, int reciever)
{
	PipeRecord* tmp = head;

	while(tmp!= NULL && (tmp->sender != sender || tmp->reciever != reciever))
	{
		tmp = tmp->next;
	}
	
	return tmp;
}

PipeRecord* PipeRecordList::findAndDeleteWithOwner(int owner)
{
	PipeRecord *tmp = head, *preTmp = NULL;

	if(head == NULL)
		return NULL;
	else if(head->owner == owner)
	{
		if(head == tail)
		{
			head = tail = NULL;
		}
		else
			head = tmp->next;
		
		return tmp;
	}

	while(tmp->next != NULL)
	{
		if(tmp->next->owner == owner)
		{
			PipeRecord *target = tmp->next;
			if(tmp->next == tail)
				tail = tmp;
			tmp->next = tmp->next->next;
			return target;
		}
	}

	return NULL;
}

PipeRecord* PipeRecordList::findAndDeleteWithCnt(int cnt, int owner)
{
	PipeRecord *tmp = head, *preTmp = NULL;

	if(head == NULL)
		return NULL;
	else if(head->cnt == cnt && head->owner == owner)
	{
		if(head == tail)
		{
			head = tail = NULL;
		}
		else
			head = tmp->next;
		
		return tmp;
	}

	while(tmp->next != NULL)
	{
		if(tmp->next->cnt == cnt && tmp->next->owner == owner)
		{
			PipeRecord *target = tmp->next;
			if(tmp->next == tail)
				tail = tmp;
			tmp->next = tmp->next->next;
			return target;
		}
	}

	return NULL;
}

PipeRecord* PipeRecordList::findAndDeleteWithUser(int sender, int reciever)
{
	PipeRecord *tmp = head, *preTmp = NULL;

	if(head == NULL)
		return NULL;
	else if(head->sender == sender && head->reciever == reciever)
	{
		if(head == tail)
		{
			head = tail = NULL;
		}
		else
			head = tmp->next;
		
		return tmp;
	}

	while(tmp->next != NULL)
	{
		if(tmp->next->sender == sender && tmp->next->reciever == reciever)
		{
			PipeRecord *target = tmp->next;
			if(tmp->next == tail)
				tail = tmp;
			tmp->next = tmp->next->next;
			return target;
		}
	}

	return NULL;
}

PipeRecord* PipeRecordList::findAndDeleteWithFd(int fd)
{
	PipeRecord *tmp = head, *preTmp = NULL;

	if(head == NULL)
		return NULL;
	else if(head->sender == fd || head->reciever == fd)
	{
		if(head == tail)
		{
			head = tail = NULL;
		}
		else
			head = tmp->next;
		
		return tmp;
	}

	while(tmp->next != NULL)
	{
		if(tmp->next->sender == fd || tmp->next->reciever == fd)
		{
			PipeRecord *target = tmp->next;
			if(tmp->next == tail)
				tail = tmp;
			tmp->next = tmp->next->next;
			return target;
		}
	}

	return NULL;
}

void PipeRecordList::countdown(int owner)
{
	if(head == NULL)
		return;

	PipeRecord* tmp = head;

	while(tmp!= NULL)
	{
		if(tmp->owner == owner)
			tmp->cnt--;
		tmp = tmp->next;
	}
	return;
}