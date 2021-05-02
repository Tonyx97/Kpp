#include <defs.h>

#include <ir/ir.h>

#include "label.h"
#include "registers.h"

using namespace kpp;

int g_current_label_id = 0;

x64::Instruction* x64::gen_label()
{
	return new Label(g_current_label_id++);
}

void x64::reset_label_count()
{
	g_current_label_id = 0;
}