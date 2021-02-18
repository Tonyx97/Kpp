#pragma once

namespace kpp
{
	namespace ast
	{
		struct Expr
		{

		};

		struct StmtBase
		{
			bool is_body;
		};

		struct Stmt : public StmtBase
		{
			std::string name;
			Token type;

			static Stmt* create(const std::string& name, Token type)
			{
				auto stmt = new Stmt();

				stmt->name = name;
				stmt->type = type;
				stmt->is_body = false;

				return stmt;
			}
		};

		struct StmtBody : public StmtBase
		{
			std::vector<StmtBase*> stmts;

			static StmtBody* create()
			{
				auto stmt = new StmtBody();

				stmt->is_body = true;

				return stmt;
			}
		};

		struct Prototype
		{
			std::string name;
			StmtBody* body;

			Prototype(const std::string& name) : name(name) {}
		};

		struct Printer
		{
			void print_body(ast::StmtBody* body);
			void print_prototype(Prototype* prototype);
			void print(const std::vector<Prototype*>& prototypes);
		};
	}
	
	class parser
	{
	private:

		lexer& lex;

		std::vector<ast::Prototype*> prototypes;

	public:

		parser(lexer& lex) : lex(lex) {}
		~parser();

		void print_ast();

		bool parse();

		ast::Prototype* parse_prototype();
		ast::StmtBody* parse_body(ast::StmtBody* body);
		ast::Stmt* parse_statement();
		ast::Expr* parse_expression();
		void parse_literal();

	};
}