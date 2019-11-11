/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#pragma once

#include "genC/ssa/statement/SSAVariableKillStatement.h"

namespace gensim
{
	namespace genc
	{
		namespace ssa
		{
			class SSABitDepositStatement : public SSAStatement
			{
			public:
				SSABitDepositStatement(SSABlock *parent, SSAStatement *expr, SSAStatement *from_expr, SSAStatement *to_expr, SSAStatement *value_expr, SSAStatement *before=nullptr);
				virtual ~SSABitDepositStatement() { }

				virtual void PrettyPrint(std::ostringstream &) const override;
				virtual bool IsFixed() const override;
				void Accept(SSAStatementVisitor& visitor) override;

				std::set<SSASymbol*> GetKilledVariables() override
				{
					return {};
				}

				const SSAType GetType() const override
				{
					return Expr()->GetType();
				}

				STATEMENT_OPERAND(Expr, 0)
				STATEMENT_OPERAND(BitFrom, 1)
				STATEMENT_OPERAND(BitTo, 2)
				STATEMENT_OPERAND(Value, 3)
			};
		}
	}
}
