/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "translate/TranslationWorkUnit.h"
#include "translate/profile/Block.h"
#include "translate/profile/Region.h"
#include "translate/interrupts/InterruptCheckingScheme.h"

#include "gensim/gensim_decode.h"
#include "gensim/gensim_translate.h"
#include "core/MemoryInterface.h"

#include "util/LogContext.h"

#include <stdio.h>

UseLogContext(LogTranslate);

using namespace archsim::translate;
using namespace archsim::translate::interrupts;

TranslationWorkUnit::TranslationWorkUnit(archsim::core::thread::ThreadInstance *thread, profile::Region& region, uint32_t generation, uint32_t weight) : thread(thread), region(region), generation(generation), weight(weight), emit_trace_calls(thread->GetTraceSource() != nullptr)
{
	region.Acquire();
	dispatch_heat_ = region.GetTotalInterpCount();
}

TranslationWorkUnit::~TranslationWorkUnit()
{
	for(auto a : blocks) delete a.second;

	region.Release();
}

uint32_t TranslationWorkUnit::GetWeight() const
{
	return GetRegion().GetTotalInterpCount() - dispatch_heat_;
}


void TranslationWorkUnit::DumpGraph()
{
	std::stringstream s;
	s << "region-" << std::hex << GetRegion().GetPhysicalBaseAddress() << ".dot";

	FILE *f = fopen(s.str().c_str(), "wt");
	fprintf(f, "digraph a {\n");

	for (auto block : blocks) {
		fprintf(f, "B_%08x [color=%s,shape=Mrecord,label=\"B_%08x (%llu)\"]\n", block.first, block.second->IsEntryBlock() ? "red" : "black", block.first.Get(), block.second->GetInterpCount());

		if (block.second->IsInterruptCheck())
			fprintf(f, "B_%08x [fillcolor=yellow,style=filled]\n", block.second->GetOffset());

		for (auto succ : block.second->GetSuccessors()) {
			fprintf(f, "B_%08x -> B_%08x\n", block.first, succ->GetOffset());
		}
	}

	fprintf(f, "}\n");
	fclose(f);
}

TranslationBlockUnit *TranslationWorkUnit::AddBlock(profile::Block& block, bool entry)
{
	auto tbu = new TranslationBlockUnit(block.GetOffset(), block.GetISAMode(), entry, block.GetInterpCount());
	blocks[block.GetOffset()] = tbu;
	return tbu;
}

TranslationWorkUnit *TranslationWorkUnit::Build(archsim::core::thread::ThreadInstance *thread, profile::Region& region, InterruptCheckingScheme& ics, uint32_t weight)
{
	TranslationWorkUnit *twu = new TranslationWorkUnit(thread, region, region.GetMaxGeneration(), weight);

	twu->potential_virtual_bases.insert(region.virtual_images.begin(), region.virtual_images.end());

	host_addr_t guest_page_data;
//	thread->GetEmulationModel().GetMemoryModel().LockRegion(region.GetPhysicalBaseAddress(), profile::RegionArch::PageSize, guest_page_data);

	archsim::LegacyMemoryInterface phys_device(thread->GetEmulationModel().GetMemoryModel());
	archsim::MemoryInterface phys_interface(thread->GetArch().GetMemoryInterfaceDescriptor().GetFetchInterface());
	phys_interface.Connect(phys_device);

	auto decode_context = thread->GetEmulationModel().GetNewDecodeContext(*thread);

	for (auto block : region.blocks) {
		auto next_block_it = ++region.blocks.lower_bound(block.first);
		Address next_block_start;
		if(next_block_it != region.blocks.end()) {
			next_block_start = next_block_it->second->GetOffset();
		}

		auto tbu = twu->AddBlock(*block.second, block.second->IsRootBlock());

		bool end_of_block = false;
		Address offset = tbu->GetOffset();
		uint32_t insn_count = 0;

		while (!end_of_block && offset.Get() < profile::RegionArch::PageSize && (next_block_start == 0_ga || offset < next_block_start)) {
//		while (!end_of_block && offset.Get() < profile::RegionArch::PageSize) {
			Address insn_addr (region.GetPhysicalBaseAddress() + offset);
			gensim::BaseDecode *decode = thread->GetArch().GetISA(block.second->GetISAMode()).GetNewDecode();
			decode_context->DecodeSync(phys_interface, insn_addr, block.second->GetISAMode(), decode);

			if(decode->Instr_Code == (uint16_t)(-1)) {
				LC_WARNING(LogTranslate) << "Invalid Instruction at " << std::hex << (uint32_t)(region.GetPhysicalBaseAddress().Get() + offset.Get()) <<  ", ir=" << decode->ir << ", isa mode=" << (uint32_t)block.second->GetISAMode() << " whilst building " << *twu;
				delete decode;
				delete twu;
				return NULL;
			}

			// tbu takes ownership of decode
			tbu->AddInstruction(decode, offset - tbu->GetOffset());

			offset += decode->Instr_Length;
			end_of_block = decode->GetEndOfBlock();
			insn_count++;
		}

		if (!end_of_block) {
			tbu->SetSpanning(true);
		}
	}

	for (auto block : region.blocks) {
		if (!twu->ContainsBlock(block.second->GetOffset()))
			continue;

		auto tbu = twu->blocks[block.first];
		for (auto succ : block.second->GetSuccessors()) {
			if (!twu->ContainsBlock(succ->GetOffset()))
				continue;

			tbu->AddSuccessor(twu->blocks[succ->GetOffset()]);
		}
	}

	// TODO: fix this for multiple ISAs
	auto jump_info = thread->GetArch().GetISA(0).GetNewJumpInfo();
	ics.ApplyInterruptChecks(*jump_info, twu->blocks);

	region.IncrementGeneration();

	delete jump_info;
	delete decode_context;
	return twu;
}

TranslationBlockUnit::TranslationBlockUnit(Address offset, uint8_t isa_mode, bool entry, uint64_t interp_count)
	:
	offset(offset),
	isa_mode(isa_mode),
	entry(entry),
	interrupt_check(false),
	spanning(false),
	interp_count_(interp_count)
{

}

TranslationBlockUnit::~TranslationBlockUnit()
{
	for(auto i : instructions) {
		delete i;
	}
}

TranslationInstructionUnit *TranslationBlockUnit::AddInstruction(gensim::BaseDecode* decode, Address offset)
{
	assert(decode);
	auto tiu = new TranslationInstructionUnit(decode, offset);
	instructions.push_back(tiu);
	return tiu;
}

TranslationInstructionUnit::TranslationInstructionUnit(gensim::BaseDecode* decode, Address offset) : decode(decode), offset(offset)
{

}

TranslationInstructionUnit::~TranslationInstructionUnit()
{
	delete decode;
}

namespace archsim
{
	namespace translate
	{

		std::ostream& operator<< (std::ostream& out, const TranslationWorkUnit& twu)
		{
			out << "[TWU weight=" << std::dec << twu.weight << ", generation=" << twu.generation << ", " << twu.region << "]";
			return out;
		}

	}
}
