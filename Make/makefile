TapeUtil: lbltp TapeUtil.class libTapeUtil.so
	
lbltp:	linemode.o functions.o fsrtns.o machine.o 
	/usr/ucb/cc  linemode.o functions.o fsrtns.o machine.o -o lbltp
TapeUtil.class: TapeUtil.java 
	/usr/um/jdk/1.1.5/bin/javac -deprecation -g TapeUtil.java 
libTapeUtil.so: TapeRoutines.o functions.o fsrtns.o machine.o 
	ld -G TapeRoutines.o functions.o fsrtns.o machine.o -o libTapeUtil.so
linemode.o:	linemode.c
	/usr/ucb/cc -c linemode.c
functions.o:	functions.c translate.h lbltp.h
	/usr/ucb/cc -c functions.c -g 
fsrtns.o:	fsrtns.c translate.h lbltp.h
	/usr/ucb/cc -c fsrtns.c
machine.o:	 machine.c lbltp.h
	/usr/ucb/cc -c machine.c
TapeRoutines.c:	TapeRoutines.class
	/usr/um/jdk/1.1.5/bin/javah -jni TapeRoutines
TapeRoutines.o:	TapeRoutines.c
	cc -c -I /usr/um/jdk/1.1.5/include  -I /usr/um/jdk/1.1/include/solaris TapeRoutines.c

