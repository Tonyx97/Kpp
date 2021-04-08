#include <defs.h>

#include "err_handler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "ssa.h"
#include "reg_alloc.h"

int main(int argc, char** argv)
{
	dbg::setup_console();

	PRINT(C_CYAN, "---------- Lexer (Lexic Analysis) ----------\n");

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

	PRINT(C_CYAN, "\n---------- Parser (Syntax Analysis) ----------\n");

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

	PRINT(C_CYAN, "---------- SSA Generation ----------\n");

	kpp::ssa_gen ssa_gen(ir_gen);

	{
		PROFILE("SSA Time");
		ssa_gen.build_ssa();
	}

	ssa_gen.print_ssa_ir();

	PRINT(C_CYAN, "\n---------- Visuals ----------\n");

	if (GetAsyncKeyState(VK_F2))
	{
		auto cfg_fut = std::async(std::launch::async, &kpp::ssa_gen::display_cfg, ssa_gen);
		auto dom_tree_fut = std::async(std::launch::async, &kpp::ssa_gen::display_dominance_tree, ssa_gen);

		cfg_fut.wait();
		dom_tree_fut.wait();
	}

	PRINT(C_CYAN, "---------- Registers Allocation ----------\n");

	kpp::reg_alloc reg_alloc(ssa_gen);

	{
		PROFILE("Regs Allocation Time");
		reg_alloc.allocate_all();
	}

	PRINT(C_CYAN, "\n---------- ASM ----------\n");

	{
		PROFILE("ASM Generation Time");
	}
	
	Sleep(500);

	do Sleep(100);
	while (!GetAsyncKeyState(VK_ESCAPE));

	return 1;
}