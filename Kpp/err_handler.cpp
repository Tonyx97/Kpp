#include <defs.h>

#include "lexer.h"

#include "err_handler.h"

void kpp::critical_error()
{
	std::cin.get();
	exit(EXIT_FAILURE);
}

void kpp::parser_error(const char* str)
{
	if (err_lexer->eof())
		PRINT(C_RED, "SYNTAX ERROR: %s (found end-of-line)", str);
	else PRINT(C_RED, "SYNTAX ERROR: %s", str);

	critical_error();
}