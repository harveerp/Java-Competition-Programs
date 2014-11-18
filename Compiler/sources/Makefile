c12: input.o admin.o scanner.o word_table.o symbol_table.o parser.o tracer.o ast.o semanalyzer.o ident_table.o access_table.o error_reporting.o interm_codegen.o
	gcc $(CFLAGS) -o c12 input.o admin.o scanner.o word_table.o symbol_table.o parser.o tracer.o ast.o semanalyzer.o ident_table.o access_table.o error_reporting.o interm_codegen.o
input.o: input.c input.h c12.h
	gcc $(CFLAGS) -c input.c -std=c99
admin.o: admin.c admin.h c12.h input.o parser.o word_table.o semanalyzer.o error_reporting.o
	gcc $(FLAGS) -c admin.c -std=c99
scanner.o: scanner.c scanner.h c12.h admin.o word_table.o symbol_table.o error_reporting.o
	gcc $(FLAGS) -c scanner.c -std=c99
word_table.o: word_table.c word_table.h c12.h
	gcc $(FLAGS) -c word_table.c -std=c99
symbol_table.o: symbol_table.c symbol_table.h c12.h
	gcc $(FLAGS) -c symbol_table.c -std=c99
parser.o: parser.c parser.h c12.h scanner.o tracer.o ast.o error_reporting.o
	gcc $(FLAGS) -c parser.c -std=c99
tracer.o: tracer.c tracer.h c12.h
	gcc $(FLAGS) -c tracer.c -std=c99
ast.o: ast.c ast.h c12.h 
	gcc $(FLAGS) -c ast.c -std=c99
semanalyzer.o: semanalyzer.c semanalyzer.h c12.h ast.o ident_table.o access_table.o error_reporting.o
	gcc $(FLAGS) -c semanalyzer.c -std=c99
ident_table.o: ident_table.c ident_table.h c12.h ast.h access_table.o
	gcc $(FLAGS) -c ident_table.c -std=c99
access_table.o: access_table.c access_table.h c12.h
	gcc $(FLAGS) -c access_table.c -std=c99 
error_reporting.o: error_reporting.c error_reporting.h c12.h
	gcc $(FLAGS) -c error_reporting.c -std=c99
interm_codegen.o:  interm_codegen.c interm_codegen.h c12.h ast.o
	gcc $(FLAGS) -c interm_codegen.c -std=c99
	
clean:
	rm -f *.o reciprocal

