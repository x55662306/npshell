all:npshell.cpp
	g++ np_simple_proc.cpp User.cpp UserList.cpp npshell.cpp PipeRecordList.cpp PipeRecord.cpp -o np_simple_proc
	g++ np_simple.cpp npshell2.cpp PipeRecordList.cpp PipeRecord.cpp -o np_simple
clean:
	rm -f np_simple_proc
	rm -f np_simple
