#pragma once

namespace kpp
{
	class reg_alloc
	{
	private:

		ir::IR& ir;

	public:

		reg_alloc(ir::IR& ir) : ir(ir) {}

		bool allocate_all();
		bool simulate_control_flow_internal(ir::Block* curr_block, const std::function<void(ir::Instruction*)>& fn);
		bool simulate_control_flow_global(ir::Prototype* prototype, ir::Block* curr_block, const std::function<void(ir::Instruction*)>& fn);
	};
}