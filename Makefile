server:
	gcc GeralCom.c -o GeralCom.o -c
	gcc AppServVar.c -o AppServVar.o -c
	gcc AppServFunc.c -o AppServFunc.o -c
	gcc AppServSystem.c -o AppServSystem.o -c	
	gcc AppServ.c -o AppServ.o -c
	gcc AppServRingVar.c -o AppServRing.o -c
	gcc -o service AppServ.o AppServSystem.o AppServFunc.o AppServVar.o GeralCom.o AppServRing.o
	
client:
	gcc GeralCom.c -o GeralCom.o -c
	gcc reqservfunc.c -o reqservfunc.o -c
	gcc reqservVar.c -o reqservVar.o -c
	
	gcc reqserv.c -o reqserv.o -c
	gcc -o reqserv reqserv.o reqservfunc.o reqservVar.o GeralCom.o 

clean:	
	rm *.o
	 
