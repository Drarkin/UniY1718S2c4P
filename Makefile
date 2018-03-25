server:
	gcc GeralCom.c -o GeralCom.o -c -g
	gcc AppServVar.c -o AppServVar.o -c -g
	gcc AppServFunc.c -o AppServFunc.o -c -g
	gcc AppServSystem.c -o AppServSystem.o -c -g
	gcc AppServ.c -o AppServ.o -c -g
	gcc AppServRingVar.c -o AppServRing.o -c -g
	gcc -o service AppServ.o AppServSystem.o AppServFunc.o AppServVar.o GeralCom.o AppServRing.o -g
	
client:
	gcc GeralCom.c -o GeralCom.o -c
	gcc reqservfunc.c -o reqservfunc.o -c
	gcc reqservVar.c -o reqservVar.o -c
	
	gcc reqserv.c -o reqserv.o -c
	gcc -o reqserv reqserv.o reqservfunc.o reqservVar.o GeralCom.o 

clean:	
	rm *.o
	 
