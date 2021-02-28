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

		TOKEN_LOGICAL_AND,
		TOKEN_LOGICAL_OR,

		TOKEN_EQUAL,
		TOKEN_NOT_EQUAL,
		TOKEN_GT,
		TOKEN_LT,

		TOKEN_VOID,
		TOKEN_BOOL,
		TOKEN_TRUE,
		TOKEN_FALSE,
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

	static constexpr int LOWEST_PRECEDENCE = 16;

	struct token_info
	{
		std::string value;

		Token id = TOKEN_NONE;

		int precedence = LOWEST_PRECEDENCE,
			line = 0;

		bool is_operator = false;

		template <typename T>
		T get()
		{
			switch (id)
			{
			case TOKEN_INT_LITERAL: return (int)std::atol(value.c_str());
			}

			return T {};
		}

		operator bool() { return (id != TOKEN_NONE && id != TOKEN_EOF); }
	};

	inline std::unordered_map<std::string, Token> keywords =
	{
		{ "for",		TOKEN_FOR },
		{ "while",		TOKEN_WHILE },
		{ "if",			TOKEN_IF },
		{ "else",		TOKEN_ELSE },
		{ "break",		TOKEN_BREAK },
		{ "continue",	TOKEN_CONTINUE },
		{ "return",		TOKEN_RETURN },
		{ "extern",		TOKEN_EXTERN },		// not an statement but we put it here for now
	};
	
	inline std::unordered_map<std::string, Token> keywords_type =
	{
		{ "void",	TOKEN_VOID },
		{ "bool",	TOKEN_BOOL },
		{ "true",	TOKEN_TRUE },
		{ "false",	TOKEN_FALSE },
		{ "u8",		TOKEN_U8 },
		{ "u16",	TOKEN_U16 },
		{ "u32",	TOKEN_U32 },
		{ "u64",	TOKEN_U64 },
		{ "i8",		TOKEN_I8 },
		{ "i16",	TOKEN_I16 },
		{ "i32",	TOKEN_I32 },
		{ "i64",	TOKEN_I64 },
		{ "m128",	TOKEN_M128 },
	};

	inline token_info static_tokens[] =
	{
		{ ">>=", TOKEN_SHR_ASSIGN, 14, 0, true },
		{ "<<=", TOKEN_SHL_ASSIGN, 14, 0, true },

		{ "==", TOKEN_EQUAL, 7, 0, true },
		{ "!=", TOKEN_NOT_EQUAL, 7, 0, true },
		{ ">=", TOKEN_GTE, 6, 0, true },
		{ "<=", TOKEN_LTE, 6, 0, true },
		{ "+=", TOKEN_ADD_ASSIGN, 14, 0, true },
		{ "-=", TOKEN_SUB_ASSIGN, 14, 0, true },
		{ "++", TOKEN_INC, 1, 0, true },
		{ "--", TOKEN_DEC, 1, 0, true },
		{ "*=", TOKEN_MUL_ASSIGN, 14, 0, true },
		{ "%=", TOKEN_MOD_ASSIGN, 14, 0, true },
		{ "/=", TOKEN_DIV_ASSIGN, 14, 0, true },
		{ "&=", TOKEN_AND_ASSIGN, 14, 0, true },
		{ "|=", TOKEN_OR_ASSIGN, 14, 0, true },
		{ "^=", TOKEN_XOR_ASSIGN, 14, 0, true },
		{ "&&", TOKEN_LOGICAL_AND, 11,0,  true },
		{ "||", TOKEN_LOGICAL_OR, 12,0,  true },
		{ ">>", TOKEN_SHR, 5, 0, true },
		{ "<<", TOKEN_SHL, 5, 0, true },

		{ ";", TOKEN_SEMICOLON },
		{ ",", TOKEN_COMMA, 15 },
		{ "(", TOKEN_PAREN_OPEN, 1 },
		{ ")", TOKEN_PAREN_CLOSE, 1 },
		{ "{", TOKEN_BRACKET_OPEN },
		{ "}", TOKEN_BRACKET_CLOSE },
		{ "[", TOKEN_BRACE_OPEN, 1, 0, true },
		{ "]", TOKEN_BRACE_CLOSE, 1, 0, true },
		{ "+", TOKEN_ADD, 4, 0, true },
		{ "-", TOKEN_SUB, 4, 0, true },
		{ "*", TOKEN_MUL, 3, 0, true },
		{ "%", TOKEN_MOD, 3, 0, true },
		{ "/", TOKEN_DIV, 3, 0, true },
		{ "&", TOKEN_AND, 8, 0, true },
		{ "|", TOKEN_OR, 10, 0, true },
		{ "^", TOKEN_XOR, 9, 0, true },
		{ "!", TOKEN_NOT, 2, 0, true },
		{ "=", TOKEN_ASSIGN, 14, 0, true },
		{ ">", TOKEN_GT, 6, 0, true },
		{ "<", TOKEN_LT, 6, 0, true },
	};

	inline std::unordered_map<std::string, token_info> static_tokens_map;

	namespace regex
	{
		static inline std::regex SINGLE_COMMENT("\\/{2}.*$"),
								 WORD("\\b[a-zA-Z]\\w*\\b"),
								 INT_LITERAL("(\\-)?[0-9]{1,20}((u|i)(8|16|32|64))?");
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
		case TOKEN_LOGICAL_AND:			return "TOKEN_LOGICAL_AND";
		case TOKEN_OR:					return "TOKEN_OR";
		case TOKEN_LOGICAL_OR:			return "TOKEN_LOGICAL_OR";
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

	inline std::string STRINGIFY_TYPE(Token id)
	{
		auto it = std::find_if(keywords_type.begin(), keywords_type.end(), [&](const auto& p) { return p.second == id; });
		return (it != keywords_type.end() ? it->first : "unknown_type");
	}

	inline std::string STRINGIFY_OPERATOR(Token id)
	{
		for (auto&& token : static_tokens)
			if (token.id == id)
				return token.value;

		return {};
	}

	class lexer
	{
	private:

		std::vector<token_info> tokens,
								eaten_tokens;

		std::vector<std::string> errors;

	public:

		lexer()  {}
		~lexer() {}

		bool parse(const std::string& filename);

		token_info eat();

		std::optional<token_info> eat_expect(Token expected_token);
		std::optional<token_info> eat_expect_keyword_declaration();

		void print_list();
		void print_errors();
		void push_and_pop_token(const token_info& token);
		void restore_last_eaten_token();

		template <typename... A>
		inline void add_error(const std::string& format, A... args)
		{
			errors.push_back(fmt(format, args...));
		}

		bool is_token_operator(const token_info& token);
		bool is_token_keyword(const token_info& token);
		bool is_token_keyword_type(const token_info& token);
		
		bool is_token_operator()						{ return is_token_operator(current()); }
		bool is_token_keyword()							{ return is_token_keyword(current()); }
		bool is_token_keyword_type()					{ return is_token_keyword_type(current()); }
		bool is_current(Token id)						{ return (current_token() == id); }
		bool is_next(Token id)							{ return (next_token() == id); }
		bool is(const token_info& token, Token id)		{ return (token.id == id); }
		bool eof()										{ return tokens.empty(); }

		token_info token(int i) const					{ return (tokens.empty() ? token_info { "eof", TOKEN_EOF } : tokens[i]); }
		token_info eaten_token(int i) const				{ return (eaten_tokens.empty() ? token_info { "eof", TOKEN_EOF } : *(eaten_tokens.rbegin() + i)); }

		token_info current() const						{ return (tokens.empty() ? token_info { "eof", TOKEN_EOF } : tokens.back()); }

		Token current_token() const						{ return (tokens.empty() ? TOKEN_EOF : tokens.back().id); }
		Token next_token() const						{ return (tokens.size() < 2 ? TOKEN_EOF : (tokens.rbegin() + 1)->id); }
		
		std::string current_value() const				{ return (tokens.empty() ? std::string {} : tokens.back().value); }

		const size_t get_tokens_count() const			{ return tokens.size(); }
	};
}