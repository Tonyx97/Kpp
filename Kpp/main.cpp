#include <defs.h>

#include "err_handler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"

uint64_t upow(uint64_t val, uint64_t n)
{
	uint64_t res = val;
	for (int i = 0; i < n; ++i)
		res *= n;
	return res;
}

int main(int argc, char** argv)
{
	PRINT(C_CYAN, "---------- LEXER (Lexic Analysis) ----------\n");

	kpp::lexer lexer;

	kpp::err_lexer = &lexer;

	{
		PROFILE("Lexer Time");

		if (argc != 1)
			lexer.parse(argv[1]);
		else lexer.parse("test.kpp");
	}

	lexer.print_list();
	lexer.print_errors();

	PRINT(C_CYAN, "\n---------- PARSER (Syntax Analysis) ----------\n");

	kpp::parser parser(lexer);

	{
		PROFILE("Parser Time");
		parser.parse();
	}

	PRINT(C_CYAN, "\n---------- AST ----------\n");

	parser.print_ast();

	PRINT(C_CYAN, "\n---------- Semantic Analysis ----------\n");

	kpp::semantic semantic(parser.get_ast());
	
	{
		PROFILE("Semantic Analyzer Time");

		if (!semantic.analyze())
		{
			semantic.print_errors();

			return std::cin.get();
		}
	}

	PRINT(C_GREEN, "\nOK");
	
	PRINT(C_CYAN, "\n---------- IR ----------\n");

	kpp::ir_gen ir_gen(parser.get_ast());

	{
		PROFILE("IR Time");
		ir_gen.generate();
	}

	PRINT(C_CYAN, "\n---------- IR Code ----------\n");

	ir_gen.print_ir();

	PRINT(C_CYAN, "---------- ASM ----------\n");

	{
		PROFILE("ASM Generation Time");
	}
	
	return std::cin.get();
}