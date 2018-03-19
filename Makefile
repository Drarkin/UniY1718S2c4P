all:
	gcc GeralCom.c -o GeralCom.o -c
	gcc AppServVar.c -o AppServVar.o -c
	gcc AppServFunc.c -o AppServFunc.o -c
	gcc AppServSystem.c -o AppServSystem.o -c
	gcc main.c -o main.o -c
	gcc -o rci main.o AppServSystem.o AppServFunc.o AppServVar.o GeralCom.o 
	rm *.o
	