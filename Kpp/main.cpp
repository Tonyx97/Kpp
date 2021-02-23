#include <defs.h>

#include "err_handler.h"
#include "lexer.h"
#include "parser.h"
#include "ir.h"

int main(int argc, char** argv)
{
	PRINT(C_WHITE, "---------- LEXER (Lexic Analysis) ----------\n");

	kpp::lexer lexer;

	kpp::err_lexer = &lexer;

	{
		PROFILE("Lexer Time");

		if (argc != 1)
			lexer.parse(argv[1]);
		else lexer.parse("test.kpp");
	}

	lexer.print_list();

	PRINT(C_WHITE, "\n---------- PARSER (Syntax Analysis) ----------\n");

	kpp::parser parser(lexer);

	{
		PROFILE("Parser Time");
		parser.parse();
	}

	PRINT(C_WHITE, "\n---------- AST ----------\n");

	parser.print_ast();

	PRINT(C_WHITE, "\n---------- IR (Semantic Analysis) ----------\n");

	kpp::ir_parser ir_parser(parser.get_ast());

	{
		PROFILE("IR Parser Time");
		ir_parser.generate();
	}

	PRINT(C_WHITE, "\n---------- IR Code ----------\n");

	ir_parser.print_ir();

	PRINT(C_WHITE, "---------- ASM ----------\n");

	{
		PROFILE("ASM Generation Time");
	}
	
	return std::cin.get();
}