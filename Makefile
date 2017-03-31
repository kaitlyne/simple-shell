default:
	gcc -g main.c files.c command.c sig.c -o simpsh
check:
	./test.sh
clean:
	rm -rf simpsh *.tar.gz *.txt tmpdir-for-makecheck
dist:
	tar -czf lab1-kaitlynechan.tar.gz main.c files.c files.h command.c command.h sig.c sig.h Makefile test.sh README report.pdf
