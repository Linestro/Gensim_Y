/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */


#include "genC/ir/IRAction.h"
#include "genC/ir/IRType.h"
#include "genC/ir/IREnums.h"
#include "genC/ssa/SSABlock.h"
#include "genC/ssa/passes/SSAPass.h"
#include "genC/ssa/SSAFormAction.h"
#include "genC/ssa/SSASymbol.h"
#include "genC/ssa/statement/SSAStatement.h"
#include "genC/ssa/statement/SSACastStatement.h"
#include "genC/ssa/statement/SSAConstantStatement.h"
#include "genC/ssa/statement/SSAIfStatement.h"
#include "genC/ssa/statement/SSAJumpStatement.h"
#include "genC/ssa/statement/SSASwitchStatement.h"
#include "genC/ssa/statement/SSAPhiStatement.h"

using namespace gensim::genc::ssa;

static bool StatementHasNoNonBlockUses(SSAStatement *stmt)
{
	for(auto use : stmt->GetUses()) {
		if(dynamic_cast<SSABlock*>(use) == nullptr) {
			return false;
		}
	}
	return true;
}

// This is not very efficient - we loop over the block multiple times
// and only remove one dead statement per loop.
static bool RemoveADeadStatement(SSABlock *block)
{
	if(block->GetStatements().empty()) {
		return false;
	}

	auto iterator = block->GetStatements().rbegin();
	// skip control flow
	iterator++;
	while(iterator != block->GetStatements().rend()) {
		SSAStatement *statement = *iterator;
		if(!statement->HasSideEffects()) {
			if(StatementHasNoNonBlockUses(statement)) {
				block->RemoveStatement(*statement);

				statement->Dispose();
				delete statement;

				return true;
			}
		}

		iterator++;
	}

	return false;
}

static bool RemoveDeadStatements(SSABlock *block)
{
	if(block->GetStatements().empty()) {
		return false;
	}

	bool changed = false;
	bool any_change = false;
	do {
		changed = RemoveADeadStatement(block);
		any_change |= changed;
	} while(changed);
	return any_change;
}

static bool RemoveDeadStatements(SSAFormAction &action)
{
	bool changed = false;
	for(auto block : action.GetBlocks()) {
		changed |= RemoveDeadStatements(block);
	}
	return changed;
}

class DeadCodeEliminationPass : public SSAPass
{
public:
	virtual ~DeadCodeEliminationPass()
	{

	}

	bool Run(SSAFormAction& action) const override
	{
		bool changed = false;
		changed |= RemoveDeadStatements(action);

		return changed;
	}
};

RegisterComponent0(SSAPass, DeadCodeEliminationPass, "DeadCodeElimination", "Remove any valued statements which are not used")
