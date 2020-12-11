.PHONY:sender receiver clean rs rr

timer.obj:Timer.cpp
	g++ -c Timer.cpp -o timer.obj -fexec-charset=gbk

rdt.obj:rdt.cpp
	g++ -c rdt.cpp -o rdt.obj -fexec-charset=gbk

sender:rdt.obj timer.obj
	g++ rdt.obj timer.obj sender.cpp -o sender.exe -fexec-charset=gbk -l WS2_32 -static

receiver:rdt.obj timer.obj
	g++ rdt.obj timer.obj receiver.cpp -o receiver.exe -fexec-charset=gbk -l WS2_32 -static

rs:sender
	./sender.exe

rr:receiver
	./receiver.exe

clean:
	rm *.exe *.obj
	