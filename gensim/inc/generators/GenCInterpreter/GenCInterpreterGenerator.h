/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#ifndef GENCINTERPRETERGENERATOR_H_
#define GENCINTERPRETERGENERATOR_H_

#include <unordered_set>
#include <fstream>

#include "generators/InterpretiveExecutionEngineGenerator.h"

namespace gensim
{
	namespace isa
	{
		class ISADescription;
	}

	namespace genc
	{
		namespace ssa
		{
			class SSAFormAction;
		}
	}

	namespace generator
	{

		class GenCInterpreterGenerator : public InterpretiveExecutionEngineGenerator
		{
		public:
			enum class HelperPrototypeVariant {
				INVALID,
				DeclarationWithDefault,
				DeclarationNoDefault,
				SpecialisationWithTracing,
				SpecialisationNoTracing
			};

			GenCInterpreterGenerator(GenerationManager &manager);

			bool GenerateExecuteBodyFor(util::cppformatstream &str, const genc::ssa::SSAFormAction &action) const;
			bool GeneratePrototype(util::cppformatstream &stream, const gensim::isa::ISADescription &isa, const genc::ssa::SSAFormAction &action, HelperPrototypeVariant variant) const;
			std::string GeneratePrototype(const gensim::isa::ISADescription &isa, const genc::ssa::SSAFormAction &action, HelperPrototypeVariant variant) const;

			bool Generate() const override;

			bool RegisterHelpers(const gensim::isa::ISADescription &isa) const;
		protected:
			virtual bool GenerateExecutionForBehaviour(util::cppformatstream &, bool, std::string, const isa::ISADescription &) const override;
			virtual bool GenerateExtraProcessorClassMembers(util::cppformatstream &stream) const override;
			virtual bool GenerateExtraProcessorSource(util::cppformatstream &stream) const override;
			virtual bool GenerateExtraProcessorInitSource(util::cppformatstream &stream) const override;
			virtual bool GenerateExtraProcessorIncludes(util::cppformatstream &stream) const override;
			virtual bool GenerateExtraProcessorDestructorSource(util::cppformatstream &stream) const override;


			virtual bool GenerateInlineHelperFns(util::cppformatstream &) const override;
			virtual bool GenerateExternHelperFns(util::cppformatstream &) const override;



		};
	}
}

#endif /* GENCINTERPRETERGENERATOR_H_ */
