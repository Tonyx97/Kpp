#include <defs.h>

#include <ir/ir.h>
#include <ssa/ssa.h>

#include "x64/registers.h"

#include "asm.h"

using namespace kpp;

asm_gen::~asm_gen()
{

}

uint8_t* g_fn_bytes = nullptr;

bool asm_gen::init()
{
	const auto& prototypes = ir.get_ir_info().prototypes;

	// generate main asm code

	uint32_t prototype_offset = 0;

	for (auto prototype : prototypes)
	{
		curr_prototype = prototype;

		// create all block labels first

		for (auto b : prototype->blocks)
		{
			b->set_label(x64::gen_label());
			PRINT(C_RED, "'%s' has 0x%llx", b->name.c_str(), b->get_label());
		}

		// push all used registers

		if (prototype->is_callee())
			for (auto r : prototype->used_regs)
				instructions.add_instructions(x64::gen_push(r));

		// generate x64 asm code

		for (auto b : prototype->blocks)
		{
			instructions.add_instruction(b->get_label());

			for (auto i : b->items)
			{
				if (i->unused)
					continue;

				switch (i->type)
				{
				case ir::INS_BINARY_OP:
					instructions.add_instructions(x64::generate_binary_op_instruction(rtti::safe_cast<ir::BinaryOp>(i)));
					break;
				case ir::INS_VALUE_INT:
				case ir::INS_ALIAS:
				case ir::INS_LOAD:
				case ir::INS_STORE:
					instructions.add_instructions(x64::generate_memory_op(i));
					break;
				case ir::INS_BRANCH_COND:
				case ir::INS_RETURN:
					instructions.add_instructions(x64::generate_control_instruction(i));
					break;
				case ir::INS_CALL:
				{
					auto call = rtti::safe_cast<ir::Call>(i);
					instructions.add_instructions(call->built_in ? x64::generate_built_in_fn(call) : x64::generate_control_instruction(call));
					break;
				}
				case ir::INS_BRANCH:
				{
					auto branch = rtti::safe_cast<ir::Branch>(i);

					if (!(branch->unused = (branch->target->get_last_ref() == b)))
						instructions.add_instructions(x64::generate_control_instruction(i));

					break;
				}
				}
			}

			bool jump_reversed = false;

			if (auto jmp_target = b->get_asm_target(jump_reversed))
				if (auto jmp_ins = instructions.last())
				{
					jmp_ins->jump_reversed = jump_reversed;
					jmp_ins->set_target_label(jmp_target->get_label(), b->reverse_postorder_index < jmp_target->reverse_postorder_index);

					PRINT(C_BLUE, "'%s' jumps to label 0x%llx", b->name.c_str(), jmp_target->get_label());
				}
		}

		prototype->set_address(prototype_offset);

		int last_byte_offset = instructions.calc_total_bytes();
		int aligned_byte_offset = (last_byte_offset + 3) & ~3;

		if (aligned_byte_offset > last_byte_offset)
			instructions.add_instructions(x64::gen_fn_padding(aligned_byte_offset - last_byte_offset));

		prototype_offset = aligned_byte_offset;

		x64::reset_label_count();
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

	g_fn_bytes = new uint8_t[bytes.size()]();

	if (DWORD old; VirtualProtect(g_fn_bytes, bytes.size(), PAGE_EXECUTE_READWRITE, &old))
	{
		memcpy(g_fn_bytes, bytes.data(), bytes.size());

		//auto result = reinterpret_cast<int(__fastcall*)()>(g_fn_bytes + prototypes[0]->address)();
		auto result = reinterpret_cast<int(__fastcall*)()>(g_fn_bytes + prototypes[3]->address)();

		PRINT(C_GREEN, "[0x%llx] Execution result: %i", g_fn_bytes, result);
	}

	delete[] g_fn_bytes;

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
		if (auto call = rtti::safe_cast<ir::Call>(ie->owner); call && !call->built_in)
		{
			ie->set_imm(call->prototype->address - offset - 5, 4);
			ie->regenerate();
		}

		offset += ie->len;
	}

	return true;
}