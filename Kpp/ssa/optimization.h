#pragma once

namespace kpp
{
	class optimization
	{
	private:

		ir_gen& ir;

		std::vector<ir::Instruction*> unused_instructions;

	public:

		optimization(ir_gen& ir) : ir(ir)			{}

		bool optimize();
		bool optimize_alias(ir::Alias* i);
		bool optimize_load(ir::Load* i);

		void remove_instruction(ir::Instruction* i) { unused_instructions.push_back(i); }
	};
}