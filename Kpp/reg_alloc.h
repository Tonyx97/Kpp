#pragma once

namespace kpp
{
	class reg_alloc
	{
	private:

		ssa_gen& ssa;

	public:

		reg_alloc(ssa_gen& ssa) : ssa(ssa) {}

		bool allocate_all();
	};
}