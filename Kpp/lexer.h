#pragma once

namespace kpp
{
	enum Token : int
	{
		TKN_ID,
		TKN_KEYWORD,
		TKN_INT_LITERAL,
		TKN_COMMENT,

		TKN_SHR_ASSIGN,
		TKN_SHL_ASSIGN,
		TKN_ADD_ASSIGN,
		TKN_INC,
		TKN_DEC,
		TKN_SUB_ASSIGN,
		TKN_MUL_ASSIGN,
		TKN_MOD_ASSIGN,
		TKN_DIV_ASSIGN,
		TKN_OR_ASSIGN,
		TKN_AND_ASSIGN,
		TKN_XOR_ASSIGN,
		TKN_GTE,
		TKN_LTE,

		TKN_COLON,
		TKN_SEMICOLON,
		TKN_COMMA,

		TKN_PAREN_OPEN,
		TKN_PAREN_CLOSE,
		TKN_BRACE_OPEN,
		TKN_BRACE_CLOSE,
		TKN_BRACKET_OPEN,
		TKN_BRACKET_CLOSE,
		TKN_ADD,
		TKN_SUB,
		TKN_MUL,
		TKN_MOD,
		TKN_DIV,
		TKN_SHR,
		TKN_SHL,
		TKN_AND,
		TKN_OR,
		TKN_XOR,
		TKN_NOT,
		TKN_ASSIGN,

		TKN_EQUAL,
		TKN_NOT_EQUAL,
		TKN_GT,
		TKN_LT,

		TKN_KW_VOID,
		TKN_KW_U8,
		TKN_KW_U16,
		TKN_KW_U32,
		TKN_KW_U64,
		TKN_KW_I8,
		TKN_KW_I16,
		TKN_KW_I32,
		TKN_KW_I64,
		TKN_KW_FOR,
		TKN_KW_WHILE,
		TKN_KW_IF,
		TKN_KW_ELSE,
		TKN_KW_BREAK,
		TKN_KW_CONTINUE,
		TKN_KW_RETURN,
		TKN_KW_EXTERN,

		TKN_EOF,
		TKN_NONE,
	};

	struct token_info
	{
		std::string value;
		Token id;

		template <typename T>
		T get()
		{
			switch (id)
			{
			case TKN_INT_LITERAL: return (int)std::atol(value.c_str());
			}

			return T {};
		}
	};

	inline std::unordered_map<std::string, token_info> keywords =
	{
		{ "for", { "for", TKN_KW_FOR } },
		{ "while", { "while", TKN_KW_WHILE } },
		{ "if", { "if", TKN_KW_IF } },
		{ "else", { "else", TKN_KW_ELSE } },
		{ "break", { "break", TKN_KW_BREAK } },
		{ "continue", { "continue", TKN_KW_CONTINUE } },
		{ "return", { "return", TKN_KW_RETURN } },
		{ "extern", { "extern", TKN_KW_EXTERN } },
	};

	inline std::unordered_map<std::string, token_info> keywords_decl =
	{
		{ "void", { "void", TKN_KW_VOID } },
		{ "u8", { "u8", TKN_KW_U8 } },
		{ "u16", { "u16", TKN_KW_U16 } },
		{ "u32", { "u32", TKN_KW_U32 } },
		{ "u64", { "u64", TKN_KW_U64 } },
		{ "i8", { "i8", TKN_KW_I8 } },
		{ "i16", { "i16", TKN_KW_I16 } },
		{ "i32", { "i32", TKN_KW_I32 } },
		{ "i64", { "i64", TKN_KW_I64 } },
	};

	inline token_info static_tokens[] =
	{
		{ ">>=", TKN_SHR_ASSIGN },
		{ "<<=", TKN_SHL_ASSIGN },

		{ "==", TKN_EQUAL },
		{ "!=", TKN_NOT_EQUAL },
		{ ">=", TKN_GTE },
		{ "<=", TKN_LTE },
		{ "+=", TKN_ADD_ASSIGN },
		{ "-=", TKN_SUB_ASSIGN },
		{ "++", TKN_INC },
		{ "--", TKN_DEC },
		{ "*=", TKN_MUL_ASSIGN },
		{ "%=", TKN_MOD_ASSIGN },
		{ "/=", TKN_DIV_ASSIGN },
		{ "&=", TKN_AND_ASSIGN },
		{ "|=", TKN_OR_ASSIGN },
		{ "^=", TKN_XOR_ASSIGN },

		{ ";", TKN_SEMICOLON },
		{ ",", TKN_COMMA },
		{ "(", TKN_PAREN_OPEN },
		{ ")", TKN_PAREN_CLOSE },
		{ "{", TKN_BRACKET_OPEN },
		{ "}", TKN_BRACKET_CLOSE },
		{ "[", TKN_BRACE_OPEN },
		{ "]", TKN_BRACE_CLOSE },
		{ "+", TKN_ADD },
		{ "-", TKN_SUB },
		{ "*", TKN_MUL },
		{ "%", TKN_MOD },
		{ "/", TKN_DIV },
		{ ">>", TKN_SHR },
		{ "<<", TKN_SHL },
		{ "&", TKN_AND },
		{ "|", TKN_OR },
		{ "^", TKN_XOR },
		{ "!", TKN_NOT },
		{ "=", TKN_ASSIGN },
		{ ">", TKN_GT },
		{ "<", TKN_LT },
	};

	namespace regex
	{
		static inline std::regex SINGLE_COMMENT("\\/{2}.*$"),
								 WORD("\\b[a-zA-Z]\\w*\\b"),
								 INT_LITERAL("\\b[0-9]{1,20}((u|i)(8|16|32|64))?\\b");
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

		bool is_token_keyword(const token_info& token);
		bool is_token_keyword_decl(const token_info& token);

		void push_and_pop_token(const token_info& token);
		
		bool eof()								{ return tokens.empty(); }

		token_info token(int i) const			{ return tokens[i]; }

		token_info current() const
		{
			if (tokens.empty())
				return { "eof", TKN_EOF };
			
			return tokens.back();
		}

		Token current_token() const
		{
			if (tokens.empty())
				return TKN_EOF;

			return tokens.back().id;
		}

		const std::string& current_value() const
		{
			if (tokens.empty())
				return {};

			return tokens.back().value;
		}

		const size_t get_tokens_count() const	{ return tokens.size(); }
	};
}