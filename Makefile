all: one-file format_all build run

update_rlib:
	cp ../rlib/rlib.c ./rlib.h

format_all:
	clang-format *.c *.h -i

build: 
	gcc rrex2.c -o rrex2 -O2 -Wall -Wextra -static
	
run:
	./rrex2

test:
	$(MAKE) build
	./rrex2 test 

cli: build 
	./rrex2 cli

one-file:
	rmerge rrex2.c > rrex2full.c
	clang-format rrex2full.c -i
	gcc rrex2full.c -o rrexfull.o -O3 -static -Wall -Wextra
	@echo "g++ rrex2full.c -o rrex2full.o -O2"

compiler:
	gcc compiler.c -o compiler.o -O3
	./compiler.o

coverage: 
	@rm -f *.gcda   2>/dev/null
	@rm -f *.gcno   2>/dev/null
	@rm -f rrex.coverage.info   2>/dev/null
	gcc -pg -fprofile-arcs -ftest-coverage -g -o rrex_coverage.o rrex2.c
	./rrex_coverage.o test
	lcov --capture --directory . --output-file rrex.coverage.info
	genhtml rrex.coverage.info --output-directory rrex.coverage
	@rm -f *.gcda   2>/dev/null
	@rm -f *.gcno   2>/dev/null
	@rm -f rrex.coverage.info   2>/dev/null
	@rm -f rrex_coverage.o
	@rm -f gmon.out
	google-chrome rrex.coverage/index.html

build_rrex3:
	gcc rrex3.c -o rrex3 

run_rrex3:
	./rrex3 

coverage_rrex3:
	@rm -f *.gcda   2>/dev/null
	@rm -f *.gcno   2>/dev/null
	@rm -f rrex3.coverage.info   2>/dev/null
	gcc -pg -fprofile-arcs -ftest-coverage -g -o rrex3_coverage.o rrex3.c
	./rrex3_coverage.o test
	lcov --capture --directory . --output-file rrex3.coverage.info
	genhtml rrex3.coverage.info --output-directory rrex3.coverage
	@rm -f *.gcda   2>/dev/null
	@rm -f *.gcno   2>/dev/null
	@rm -f rrex3.coverage.info   2>/dev/null
	@rm -f rrex3_coverage.o
	@rm -f gmon.out
	google-chrome rrex3.coverage/index.html

publish:
	brz add 
	brz commit 
	brz push lp:rrex2

