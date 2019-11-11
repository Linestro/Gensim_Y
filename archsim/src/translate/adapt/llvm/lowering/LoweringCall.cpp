/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "translate/adapt/BlockJITAdaptorLowering.h"
#include "translate/adapt/BlockJITAdaptorLoweringContext.h"

#include <llvm/IR/Function.h>

using namespace archsim::translate::adapt;


bool BlockJITCALLLowering::Lower(const captive::shared::IRInstruction*& insn)
{
	const auto &rval = insn->operands[0];
	const auto &target = insn->operands[1];

	std::vector<llvm::Value*> arg_values;
	std::vector<llvm::Type*> arg_types;

	auto thread_ptr = GetContext().GetThreadPtr();
	auto thread_ptr_type = thread_ptr->getType();

	arg_values.push_back(thread_ptr);
	arg_types.push_back(thread_ptr_type);

	for(int i = 2; i < insn->operands.size(); ++i) {
		if(insn->operands[i].type == IROperand::NONE) {
			break;
		}

		arg_values.push_back(GetValueFor(insn->operands[i]));
		arg_types.push_back(GetContext().GetLLVMType(insn->operands[i]));
	}

	llvm::Type *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(GetContext().GetLLVMContext()), arg_types, false)->getPointerTo(0);

	auto fn_value = GetBuilder().CreateIntToPtr(GetValueFor(target), ftype);
	auto out_value = GetBuilder().CreateCall(fn_value, arg_values);

	if(target.is_vreg()) {
		SetValueFor(target, out_value);
	}

	insn++;

	return true;
}
