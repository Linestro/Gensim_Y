/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * LowerCall.cpp
 *
 *  Created on: 16 Nov 2015
 *      Author: harry
 */

#include "define.h"
#include "blockjit/block-compiler/lowering/x86/X86LoweringContext.h"
#include "blockjit/block-compiler/lowering/x86/X86Lowerers.h"
#include "blockjit/block-compiler/block-compiler.h"
#include "blockjit/translation-context.h"
#include "blockjit/block-compiler/lowering/x86/X86BlockjitABI.h"

using namespace captive::arch::jit::lowering::x86;
using namespace captive::shared;

bool LowerCall::Lower(const captive::shared::IRInstruction *&insn)
{
	const IROperand *rval = &insn->operands[0];
	const IROperand *target = &insn->operands[1];

	if(!target->is_func()) {
		UNEXPECTED;
	}

	const IRInstruction *prev_insn = insn-1;
	const IRInstruction *next_insn = insn+1;

	if (prev_insn) {
		if (prev_insn->type == IRInstruction::CALL && insn->count_operands() == prev_insn->count_operands()) {
			// Don't save the state, because the previous instruction was a call and it is already saved.
		} else {
			GetLoweringContext().emit_save_reg_state(insn->count_operands(), GetStackMap(), GetIsStackFixed());
		}
	} else {
		GetLoweringContext().emit_save_reg_state(insn->count_operands(), GetStackMap(), GetIsStackFixed());
	}

	//emit_save_reg_state();

	// DI, SI, DX, CX, R8, R9
	const X86Register *sysv_abi[] = { &REG_RDI, &REG_RSI, &REG_RDX, &REG_RCX, &REG_R8, &REG_R9 };

	// CPU State
	GetLoweringContext().load_state_field("thread_ptr", *sysv_abi[0]);

	for (int i = 2; i < insn->operands.size(); i++) {
		if (insn->operands[i].type != IROperand::NONE) {
			GetLoweringContext().encode_operand_function_argument(&insn->operands[i], *sysv_abi[i-1], GetStackMap());
		}
	}

	// Load the address of the target function into a temporary, and perform an indirect call.
	Encoder().mov(target->value, BLKJIT_RETURN(8));
	Encoder().call(BLKJIT_RETURN(8));

	if(rval->is_vreg()) {
		if(rval->is_alloc_reg()) {
			Encoder().mov(REG_RAX, GetLoweringContext().register_from_operand(rval));
		} else if(rval->is_alloc_stack()) {
			Encoder().mov(REG_RAX, GetLoweringContext().stack_from_operand(rval));
		} else {
			UNEXPECTED;
		}
	}

	if (next_insn) {
		if (next_insn->type == IRInstruction::CALL && insn->count_operands() == next_insn->count_operands()) {
			// Don't restore the state, because the next instruction is a call and it will use it.
		} else {
			GetLoweringContext().emit_restore_reg_state(GetIsStackFixed());
		}
	} else {
		GetLoweringContext().emit_restore_reg_state(GetIsStackFixed());
	}


	insn++;
	return true;
}

