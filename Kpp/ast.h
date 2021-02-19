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

			Stmt(const std::string& name, Token type) : name(name), type(type)	{ is_body = false; }

			static Stmt* create(const std::string& name, Token type)			{ return  new Stmt(name, type); }
		};

		struct StmtDecl : public StmtBase
		{
			std::string name;
			Token type;

			StmtDecl(const std::string& name, Token type) : name(name), type(type)	{ is_body = true; }

			static StmtDecl* create(const std::string& name, Token type)			{ return new StmtDecl(name, type); }
		};

		struct StmtBody : public StmtBase
		{
			std::vector<StmtBase*> stmts;

			StmtBody()					{ is_body = true; }

			static StmtBody* create()	{ return new StmtBody(); }
		};

		struct Prototype
		{
			std::vector<StmtBase*> stmts;

			std::string name;

			StmtBody* body;

			Prototype(const std::string& name) : name(name)		{}
			
			static Prototype* create(const std::string& name)	{ return new Prototype(name); }
		};

		struct Printer
		{
			int curr_level = 0;

			void print_body(ast::StmtBody* body);
			void print_prototype(Prototype* prototype);
			void print(const std::vector<Prototype*>& prototypes);
		};
	}
}