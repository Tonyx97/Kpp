#include <defs.h>

#include "lexer.h"
#include "parser.h"
#include "err_handler.h"

int main()
{
	printf_s("---------- LEXER ----------\n");

	kpp::lexer lexer;

	kpp::err_lexer = &lexer;

	lexer.parse("test.kpp");

	printf_s("---------- PARSER ----------\n");

	kpp::parser parser(lexer);

	parser.parse();

	printf_s("---------- AST ----------\n");

	parser.print_ast();

	printf_s("---------- IR ----------\n");
	printf_s("---------- ASM ----------\n");
	
	return std::cin.get();
}