.PHONY:clean rs rr

timer:Timer.cpp
	g++ -c Timer.cpp -o timer.obj -fexec-charset=gbk

rdt:rdt.cpp
	g++ -c rdt.cpp -o rdt.obj -fexec-charset=gbk

connection:connection.cpp
	g++ -c connection.cpp -o connection.obj -fexec-charset=gbk

packet:packet.cpp
	g++ -c packet.cpp -o packet.obj -fexec-charset=gbk

rbuf:rbuf.cpp
	g++ -c rbuf.cpp -o rbuf.obj -fexec-charset=gbk

sbuf:sbuf.cpp
	g++ -c sbuf.cpp -o sbuf.obj -fexec-charset=gbk	

sender:rdt timer connection packet rbuf sbuf
	g++ $(shell ls *.obj) sender.cpp -o sender.exe -fexec-charset=gbk -l WS2_32 -static

receiver:rdt timer connection packet rbuf sbuf
	g++ $(shell ls *.obj) receiver.cpp -o receiver.exe -fexec-charset=gbk -l WS2_32 -static

rs:sender
	./sender.exe

rr:receiver
	./receiver.exe

clean:
	rm *.exe *.obj
	