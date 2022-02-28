class PipeRecord
{
	public:
		int cnt, pip[2], sender, reciever, owner;
		PipeRecord* next;

		PipeRecord(int c, int p[], int o);
		PipeRecord(int p[], int s, int r);
};
