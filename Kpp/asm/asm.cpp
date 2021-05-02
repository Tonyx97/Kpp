#include <defs.h>

#include <ir/ir.h>
#include <ssa/ssa.h>

#include "asm.h"

using namespace kpp;

asm_gen::~asm_gen()
{

}

bool asm_gen::init()
{
	const auto& prototypes = ir.get_ir_info().prototypes;

	// generate main asm code

	uint32_t prototype_offset = 0;

	for (auto prototype : prototypes)
	{
		// create all block labels first

		for (auto b : prototype->blocks)
		{
			b->set_label(x64::gen_label());
			PRINT(C_RED, "'%s' has 0x%llx", b->name.c_str(), b->get_label());
		}

		// generate x64 asm code

		for (auto b : prototype->blocks)
		{
			instructions.add_instruction(b->get_label());

			for (auto i : b->items)
			{
				if (auto binary_op = rtti::safe_cast<ir::BinaryOp>(i))
					instructions.add_instructions(generate_from_binary_op(binary_op));
				else if (auto store = rtti::safe_cast<ir::Store>(i))
					instructions.add_instructions(generate_from_store(store));
				else if (auto value_int = rtti::safe_cast<ir::ValueInt>(i))
					instructions.add_instructions(generate_from_value_int(value_int));
				else if (auto alias = rtti::safe_cast<ir::Alias>(i))
					instructions.add_instructions(generate_from_alias(alias));
				else if (auto load = rtti::safe_cast<ir::Load>(i))
					instructions.add_instructions(generate_from_load(load));
				else if (auto call = rtti::safe_cast<ir::Call>(i))
					instructions.add_instructions(generate_from_call(call));
				else if (auto branch = rtti::safe_cast<ir::Branch>(i))
				{
					if (!(branch->unused = (branch->target->get_last_ref() == b)))
						instructions.add_instructions(generate_from_any_branch(i));
				}
				else if (i->type == ir::INS_BRANCH_COND || i->type == ir::INS_RETURN)
					instructions.add_instructions(generate_from_any_branch(i));
			}

			if (auto jmp_target = b->get_asm_target())
				if (auto jmp_ins = instructions.last())
				{
					jmp_ins->set_target_label(jmp_target->get_label(), b->reverse_postorder_index < jmp_target->reverse_postorder_index);
					PRINT(C_BLUE, "'%s' jumps to label 0x%llx", b->name.c_str(), jmp_target->get_label());
				}
		}

		x64::reset_label_count();

		prototype->set_address(prototype_offset);

		prototype_offset += instructions.calc_total_bytes();
	}

	// create instructions linking

	x64::Instruction* last_instruction = nullptr;

	for (auto it = instructions.begin(); it != instructions.end();)
	{
		auto ie = *it;

		if (last_instruction)
			ie->prev = last_instruction;

		last_instruction = ie;

		if (auto next = ++it; next != instructions.end())
			ie->next = *next;
	}

	// fix jumps, calls and relative addressing

	if (!fix_jumps())
		return false;

	if (!fix_calls())
		return false;

	// copy bytes to clipboard

	x64::bytes_list bytes;

	for (auto ie : instructions)
		bytes.insert(bytes.end(), ie->bytes.begin(), ie->bytes.end());

	std::stringstream ss;

	for (auto b : bytes)
		ss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(b) << " ";

	util::winapi::copy_to_clipboard(ss.str());

	// execute test function

	auto fn_bytes = new uint8_t[bytes.size()]();

	if (DWORD old; VirtualProtect(fn_bytes, bytes.size(), PAGE_EXECUTE_READWRITE, &old))
	{
		memcpy(fn_bytes, bytes.data(), bytes.size());

		auto result = reinterpret_cast<int(__fastcall*)()>(fn_bytes + 8)();

		PRINT(C_GREEN, "[0x%llx] Execution result: %i", fn_bytes, result);
	}

	delete[] fn_bytes;

	return true;
}

bool asm_gen::fix_jumps()
{
	bool expanded = false;
	
	do {
		for (auto ie : instructions)
			if (auto target_label = ie->target_label)
			{
				int offset = 0;

				const bool look_up = ie->target_label_look_up;

				auto curr = look_up ? ie->prev : ie->next;

				while (curr)
				{
					offset += curr->len;

					if (rtti::safe_cast<x64::Label>(curr) == target_label)
						break;

					curr = look_up ? curr->prev : curr->next;
				}

				if (expanded = x64::fix_jump(ie, offset))
					break;
			}
	} while (expanded);

	return true;
}

bool asm_gen::fix_calls()
{
	int offset = 0;
	
	for (auto ie : instructions)
	{
		if (auto call = rtti::safe_cast<ir::Call>(ie->owner))
		{
			ie->set_imm(call->prototype->address - offset - 5, 4);
			ie->regenerate();
		}

		offset += ie->len;
	}

	return true;
}

x64::instruction_list asm_gen::generate_from_binary_op(ir::BinaryOp* i)
{
	return x64::generate_binary_op_instruction(i);
}

x64::instruction_list asm_gen::generate_from_store(ir::Store* i)
{
	return x64::generate_memory_op(i);
}

x64::instruction_list asm_gen::generate_from_value_int(ir::ValueInt* i)
{
	return x64::generate_memory_op(i);
}

x64::instruction_list asm_gen::generate_from_alias(ir::Alias* i)
{
	return x64::generate_memory_op(i);
}

x64::instruction_list asm_gen::generate_from_load(ir::Load* i)
{
	return x64::generate_memory_op(i);
}

x64::instruction_list asm_gen::generate_from_call(ir::Call* i)
{
	return x64::generate_control_instruction(i);
}

x64::instruction_list asm_gen::generate_from_any_branch(ir::Instruction* i)
{
	return x64::generate_control_instruction(i);
}