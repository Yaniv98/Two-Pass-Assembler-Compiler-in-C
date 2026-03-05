FLAGS = -g -ansi -pedantic -Wall

main: main.o lineManager.o util.o stringUtil.o macros.o preProcessor.o labels.o memory.o operands.o operations.o firstPass.o secondPass.o errors.o
	 @gcc $(FLAGS) main.o lineManager.o util.o stringUtil.o macros.o preProcessor.o labels.o memory.o operands.o operations.o firstPass.o secondPass.o errors.o -lm -o assembler

lineManager.o: lineManager.c master_header.h
	 @gcc $(FLAGS) -c lineManager.c -o lineManager.o

util.o: util.c master_header.h
	 @gcc $(FLAGS) -c util.c -o util.o

stringUtil.o: stringUtil.c master_header.h
	 @gcc $(FLAGS) -c stringUtil.c -o stringUtil.o

macros.o: macros.c master_header.h
	 @gcc $(FLAGS) -c macros.c -o macros.o

preProcessor.o: preProcessor.c master_header.h
	 @gcc $(FLAGS) -c preProcessor.c -o preProcessor.o

labels.o: labels.c master_header.h
	 @gcc $(FLAGS) -c labels.c -o labels.o

memory.o: memory.c master_header.h
	 @gcc $(FLAGS) -c memory.c -o memory.o

operands.o: operands.c master_header.h
	 @gcc $(FLAGS) -c operands.c -o operands.o

operations.o: operations.c master_header.h
	 @gcc $(FLAGS) -c operations.c -o operations.o

firstPass.o: firstPass.c master_header.h
	 @gcc $(FLAGS) -c firstPass.c -o firstPass.o

secondPass.o: secondPass.c master_header.h
	 @gcc $(FLAGS) -c secondPass.c -o secondPass.o

errors.o: errors.c master_header.h
	 @gcc $(FLAGS) -c errors.c -o errors.o

main.o: main.c master_header.h
	 @gcc $(FLAGS) -c main.c -o main.o

clean: 
	rm -rf *.o
	rm ./assembler
	rm -rf *.am
	rm -rf *.ent
	rm -rf *.ext
	rm -rf *.ob

