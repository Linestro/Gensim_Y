/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * LowerLoadPC.cpp
 *
 *  Created on: 16 Nov 2015
 *      Author: harry
 */

#include "blockjit/block-compiler/lowering/x86/X86LoweringContext.h"
#include "blockjit/block-compiler/lowering/x86/X86Lowerers.h"
#include "blockjit/block-compiler/block-compiler.h"
#include "blockjit/translation-context.h"
#include "blockjit/block-compiler/lowering/x86/X86BlockjitABI.h"

using namespace captive::arch::jit::lowering::x86;
using namespace captive::shared;

bool LowerLoadPC::Lower(const captive::shared::IRInstruction *&insn)
{
	const IROperand *target = &insn->operands[0];

	assert(target->is_vreg());

	const auto &rfd = GetLoweringContext().GetArchDescriptor().GetRegisterFileDescriptor();
	uint32_t pc_offset = rfd.GetTaggedEntry("PC").GetOffset();

	if (target->is_alloc_reg()) {
		// TODO: FIXME: XXX: HACK HACK HACK
		Encoder().mov(X86Memory::get(BLKJIT_REGSTATE_REG, pc_offset), GetLoweringContext().register_from_operand(target));
	} else if(target->is_alloc_stack()) {
		Encoder().mov(X86Memory::get(BLKJIT_REGSTATE_REG, pc_offset), BLKJIT_TEMPS_0(4));
		Encoder().mov(BLKJIT_TEMPS_0(4), GetLoweringContext().stack_from_operand(target));
	}

	insn++;
	return true;
}
