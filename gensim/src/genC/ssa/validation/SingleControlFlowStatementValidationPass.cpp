/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "genC/ssa/validation/SSAActionValidationPass.h"
#include "genC/ssa/SSAFormAction.h"
#include "genC/ssa/SSABlock.h"
#include "genC/ssa/statement/SSAControlFlowStatement.h"
#include "genC/ssa/SSAType.h"
#include "DiagnosticContext.h"
#include "ComponentManager.h"

using gensim::DiagnosticContext;
using gensim::DiagnosticClass;

using namespace gensim::genc::ssa;
using namespace gensim::genc::ssa::validation;

class SingleControlFlowStatementValidationPass : public SSAActionValidationPass
{
	bool Run(const SSAFormAction* action, DiagnosticContext& ctx) override
	{
		bool success = true;

		for(auto block : action->GetBlocks()) {
			bool found = false;
			for(auto stmt : block->GetStatements()) {
				if(dynamic_cast<const SSAControlFlowStatement*>(stmt)) {
					if(found) {
						ctx.AddEntry(DiagnosticClass::Error, "Block has multiple control flow statements", block->GetDiag());
						success = false;
						break;
					}
					found = true;
				}
			}
		}

		return success;
	}
};

RegisterComponent0(SSAActionValidationPass, SingleControlFlowStatementValidationPass, "SingleControlFlowStatement", "Check that all blocks contain only one control flow statement");
