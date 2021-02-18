#include <defs.h>

#include "err_handler.h"

#include "lexer.h"

using namespace kpp;

bool lexer::parse(const std::string& filename)
{
	// open the input file

	auto file = std::ifstream(filename);
	if (!file)
		return false;

	// parse each line of the input file

	std::string line;

	auto remove_spaces = [&]()
	{
		if (line.empty())
			return;

		if (line.find_first_not_of("\t ") != std::string::npos)
			for (auto curr_char = line.at(0); (curr_char == ' ' || curr_char == '\t'); curr_char = line.at(0))
				line = line.substr(line.find_first_not_of("\t "), std::string::npos);
		else line.clear();
	};

	auto strip_line = [&](size_t off, size_t count)
	{
		return (line = line.erase(off, count)).empty();
	};
	
	auto next = [&](size_t off)
	{
		line = line.substr(off, std::string::npos);
		remove_spaces();
	};

	int line_num = 1;

	bool multiline_comment = false;

	while (std::getline(file, line))
	{
		// remove spaces and tabs at the beginning of the line

		remove_spaces();
		
		while (!line.empty())
		{
			auto multi_comment_begin_pos = line.find("/*"),
				 multi_comment_end_pos = line.find("*/");

			if (multi_comment_begin_pos != std::string::npos &&
				multi_comment_end_pos != std::string::npos)
			{
				if (strip_line(multi_comment_begin_pos, multi_comment_end_pos - multi_comment_begin_pos + 2))
					break;
			}
			else
			{
				if (multi_comment_end_pos != std::string::npos)
				{
					multiline_comment = false;

					if (strip_line(0, multi_comment_end_pos + 2))
						break;
				}
				else if (multi_comment_begin_pos == 0)
				{
					multiline_comment = true;
					break;
				}
			}

			if (multiline_comment)
				break;

			std::smatch sm;

			if (std::regex_search(line, sm, regex::SINGLE_COMMENT))
				break;

			token_info curr_token;
			
			bool valid = false;

			const std::string token_small = line.substr(0, 1),
							  token_medium = line.substr(0, 2),
							  token_big = line.substr(0, 3);

			for (auto&& [token, id] : static_tokens)
			{
				switch (token.length())
				{
				case 1: valid = !token.compare(token_small);  break;
				case 2: valid = !token.compare(token_medium); break;
				case 3: valid = !token.compare(token_big);	  break;
				}

				if (valid)
				{
					curr_token.value = token;
					curr_token.id = id;
					break;
				}
			}
		
			if (!valid)
			{
				auto check_token_regex = [&](const std::regex& rgx, Token token_type)
				{
					if (valid)
						return false;

					if (std::regex_search(line, sm, rgx))
					{
						if (auto token_found = sm.str(); valid = (line.find(token_found) == 0))
						{
							curr_token.value = token_found;

							switch (token_type)
							{
							case TKN_ID:
							{
								if (auto it = keywords.find(token_found); it != keywords.end())
									curr_token.id = it->second.id;
								else if (auto it_decl = keywords_decl.find(token_found); it_decl != keywords_decl.end())
									curr_token.id = it_decl->second.id;
								else curr_token.id = token_type;

								break;
							}
							case TKN_INT_LITERAL:
							{
								curr_token.id = token_type;
								break;
							}
							}

							return true;
						}
					}

					return false;
				};

				if (!check_token_regex(regex::WORD, TKN_ID))
					check_token_regex(regex::INT_LITERAL, TKN_INT_LITERAL);
			}

			if (valid)
			{
				tokens.push_back(curr_token);

				next(curr_token.value.length());
			}
			else
			{
				std::string invalid_token = line;
				
				if (auto next_space = invalid_token.find_first_of("\t "); next_space != std::string::npos)
				{
					invalid_token = invalid_token.substr(0, next_space);
					next(next_space);
				}
				else next(line.length());
				
				lexer_error("Lexical error '%s' in %s:%i", invalid_token.c_str(), filename.c_str(), line_num);
			}
		}

		++line_num;
	}

	for (auto&& token : tokens)
		printf_s("'%s' %i\n", token.value.c_str(), token.id);

	std::reverse(tokens.begin(), tokens.end());

	return true;
}

std::optional<token_info> lexer::eat_expect(Token expected_token)
{
	if (eof())
		return {};

	if (auto curr = current(); curr.id != expected_token)
	{
		printf_s("Unexpected token '%s'\n", curr.value.c_str());
		return {};
	}
	else
	{
		push_and_pop_token(curr);

		return curr;
	}
}

std::optional<token_info> lexer::eat_expect_keyword_declaration()
{
	if (eof())
		return {};

	if (auto curr = current(); !is_token_keyword_decl(curr))
	{
		printf_s("Unexpected token '%s'\n", curr.value.c_str());
		return {};
	}
	else
	{
		push_and_pop_token(curr);

		return curr;
	}
}

token_info lexer::eat()
{
	if (eof())
		return {};

	auto curr = current();

	push_and_pop_token(curr);

	return curr;
}

bool lexer::is_token_keyword(const token_info& token)
{
	return (keywords.find(token.value) != keywords.end());
}

bool lexer::is_token_keyword_decl(const token_info& token)
{
	return (keywords_decl.find(token.value) != keywords_decl.end());
}

void lexer::push_and_pop_token(const token_info& token)
{
	eaten_tokens.push_back(token);
	tokens.pop_back();
}