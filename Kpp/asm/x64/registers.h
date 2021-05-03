#pragma once

namespace kpp
{
	enum reg_id
	{
		RAX,
		RCX,
		RDX,
		RBX,
		RSP,
		RBP,
		RSI,
		RDI,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		MAX_REGISTERS,
	};

	inline std::string STRINGIFY_REGISTER(reg_id name)
	{
		switch (name)
		{
		case RAX:	return "rax";
		case RCX:	return "rcx";
		case RDX:	return "rdx";
		case RBX:	return "rbx";
		case RSP:	return "rsp";
		case RBP:	return "rbp";
		case RSI:	return "rsi";
		case RDI:	return "rdi";
		case R8:	return "r8";
		case R9:	return "r9";
		case R10:	return "r10";
		case R11:	return "r11";
		case R12:	return "r12";
		case R13:	return "r13";
		case R14:	return "r14";
		case R15:	return "r15";
		default:	return "unknown";
		}
	}

	struct reg
	{
		reg_id id;

		bool general,
			 volatil;
	};

	struct reg_set_order
	{
		template <typename T> bool operator() (const T& x, const T& y) const { return (x->id < y->id); }
	};

	namespace x64
	{
		using register_fn = void(reg*);

		void init_registers();
		void destroy_registers();
		void for_each_register(const std::function<register_fn>& fn);

		reg* get_reg(reg_id id);
	}
}