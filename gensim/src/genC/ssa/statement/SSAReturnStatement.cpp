/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "genC/ssa/statement/SSAReturnStatement.h"
#include "genC/ssa/SSAStatementVisitor.h"

using namespace gensim::genc::ssa;

SSAReturnStatement::SSAReturnStatement(SSABlock* parent, SSAStatement* value, SSAStatement* before) : SSAControlFlowStatement(1, parent, before)
{
	SetOperand(0, value);
}


bool SSAReturnStatement::IsFixed() const
{
	return true;
}

bool SSAReturnStatement::Resolve(DiagnosticContext &ctx)
{
	bool success = true;
	if (Value() != NULL) success &= Value()->Resolve(ctx);
	return success;
}

std::set<SSASymbol *> SSAReturnStatement::GetKilledVariables()
{
	return std::set<SSASymbol *>();
}

SSAReturnStatement::~SSAReturnStatement()
{
}

void SSAReturnStatement::Accept(SSAStatementVisitor& visitor)
{
	visitor.VisitReturnStatement(*this);
}
