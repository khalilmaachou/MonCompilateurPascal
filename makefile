all:	test
clean:
		rm *.o *.s
		rm tokeniser.cpp
tokeniser.cpp:	tokeniser.l
		flex++ -d -otokeniser.cpp tokeniser.l
tokeniser.o:	tokeniser.cpp
		g++ -c tokeniser.cpp
compilateur:	compilateur.cpp tokeniser.o
		g++ -ggdb -o compilateur compilateur.cpp tokeniser.o
test.s:		compilateur test.p
		./compilateur <test.p >test.s
test:		test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
BoucleForEmbr:		compilateur boucleForEmbr.p
		./compilateur <boucleForEmbr.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
BoucleUntil:		compilateur doUntil.p
		./compilateur <doUntil.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
BoucleUntilNot:		compilateur doUntilNot.p
		./compilateur <doUntilNot.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
SwitchCaseFlo:		compilateur switchCaseFlo.p
		./compilateur <switchCaseFlo.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
SwitchCaseInt:		compilateur switchCaseInt.p
		./compilateur <switchCaseInt.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
SwitchCaseChar:		compilateur switchCaseChar.p
		./compilateur <switchCaseChar.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test
ModelJourlin:		compilateur modelJourlin.p   
		./compilateur <modelJourlin.p >test.s
		gcc -ggdb -no-pie -fno-pie test.s -o test



