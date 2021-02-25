#pragma once

#include "ast.h"

namespace kpp
{
	namespace ir
	{
		enum IrType
		{
			IR_NONE,
			IR_BODY,
			IR_EXPR,
		};

		enum IrExprType
		{
			IR_EXPR_NONE,
			IR_EXPR_DECL_OR_ASSIGN,
			IR_EXPR_BINARY_OP,
			IR_EXPR_CALL,
		};

		struct IrBase
		{
			IrType ir_type = IR_NONE;
		};

		struct IrExpr : public IrBase
		{
			IrExprType ir_expr_type = IR_EXPR_NONE;

			IrExpr() { ir_type = IR_EXPR; }
		};

		struct IrExprDeclOrAssign : public IrExpr
		{
			std::string name;

			IrExpr* value = nullptr;

			Token type = TOKEN_NONE;

			IrExprDeclOrAssign()		{ ir_expr_type = IR_EXPR_DECL_OR_ASSIGN; }

			bool is_declaration() const { return (type != TOKEN_NONE); }
		};
		
		struct IrBody : public IrBase
		{
			std::vector<IrBase*> items;

			IrBody() { ir_type = IR_BODY; }
		};

		struct PrototypeParam
		{
			std::string name;

			Token type = TOKEN_NONE;

			PrototypeParam(const std::string& name, Token type) : name(name), type(type) {}
		};

		struct Prototype
		{
			std::vector<PrototypeParam*> params;

			std::string name;

			IrBody* body = nullptr;

			Token return_type = TOKEN_NONE;
		};

		struct IR
		{
			std::vector<Prototype*> prototypes;
		};

		struct GlobalInfo
		{
			std::unordered_map<std::string, Prototype*> prototypes;
		};
	}

	class ir_parser
	{
	private:

		ast::AST* tree = nullptr;

		ir::IR iri {};

		ir::GlobalInfo gi {};

		int print_level = 0;

	public:

		ir_parser(ast::AST* tree);
		~ir_parser();

		void print_ir();
		void print_body(ir::IrBody* body);
		void print_expr(ir::IrExpr* expr_base);

		void add_prototype(ir::Prototype* prototype);

		bool generate();

		ir::Prototype* generate_prototype(ast::Prototype* prototype);
		ir::IrBody* generate_body(ast::StmtBody* body);
		ir::IrExpr* generate_expr(ast::Expr* expr);
		ir::IrExprDeclOrAssign* generate_expr_decl_or_assign(ast::ExprDeclOrAssign* expr);

		ir::Prototype* get_defined_prototype(ast::Prototype* prototype);

		ast::Prototype* get_prototype_definition(ast::Prototype* prototype_decl);
	};
}