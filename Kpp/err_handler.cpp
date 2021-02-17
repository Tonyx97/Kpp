#include <defs.h>

#include "lexer.h"

#include "err_handler.h"

void kpp::critical_error()
{
	std::cin.get();
	exit(EXIT_FAILURE);
}

void kpp::lexer_error(const char* str)
{
	printf_s("LEXICAL ERROR: '%s'\n", str);
}

void kpp::parser_error(const char* str)
{
	if (err_lexer->eof())
		printf_s("SYNTAX ERROR: '%s' (found end-of-fine)\n", str);
	else printf_s("SYNTAX ERROR: '%s'\n", str);

	critical_error();
}