/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * File:   TranslationWorkUnit.h
 * Author: s0457958
 *
 * Created on 21 July 2014, 08:05
 */

#ifndef TRANSLATIONWORKUNIT_H
#define	TRANSLATIONWORKUNIT_H

#include "define.h"

#include "core/thread/ThreadInstance.h"
#include "translate/profile/RegionArch.h"
#include "util/Zone.h"

#include <list>
#include <map>
#include <ostream>
#include <set>

namespace gensim
{
	class BaseDecode;
}

namespace archsim
{
	namespace translate
	{
		namespace profile
		{
			class Region;
			class Block;
		}
		namespace interrupts
		{
			class InterruptCheckingScheme;
		}

		class TranslationBlockUnit;
		class TranslationInstructionUnit
		{
		public:
			TranslationInstructionUnit(const TranslationInstructionUnit &) = delete;
			TranslationInstructionUnit(gensim::BaseDecode* decode, Address offset);
			~TranslationInstructionUnit();

			inline const gensim::BaseDecode& GetDecode() const
			{
				return *decode;
			}

			inline Address GetOffset() const
			{
				return offset;
			}

		private:
			gensim::BaseDecode* decode;
			Address offset;
		};

		class TranslationWorkUnit;
		class TranslationBlockUnit
		{
		public:
			TranslationBlockUnit(const TranslationBlockUnit&) = delete;
			TranslationBlockUnit(Address offset, uint8_t isa_mode, bool entry_block, uint64_t interp_count);
			~TranslationBlockUnit();

			TranslationInstructionUnit *AddInstruction(gensim::BaseDecode* decode, Address offset);

			inline Address GetOffset() const
			{
				return offset;
			}

			inline uint8_t GetISAMode() const
			{
				return isa_mode;
			}

			inline const std::list<TranslationInstructionUnit *>& GetInstructions() const
			{
				return instructions;
			}

			inline TranslationInstructionUnit& GetLastInstruction() const
			{
				return *instructions.back();
			}

			inline bool IsEntryBlock() const
			{
				return entry;
			}

			inline const std::list<TranslationBlockUnit *> GetSuccessors() const
			{
				return successors;
			}

			inline bool HasSuccessors() const
			{
				return successors.size() > 0;
			}

			inline bool HasSelfLoop() const
			{
				for (auto succ : successors) {
					if (succ == this)
						return true;
				}

				return false;
			}

			inline void AddSuccessor(TranslationBlockUnit *successor)
			{
				successors.push_back(successor);
			}

			inline void SetInterruptCheck(bool is_check)
			{
				interrupt_check = is_check;
			}

			inline bool IsInterruptCheck() const
			{
				return interrupt_check;
			}

			inline void SetSpanning(bool is_spanning)
			{
				spanning = is_spanning;
			}

			inline bool IsSpanning() const
			{
				return spanning;
			}

			uint64_t GetInterpCount() const
			{
				return interp_count_;
			}

		private:

			Address offset;
			uint64_t interp_count_;
			uint8_t isa_mode;
			bool entry;
			bool interrupt_check;
			bool spanning;

			std::list<TranslationInstructionUnit *> instructions;
			std::list<TranslationBlockUnit *> successors;
		};

		class TranslationWorkUnit
		{
		public:
			friend std::ostream& operator<< (std::ostream& out, const TranslationWorkUnit& twu);

			TranslationWorkUnit(const TranslationWorkUnit&) = delete;
			TranslationWorkUnit(archsim::core::thread::ThreadInstance *thread, profile::Region& region, uint32_t generation, uint32_t weight);
			~TranslationWorkUnit();

			static TranslationWorkUnit *Build(archsim::core::thread::ThreadInstance *thread, profile::Region& region, interrupts::InterruptCheckingScheme& ics, uint32_t weight);

			inline bool ShouldEmitTracing() const
			{
				return emit_trace_calls;
			}

			uint32_t GetWeight() const;

			inline uint32_t GetGeneration() const
			{
				return generation;
			}

			inline profile::Region& GetRegion() const
			{
				return region;
			}

			inline archsim::core::thread::ThreadInstance *GetThread() const
			{
				return thread;
			}

			inline const std::map<Address, TranslationBlockUnit *>& GetBlocks() const
			{
				return blocks;
			}

			TranslationBlockUnit *AddBlock(profile::Block& block, bool entry);

			inline bool ContainsBlock(Address block_offset) const
			{
				return blocks.count(block_offset);
			}

			inline uint32_t GetBlockCount() const
			{
				return blocks.size();
			}

			void DumpGraph();

			typedef util::Zone<TranslationInstructionUnit, 2048> instruction_zone_t;
			inline instruction_zone_t &GetInstructionZone()
			{
				return instruction_zone;
			}

			std::set<Address> potential_virtual_bases;

		private:
			uint64_t dispatch_heat_;
			archsim::core::thread::ThreadInstance *thread;
			profile::Region& region;
			uint32_t generation;
			uint32_t weight;
			bool emit_trace_calls;

			/**
			 * Contains a mapping from page offsets to translation block units.
			 */
			std::map<Address, TranslationBlockUnit *> blocks;

			instruction_zone_t instruction_zone;
		};
	}
}


#endif	/* TRANSLATIONWORKUNIT_H */

