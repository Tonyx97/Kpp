#include <defs.h>

#include "lexer.h"
#include "parser.h"
#include "err_handler.h"

int main()
{
	PRINT(C_WHITE, "---------- LEXER ----------");

	kpp::lexer lexer;

	kpp::err_lexer = &lexer;

	lexer.parse("test.kpp");

	PRINT(C_WHITE, "---------- PARSER ----------");

	kpp::parser parser(lexer);

	parser.parse();

	PRINT(C_WHITE, "---------- AST ----------");

	parser.print_ast();

	PRINT(C_WHITE, "---------- IR ----------");
	PRINT(C_WHITE, "---------- ASM ----------");
	
	return std::cin.get();
}