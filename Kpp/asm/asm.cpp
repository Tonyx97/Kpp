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
	x64::instruction_list instructions;

	const auto& prototypes = ir.get_ir_info().prototypes;
	
	// generate main asm code

	for (auto prototype : prototypes)
	{
		int offset = 0;

		for (auto b : prototype->blocks)
		{
			b->set_instructions_offset(offset);

			for (auto i : b->items)
			{
				if (auto binary_op = rtti::safe_cast<ir::BinaryOp>(i))
					b->add_instructions(generate_from_binary_op(binary_op));
				else if (auto store = rtti::safe_cast<ir::Store>(i))
					b->add_instructions(generate_from_store(store));
				else if (auto value_int = rtti::safe_cast<ir::ValueInt>(i))
					b->add_instructions(generate_from_value_int(value_int));
				else if (auto alias = rtti::safe_cast<ir::Alias>(i))
					b->add_instructions(generate_from_alias(alias));
				else if (auto branch = rtti::safe_cast<ir::Branch>(i))
				{
					if (!(branch->unused = (branch->target->get_last_ref() == b)))
						b->add_instructions(generate_from_any_branch(i));
				}
				else if (i->type == ir::INS_BRANCH_COND || i->type == ir::INS_RETURN)
					b->add_instructions(generate_from_any_branch(i));
			}

			offset += b->get_total_bytes();

			const auto& block_instructions = b->get_instructions();

			instructions.insert(instructions.end(), block_instructions.begin(), block_instructions.end());
		}
	}

	// fix jumps

	for (auto prototype : prototypes)
	{
		for (auto b : prototype->blocks)
		{
			auto jmp_item = b->get_jump_item();

			ir::Block* jmp_target = nullptr;

			if (auto branch = rtti::safe_cast<ir::Branch>(jmp_item))
			{
				if (!branch->unused)
					jmp_target = branch->target;
			}
			else if (auto bcond = rtti::safe_cast<ir::BranchCond>(jmp_item))
				jmp_target = bcond->target_if_false;

			if (jmp_target && jmp_target)
			{
				auto jmp_ins = b->get_jump_instruction();

				jmp_ins->set_imm(jmp_target->get_instructions_offset() - (b->get_total_bytes() + b->get_instructions_offset()), 1);
				jmp_ins->regenerate();
			}
		}
	}

	// copy bytes to clipboard

	std::stringstream ss;

	for (auto i : instructions)
	{
		x64::print_instruction(i);

		for (auto b : i->all_bytes)
			ss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(b) << " ";
	}

	auto str = ss.str();
	
	const size_t len = str.length() + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), str.data(), len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();

	return true;
}

std::vector<x64::Instruction*> asm_gen::generate_from_binary_op(ir::BinaryOp* i)
{
	return x64::generate_binary_op_instruction(i);
}

std::vector<x64::Instruction*> asm_gen::generate_from_store(ir::Store* i)
{
	return x64::generate_memory_op(i);
}

std::vector<x64::Instruction*> asm_gen::generate_from_value_int(ir::ValueInt* i)
{
	return x64::generate_memory_op(i);
}

std::vector<x64::Instruction*> asm_gen::generate_from_alias(ir::Alias* i)
{
	return x64::generate_memory_op(i);
}

std::vector<x64::Instruction*> asm_gen::generate_from_any_branch(ir::Instruction* i)
{
	return x64::generate_control_instruction(i);
}