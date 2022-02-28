#include <stddef.h>
#include "PipeRecord.h"

PipeRecord::PipeRecord(int c, int p[], int o)
{
	cnt = c;
	pip[0] = p[0];
	pip[1] = p[1];
	owner = o;
	next = NULL;
}

PipeRecord::PipeRecord(int p[], int s, int r)
{
	cnt = -1;
	pip[0] = p[0];
	pip[1] = p[1];
	sender = s;
	reciever = r;
	next = NULL;
}
