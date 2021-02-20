#pragma once

#ifndef DEBUG_H
#define DEBUG_H

#include <Windows.h>

#include "defs.h"

enum eColor : unsigned short
{
	C_BLACK = 0x0,
	C_DARK_BLUE = 0x1,
	C_DARK_GREEN = 0x2,
	C_DARK_CYAN = 0x3,
	C_DARK_RED = 0x4,
	C_DARK_PURPLE = 0x5,
	C_DARK_YELLOW = 0x6,
	C_GREY = 0x7,
	C_DARK_GREY = 0x8,
	C_BLUE = 0x9,
	C_GREEN = 0xA,
	C_CYAN = 0xB,
	C_RED = 0xC,
	C_PURPLE = 0xD,
	C_YELLOW = 0xE,
	C_WHITE = 0xF,
};

namespace dbg
{
	static constexpr auto SINGLE_TEXT_MAX_LENGTH = 0x100;
	static constexpr auto SPACES_PER_TAB = 2;

	enum eTextSection
	{
		HEADER,
		FOOTER,
	};

	class basic_buffer
	{
	private:

		char* data = nullptr;

	public:

		basic_buffer(size_t max_len)	{ data = new char[max_len](); }
		~basic_buffer()					{ delete[] data; }
		
		char* get()						{ return data; }

		friend std::ostream& operator << (std::ostream& os, const basic_buffer& buffer)
		{
			std::cout << buffer.data;
			return os;
		}
	};

	template <typename... A>
	std::string format(const std::string& txt, A... args)
	{
		basic_buffer buffer(SINGLE_TEXT_MAX_LENGTH);
		sprintf_s(buffer.get(), SINGLE_TEXT_MAX_LENGTH, txt.c_str(), std::forward<A>(args)...);
		return std::string(buffer.get());
	}

	struct color
	{
		color(uint16_t value)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), value); }
		~color()				{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF); }
	};

	class text
	{
	private:

		std::string data;

		uint16_t color_id;

		int alignment = 0;

		bool nl;

	public:

		text(uint16_t color_id, const std::string& data, bool nl, int alignment = 0) : data(data), color_id(color_id), nl(nl), alignment(alignment) {}

		void print() { std::cout << *this; }

		friend std::ostream& operator << (std::ostream& os, const text& t)
		{
			auto align = [&](int width)
			{
				std::cout.setf(std::ios_base::left, std::ios_base::adjustfield);
				std::cout.fill(' ');
				std::cout.width(width);
			};

			if (t.alignment > 0)
				align(t.alignment);

			auto new_line = (t.nl ? "\n" : "");

			color c(t.color_id);

			std::cout << t.data << new_line;

			return os;
		}
	};

	template <typename... A>
	static inline text make_text(uint16_t color, const std::string& txt, A&&... args)
	{
		return dbg::text(color, dbg::format(txt, args...), false);
	}

	template <typename... A>
	static inline text make_text_nl(uint16_t color, const std::string& txt, A&&... args)
	{
		return dbg::text(color, dbg::format(txt, args...), true);
	}

	template <typename... A>
	static inline text make_text_align(uint16_t color, const std::string& txt, int alignment, A&&... args)
	{
		return dbg::text(color, dbg::format(txt, args...), false, alignment);
	}

	template <typename... A>
	static inline text make_text_with_tabs(uint16_t color, const std::string& txt, int tabs, A&&... args)
	{
		std::string tabs_str(tabs * SPACES_PER_TAB, 0);

		std::generate(tabs_str.begin(), tabs_str.end(), [] { return ' '; });

		return dbg::text(color, tabs_str + dbg::format(txt, args...), false, 0);
	}

	template <typename... A>
	static inline void print(uint16_t color_id, const std::string& txt, A&&... args)
	{
		color c(color_id);
		std::cout << format(txt, args...);
	}

	template <typename... A>
	static inline void println(uint16_t color_id, const std::string& txt, A&&... args)
	{
		color c(color_id);
		std::cout << format(txt, args...) << std::endl;
	}

	template <typename Tx, typename T, typename F>
	static inline void print_vec(eColor color, const std::vector<T>& vec, const std::string& separator, const F& fn)
	{
		for (int i = 0; i < vec.size() - 1; ++i)
			dbg::make_text(color, "%s%s", fn(static_cast<Tx*>(vec[i])).c_str(), separator.c_str()).print();

		dbg::make_text(color, "%s", fn(static_cast<Tx*>(vec.back())).c_str()).print();
	}

	struct TimeProfiling
	{
		std::chrono::high_resolution_clock::time_point m_start;
		uint64_t cycles = 0;
		std::string name;

		TimeProfiling(const std::string& name) : name(name)
		{
			m_start = std::chrono::high_resolution_clock::now();
			cycles = __rdtsc();
		}

		~TimeProfiling()
		{
			const auto cycles_passed = __rdtsc() - cycles;
			const auto time_passed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_start).count();
			dbg::make_text_nl(C_YELLOW, "%s: %.3f ms | %i mcs | %i cycles", name.c_str(), static_cast<double>(time_passed) / 1000.f, time_passed, cycles_passed).print();
		}
	};
};

#define PROFILE(x)						dbg::TimeProfiling p(x)
#define EMPTY_NEW_LINE					C_WHITE, "\n"
#define PRINT_NNL(x, y, ...)			dbg::make_text(x, y, __VA_ARGS__).print()
#define PRINT(x, y, ...)				dbg::make_text_nl(x, y, __VA_ARGS__).print()
#define PRINT_ALIGN(x, y, z, ...)		dbg::make_text_align(x, z, y, __VA_ARGS__).print()
#define PRINT_ALIGN_NL(x, y, z, ...)	dbg::make_text_align(x, z, y, __VA_ARGS__).print(); dbg::make_text(EMPTY_NEW_LINE).print()
#define PRINT_TABS(x, y, z, ...)		dbg::make_text_with_tabs(x, z, y, __VA_ARGS__).print()
#define PRINT_TABS_NL(x, y, z, ...)		dbg::make_text_with_tabs(x, z, y, __VA_ARGS__).print(); dbg::make_text(EMPTY_NEW_LINE).print()
#define PRINT_NL						dbg::make_text(EMPTY_NEW_LINE).print()

#endif