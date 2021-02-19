#pragma once

namespace kpp
{
	enum Token : int
	{
		TOKEN_ID,
		TOKEN_KEYWORD,
		TOKEN_INT_LITERAL,
		TOKEN_COMMENT,

		TOKEN_SHR_ASSIGN,
		TOKEN_SHL_ASSIGN,
		TOKEN_ADD_ASSIGN,
		TOKEN_INC,
		TOKEN_DEC,
		TOKEN_SUB_ASSIGN,
		TOKEN_MUL_ASSIGN,
		TOKEN_MOD_ASSIGN,
		TOKEN_DIV_ASSIGN,
		TOKEN_OR_ASSIGN,
		TOKEN_AND_ASSIGN,
		TOKEN_XOR_ASSIGN,
		TOKEN_GTE,
		TOKEN_LTE,

		TOKEN_COLON,
		TOKEN_SEMICOLON,
		TOKEN_COMMA,

		TOKEN_PAREN_OPEN,
		TOKEN_PAREN_CLOSE,
		TOKEN_BRACE_OPEN,
		TOKEN_BRACE_CLOSE,
		TOKEN_BRACKET_OPEN,
		TOKEN_BRACKET_CLOSE,
		TOKEN_ADD,
		TOKEN_SUB,
		TOKEN_MUL,
		TOKEN_MOD,
		TOKEN_DIV,
		TOKEN_SHR,
		TOKEN_SHL,
		TOKEN_AND,
		TOKEN_OR,
		TOKEN_XOR,
		TOKEN_NOT,
		TOKEN_ASSIGN,

		TOKEN_EQUAL,
		TOKEN_NOT_EQUAL,
		TOKEN_GT,
		TOKEN_LT,

		TOKEN_VOID,
		TOKEN_BOOL,
		TOKEN_U8,
		TOKEN_U16,
		TOKEN_U32,
		TOKEN_U64,
		TOKEN_I8,
		TOKEN_I16,
		TOKEN_I32,
		TOKEN_I64,
		TOKEN_M128,
		TOKEN_FOR,
		TOKEN_WHILE,
		TOKEN_IF,
		TOKEN_ELSE,
		TOKEN_BREAK,
		TOKEN_CONTINUE,
		TOKEN_RETURN,
		TOKEN_EXTERN,

		TOKEN_EOF,
		TOKEN_NONE,
	};

	struct token_info
	{
		std::string value;
		Token id = TOKEN_NONE;

		template <typename T>
		T get()
		{
			switch (id)
			{
			case TOKEN_INT_LITERAL: return (int)std::atol(value.c_str());
			}

			return T {};
		}

		operator bool() { return (id != TOKEN_NONE); }
	};

	inline std::unordered_map<std::string, token_info> keywords =
	{
		{ "for",		{ "for",		TOKEN_FOR } },
		{ "while",		{ "while",		TOKEN_WHILE } },
		{ "if",			{ "if",			TOKEN_IF } },
		{ "else",		{ "else",		TOKEN_ELSE } },
		{ "break",		{ "break",		TOKEN_BREAK } },
		{ "continue",	{ "continue",	TOKEN_CONTINUE } },
		{ "return",		{ "return",		TOKEN_RETURN } },
		{ "extern",		{ "extern",		TOKEN_EXTERN } },
	};

	inline std::unordered_map<std::string, token_info> keywords_type =
	{
		{ "void",	{ "void",	TOKEN_VOID } },
		{ "bool",	{ "bool",	TOKEN_BOOL } },
		{ "u8",		{ "u8",		TOKEN_U8 } },
		{ "u16",	{ "u16",	TOKEN_U16 } },
		{ "u32",	{ "u32",	TOKEN_U32 } },
		{ "u64",	{ "u64",	TOKEN_U64 } },
		{ "i8",		{ "i8",		TOKEN_I8 } },
		{ "i16",	{ "i16",	TOKEN_I16 } },
		{ "i32",	{ "i32",	TOKEN_I32 } },
		{ "i64",	{ "i64",	TOKEN_I64 } },
		{ "m128",	{ "m128",	TOKEN_M128 } },
	};

	inline token_info static_tokens[] =
	{
		{ ">>=", TOKEN_SHR_ASSIGN },
		{ "<<=", TOKEN_SHL_ASSIGN },

		{ "==", TOKEN_EQUAL },
		{ "!=", TOKEN_NOT_EQUAL },
		{ ">=", TOKEN_GTE },
		{ "<=", TOKEN_LTE },
		{ "+=", TOKEN_ADD_ASSIGN },
		{ "-=", TOKEN_SUB_ASSIGN },
		{ "++", TOKEN_INC },
		{ "--", TOKEN_DEC },
		{ "*=", TOKEN_MUL_ASSIGN },
		{ "%=", TOKEN_MOD_ASSIGN },
		{ "/=", TOKEN_DIV_ASSIGN },
		{ "&=", TOKEN_AND_ASSIGN },
		{ "|=", TOKEN_OR_ASSIGN },
		{ "^=", TOKEN_XOR_ASSIGN },
		{ ">>", TOKEN_SHR },
		{ "<<", TOKEN_SHL },

		{ ";", TOKEN_SEMICOLON },
		{ ",", TOKEN_COMMA },
		{ "(", TOKEN_PAREN_OPEN },
		{ ")", TOKEN_PAREN_CLOSE },
		{ "{", TOKEN_BRACKET_OPEN },
		{ "}", TOKEN_BRACKET_CLOSE },
		{ "[", TOKEN_BRACE_OPEN },
		{ "]", TOKEN_BRACE_CLOSE },
		{ "+", TOKEN_ADD },
		{ "-", TOKEN_SUB },
		{ "*", TOKEN_MUL },
		{ "%", TOKEN_MOD },
		{ "/", TOKEN_DIV },
		{ "&", TOKEN_AND },
		{ "|", TOKEN_OR },
		{ "^", TOKEN_XOR },
		{ "!", TOKEN_NOT },
		{ "=", TOKEN_ASSIGN },
		{ ">", TOKEN_GT },
		{ "<", TOKEN_LT },
	};

	namespace regex
	{
		static inline std::regex SINGLE_COMMENT("\\/{2}.*$"),
								 WORD("\\b[a-zA-Z]\\w*\\b"),
								 INT_LITERAL("\\b[0-9]{1,20}((u|i)(8|16|32|64))?\\b");
	}

	using opt_token_info = std::optional<token_info>;

	inline std::string STRINGIFY_TOKEN(Token id)
	{
		switch (id)
		{
		case TOKEN_ID:					return "TOKEN_ID";
		case TOKEN_KEYWORD:				return "TOKEN_KEYWORD";
		case TOKEN_INT_LITERAL:			return "TOKEN_INT_LITERAL";
		case TOKEN_COMMENT:				return "TOKEN_COMMENT";
		case TOKEN_SHR_ASSIGN:			return "TOKEN_SHR_ASSIGN";
		case TOKEN_SHL_ASSIGN:			return "TOKEN_SHL_ASSIGN";
		case TOKEN_ADD_ASSIGN:			return "TOKEN_ADD_ASSIGN";
		case TOKEN_INC:					return "TOKEN_INC";
		case TOKEN_DEC:					return "TOKEN_DEC";
		case TOKEN_SUB_ASSIGN:			return "TOKEN_SUB_ASSIGN";
		case TOKEN_MUL_ASSIGN:			return "TOKEN_MUL_ASSIGN";
		case TOKEN_MOD_ASSIGN:			return "TOKEN_MOD_ASSIGN";
		case TOKEN_DIV_ASSIGN:			return "TOKEN_DIV_ASSIGN";
		case TOKEN_OR_ASSIGN:			return "TOKEN_OR_ASSIGN";
		case TOKEN_AND_ASSIGN:			return "TOKEN_AND_ASSIGN";
		case TOKEN_XOR_ASSIGN:			return "TOKEN_XOR_ASSIGN";
		case TOKEN_GTE:					return "TOKEN_GTE";
		case TOKEN_LTE:					return "TOKEN_LTE";
		case TOKEN_COLON:				return "TOKEN_COLON";
		case TOKEN_SEMICOLON:			return "TOKEN_SEMICOLON";
		case TOKEN_COMMA:				return "TOKEN_COMMA";
		case TOKEN_PAREN_OPEN:			return "TOKEN_PAREN_OPEN";
		case TOKEN_PAREN_CLOSE:			return "TOKEN_PAREN_CLOSE";
		case TOKEN_BRACE_OPEN:			return "TOKEN_BRACE_OPEN";
		case TOKEN_BRACE_CLOSE:			return "TOKEN_BRACE_CLOSE";
		case TOKEN_BRACKET_OPEN:		return "TOKEN_BRACKET_OPEN";
		case TOKEN_BRACKET_CLOSE:		return "TOKEN_BRACKET_CLOSE";
		case TOKEN_ADD:					return "TOKEN_ADD";
		case TOKEN_SUB:					return "TOKEN_SUB";
		case TOKEN_MUL:					return "TOKEN_MUL";
		case TOKEN_MOD:					return "TOKEN_MOD";
		case TOKEN_DIV:					return "TOKEN_DIV";
		case TOKEN_SHR:					return "TOKEN_SHR";
		case TOKEN_SHL:					return "TOKEN_SHL";
		case TOKEN_AND:					return "TOKEN_AND";
		case TOKEN_OR:					return "TOKEN_OR";
		case TOKEN_XOR:					return "TOKEN_XOR";
		case TOKEN_NOT:					return "TOKEN_NOT";
		case TOKEN_ASSIGN:				return "TOKEN_ASSIGN";
		case TOKEN_EQUAL:				return "TOKEN_EQUAL";
		case TOKEN_NOT_EQUAL:			return "TOKEN_NOT_EQUAL";
		case TOKEN_GT:					return "TOKEN_GT";
		case TOKEN_LT:					return "TOKEN_LT";
		case TOKEN_VOID:				return "TOKEN_VOID";
		case TOKEN_BOOL:				return "TOKEN_BOOL";
		case TOKEN_U8:					return "TOKEN_U8";
		case TOKEN_U16:					return "TOKEN_U16";
		case TOKEN_U32:					return "TOKEN_U32";
		case TOKEN_U64:					return "TOKEN_U64";
		case TOKEN_I8:					return "TOKEN_I8";
		case TOKEN_I16:					return "TOKEN_I16";
		case TOKEN_I32:					return "TOKEN_I32";
		case TOKEN_I64:					return "TOKEN_I64";
		case TOKEN_M128:				return "TOKEN_M128";
		case TOKEN_FOR:					return "TOKEN_FOR";
		case TOKEN_WHILE:				return "TOKEN_WHILE";
		case TOKEN_IF:					return "TOKEN_IF";
		case TOKEN_ELSE:				return "TOKEN_ELSE";
		case TOKEN_BREAK:				return "TOKEN_BREAK";
		case TOKEN_CONTINUE:			return "TOKEN_CONTINUE";
		case TOKEN_RETURN:				return "TOKEN_RETURN";
		case TOKEN_EXTERN:				return "TOKEN_EXTERN";
		case TOKEN_EOF:					return "TOKEN_EOF";
		}

		return "TOKEN_NONE";
	}

	class lexer
	{
	private:

		std::vector<token_info> tokens,
								eaten_tokens;

	public:

		lexer()  {}
		~lexer() {}

		bool parse(const std::string& filename);

		token_info eat();

		std::optional<token_info> eat_expect(Token expected_token);
		std::optional<token_info> eat_expect_keyword_declaration();

		void push_and_pop_token(const token_info& token);

		bool is_token_keyword(const token_info& token);
		bool is_token_keyword_type(const token_info& token);

		bool is_token_keyword_type()			{ return is_token_keyword_type(current()); }
		bool is_current(Token id)				{ return (current_token() == id); }
		bool eof()								{ return tokens.empty(); }

		token_info token(int i) const			{ return tokens[i]; }

		token_info current() const				{ return (tokens.empty() ? token_info { "eof", TOKEN_EOF } : tokens.back()); }

		Token current_token() const				{ return (tokens.empty() ? TOKEN_EOF : tokens.back().id); }
		
		std::string current_value() const		{ return (tokens.empty() ? std::string {} : tokens.back().value); }

		const size_t get_tokens_count() const	{ return tokens.size(); }
	};
}