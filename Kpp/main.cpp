#include <defs.h>

#include "lexer.h"
#include "parser.h"
#include "err_handler.h"

int main()
{
	PRINT(C_WHITE, "---------- LEXER ----------\n");

	kpp::lexer lexer;

	kpp::err_lexer = &lexer;

	{
		PROFILE("Lexer Time");
		lexer.parse("test.kpp");
	}

	lexer.print_list();

	PRINT(C_WHITE, "\n---------- PARSER ----------\n");

	kpp::parser parser(lexer);

	{
		PROFILE("Parser Time");
		parser.parse();
	}

	PRINT(C_WHITE, "\n---------- AST ----------\n");

	parser.print_ast();

	PRINT(C_WHITE, "\n---------- IR ----------\n");
	PRINT(C_WHITE, "\n---------- ASM ----------\n");
	
	return std::cin.get();
}