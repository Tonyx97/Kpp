#pragma once

namespace kpp
{
	void critical_error();
	void parser_error(const char* str);

	template <typename... A>
	inline std::string fmt(const std::string& format, A... args)
	{
		std::string buff;
		buff.resize(1024);
		sprintf_s(buff.data(), buff.size(), format.c_str(), std::forward<A>(args)...);
		return buff;
	}

	template <typename... A>
	inline void parser_error(const std::string& format, A... args)
	{
		parser_error(fmt(format, args...).c_str());
	}

	inline class lexer* err_lexer = nullptr;
}