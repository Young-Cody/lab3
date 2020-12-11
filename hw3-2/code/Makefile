.PHONY:clean

timer.obj:Timer.cpp
	g++ -c Timer.cpp -o timer.obj -fexec-charset=gbk

rdt.obj:rdt.cpp
	g++ -c rdt.cpp -o rdt.obj -fexec-charset=gbk

connection.obj:connection.cpp
	g++ -c connection.cpp -o connection.obj -fexec-charset=gbk

packet.obj:packet.cpp
	g++ -c packet.cpp -o packet.obj -fexec-charset=gbk

rbuf.obj:rbuf.cpp
	g++ -c rbuf.cpp -o rbuf.obj -fexec-charset=gbk

sbuf.obj:sbuf.cpp
	g++ -c sbuf.cpp -o sbuf.obj -fexec-charset=gbk	

sender:rdt.obj timer.obj connection.obj packet.obj rbuf.obj sbuf.obj
	g++ $(shell ls *.obj) sender.cpp -o sender.exe -fexec-charset=gbk -l WS2_32 -static

receiver:rdt.obj timer.obj connection.obj packet.obj rbuf.obj sbuf.obj
	g++ $(shell ls *.obj) receiver.cpp -o receiver.exe -fexec-charset=gbk -l WS2_32 -static

rs:sender
	./sender.exe

rr:receiver
	./receiver.exe

clean:
	rm *.exe *.obj
	