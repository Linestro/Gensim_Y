/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "genC/ssa/statement/SSABinaryArithmeticStatement.h"
#include "genC/ssa/SSAStatementVisitor.h"

using namespace gensim::genc::ssa;
using gensim::genc::IRType;

bool SSABinaryArithmeticStatement::IsFixed() const
{
	return LHS()->IsFixed() && RHS()->IsFixed();
}

bool SSABinaryArithmeticStatement::Resolve(DiagnosticContext &ctx)
{
	bool success = true;
	success &= SSAStatement::Resolve(ctx);
	success &= LHS()->Resolve(ctx);
	success &= RHS()->Resolve(ctx);
	return success;
}

void SSABinaryArithmeticStatement::Accept(SSAStatementVisitor& visitor)
{
	visitor.VisitBinaryArithmeticStatement(*this);
}

bool SSABinaryArithmeticStatement::HasSideEffects() const
{
	return false;
}

std::set<SSASymbol *> SSABinaryArithmeticStatement::GetKilledVariables()
{
	return std::set<SSASymbol *>();
}

SSABinaryArithmeticStatement::~SSABinaryArithmeticStatement()
{
}
