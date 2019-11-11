/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */


#include "blockjit/block-compiler/lowering/x86/X86LoweringContext.h"
#include "blockjit/block-compiler/lowering/x86/X86Lowerers.h"
#include "blockjit/block-compiler/block-compiler.h"
#include "blockjit/translation-context.h"
#include "blockjit/block-compiler/lowering/x86/X86BlockjitABI.h"
#include "util/LogContext.h"

UseLogContext(LogBlockJit)

using namespace captive::arch::jit::lowering::x86;
using namespace captive::shared;

bool LowerVSubI::Lower(const captive::shared::IRInstruction*& insn)
{
	const IROperand &width = insn->operands[0]; // VECTOR width, i.e. number of elements
	const IROperand &lhs = insn->operands[1];
	const IROperand &rhs = insn->operands[2];
	const IROperand &dest = insn->operands[3];

	// assume 2x32 for now
	assert(width.value == 2);
	assert(lhs.size == 8);

	//TODO: make stack based accesses more efficient (movq from memory)
	if(lhs.is_alloc_reg())
		Encoder().movq(GetLoweringContext().register_from_operand(&lhs), BLKJIT_FP_1);
	else {
		Encoder().mov(GetLoweringContext().stack_from_operand(&lhs), BLKJIT_TEMPS_1(lhs.size));
		Encoder().movq(BLKJIT_TEMPS_0(lhs.size), BLKJIT_FP_1);
	}

	if(rhs.is_alloc_reg())
		Encoder().movq(GetLoweringContext().register_from_operand(&rhs), BLKJIT_FP_0);
	else {
		Encoder().mov(GetLoweringContext().stack_from_operand(&rhs), BLKJIT_TEMPS_0(rhs.size));
		Encoder().movq(BLKJIT_TEMPS_0(lhs.size), BLKJIT_FP_0);
	}

	// emit instruction based on ELEMENT size (total vector size / number of elements)
	switch(lhs.size / width.value) {
		case 4:
			Encoder().psubd(BLKJIT_FP_0, BLKJIT_FP_1);
			break;
		case 8:
			Encoder().psubq(BLKJIT_FP_0, BLKJIT_FP_1);
			break;
		default:
			LC_ERROR(LogBlockJit) << "Cannot lower an instruction with vector size " << (uint32_t)lhs.size << " and element count " << (uint32_t)width.value;
			assert(false);
	}

	if(dest.is_alloc_reg())
		Encoder().movq(BLKJIT_FP_1, GetLoweringContext().register_from_operand(&dest));
	else  {
		Encoder().movq(BLKJIT_FP_1, BLKJIT_TEMPS_0(dest.size));
		Encoder().mov(BLKJIT_TEMPS_0(dest.size), GetLoweringContext().stack_from_operand(&dest));
	}

	insn++;
	return true;
}

bool LowerVSubF::Lower(const captive::shared::IRInstruction*& insn)
{
	assert(false);
	insn++;
	return true;
}
