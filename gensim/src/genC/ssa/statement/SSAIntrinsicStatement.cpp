/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "genC/ssa/statement/SSAIntrinsicStatement.h"
#include "genC/ssa/statement/SSAConstantStatement.h"
#include "genC/ssa/analysis/FeatureDominanceAnalysis.h"
#include "genC/ssa/SSAStatementVisitor.h"
#include "genC/ir/IR.h"
#include "genC/ir/IRAction.h"

using namespace gensim::genc::ssa;
using gensim::genc::IRType;

#if 0

const SSAType& SSAIntrinsicStatement::ResolveType(IntrinsicType kind) const
{
	switch (kind) {
		case SSAIntrinsic_ReadPc:
			switch (Parent->Parent->Arch->wordsize) {
				case 32:
					return IRTypes::UInt32;
				case 64:
					return IRTypes::UInt64;
				default:
					throw std::logic_error("Unsupported architectural word size '" + std::to_string(Parent->Parent->Arch->wordsize) + "'");
			}

		case SSAIntrinsic_FPGetRounding:
			return IRTypes::UInt8;

		case SSAIntrinsic_Popcount32:
		case SSAIntrinsic_FPGetFlush:
		case SSAIntrinsic_GetFeature:
			return IRTypes::UInt32;  // todo: this should depend on the width of the arch

		case SSAIntrinsic_GetCpuMode:
		case SSAIntrinsic_ProbeDevice:
		case SSAIntrinsic_WriteDevice:
		case SSAIntrinsic_WriteDevice64:
		case SSAIntrinsic_FloatIsQnan:
		case SSAIntrinsic_FloatIsSnan:
		case SSAIntrinsic_DoubleIsQnan:
		case SSAIntrinsic_DoubleIsSnan:
			return IRTypes::UInt8;

		case SSAIntrinsic_FloatSqrt:
		case SSAIntrinsic_FloatAbs:
			return IRTypes::Float;

		case SSAIntrinsic_DoubleSqrt:
		case SSAIntrinsic_DoubleAbs:
			return IRTypes::Double;

		case SSAIntrinsic_Adc8WithFlags:
		case SSAIntrinsic_Sbc8WithFlags:
			return IRTypes::UInt8;
		case SSAIntrinsic_Adc16WithFlags:
		case SSAIntrinsic_Sbc16WithFlags:
			return IRTypes::UInt16;

		case SSAIntrinsic_BSwap32:
			return IRTypes::UInt32;
		case SSAIntrinsic_BSwap64:
			return IRTypes::UInt64;

		case SSAIntrinsic_Clz32:
		case SSAIntrinsic_Ctz32:
			return IRTypes::UInt32;
		case SSAIntrinsic_Clz64:
		case SSAIntrinsic_Ctz64:
			return IRTypes::UInt64;

		case SSAIntrinsic_MemMonitorWrite8:
		case SSAIntrinsic_MemMonitorWrite16:
		case SSAIntrinsic_MemMonitorWrite32:
		case SSAIntrinsic_MemMonitorWrite64:
			return IRTypes::UInt8;

		default:
			return IRTypes::Void;
	}
}

bool SSAIntrinsicStatement::IsFixed() const
{
	switch (Type) {
		case SSAIntrinsic_GetCpuMode:
			return true;
		case SSAIntrinsic_Clz32:
		case SSAIntrinsic_Popcount32:
		case SSAIntrinsic_ProbeDevice:
			return Args(0)->IsFixed();

		case SSAIntrinsic_GetFeature: {
			FeatureDominanceAnalysis fda;

			const SSAConstantStatement *feature_id = dynamic_cast<const SSAConstantStatement *>(this->Args(0));
			assert(feature_id);

			return !fda.HasDominatingSetFeature(feature_id->Constant.Int(), this);
		}

		default:
			return false;
	}
}
#endif

SSAIntrinsicStatement::~SSAIntrinsicStatement()
{

}

std::set<SSASymbol *> SSAIntrinsicStatement::GetKilledVariables()
{
	std::set<SSASymbol *> killed_vars;

	return killed_vars;
}

void SSAIntrinsicStatement::Accept(SSAStatementVisitor& visitor)
{
	visitor.VisitIntrinsicStatement(*this);
}
