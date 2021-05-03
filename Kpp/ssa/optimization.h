#pragma once

namespace kpp
{
	class optimization
	{
	private:

		ir_gen& ir;

		std::vector<ir::Instruction*> unused_instructions;
		std::vector<std::pair<ir::Instruction*, ir::Instruction*>> replaceable_instructions;

	public:

		optimization(ir_gen& ir) : ir(ir)										{}

		bool optimize();
		bool optimize_alias(ir::Alias* i);
		bool optimize_load(ir::Load* i);
		bool optimize_store(ir::Store* i);

		void remove_instruction(ir::Instruction* i)								{ unused_instructions.push_back(i); }
		void replace_instruction(ir::Instruction* from, ir::Instruction* to)	{ replaceable_instructions.push_back({ from, to }); }
	};
}