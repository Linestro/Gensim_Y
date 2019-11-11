/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * File:   gensim_translate.h
 * Author: s0803652
 *
 * Created on 07 November 2011, 14:55
 */

#ifndef _GENSIM_TRANSLATE_H
#define _GENSIM_TRANSLATE_H

#include "../define.h"
#include "../abi/Address.h"
#include "gensim_decode.h"
#include "gensim_processor_api.h"
#include "util/Counter.h"
#include "util/Histogram.h"
#include "core/arch/RegisterFileDescriptor.h"

#include <string>
#include <map>
#include <list>

namespace llvm
{
	class Value;
	class Module;
	class Function;
	class BasicBlock;

	class ConstantFolder;
	class IRBuilderDefaultInserter;
	template<typename T, typename Inserter> class IRBuilder;
}

namespace archsim
{
	namespace ij
	{
		class IJTranslationContext;
	}
	namespace translate
	{
		namespace translate_llvm
		{
			class LLVMTranslationContext;
		}
	}
	namespace core
	{
		namespace thread
		{
			class ThreadInstance;
		}
	}
}

namespace gensim
{
	class Processor;

	class JumpInfo
	{
	public:
		JumpInfo() : IsJump(false), IsIndirect(false), IsConditional(false), JumpTarget(0) {}

		bool IsJump;
		bool IsIndirect;
		bool IsConditional;
		archsim::Address JumpTarget;
	};

	class BaseJumpInfoProvider
	{
	public:
		virtual ~BaseJumpInfoProvider() {}
		virtual void GetJumpInfo(const gensim::BaseDecode *instr, archsim::Address pc, JumpInfo &info) {}
	};

	class BaseTranslate
	{
	public:
		BaseTranslate(const gensim::Processor& cpu) : cpu(cpu) { }
		virtual ~BaseTranslate() { }

		virtual void GetJumpInfo(const gensim::BaseDecode *instr, uint32_t pc, bool& indirect_jump, bool& direct_jump, uint32_t& jump_target) = 0;

	protected:
		const gensim::Processor &cpu;
	};

	class BaseLLVMTranslate
	{
	public:
		using Builder = llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>;

		virtual bool TranslateInstruction(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, archsim::core::thread::ThreadInstance *thread, const gensim::BaseDecode *decode, archsim::Address phys_pc, llvm::Function *fn) = 0;
		virtual llvm::Value *EmitPredicateCheck(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, archsim::core::thread::ThreadInstance *thread, const gensim::BaseDecode *decode, archsim::Address phys_pc, llvm::Function *fn) = 0;

		llvm::Value *EmitRegisterRead(Builder& builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, const archsim::RegisterFileEntryDescriptor &entry, llvm::Value *index);
		bool EmitRegisterWrite(Builder& builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, const archsim::RegisterFileEntryDescriptor &entry, llvm::Value *index, llvm::Value *value);

		void EmitTraceRegisterWrite(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int id, llvm::Value *value);
		void EmitTraceBankedRegisterWrite(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int id, llvm::Value *regnum, int size, llvm::Value *value_ptr);
		void EmitTraceRegisterRead(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int id, llvm::Value *value);
		void EmitTraceBankedRegisterRead(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int id, llvm::Value *regnum, int size, llvm::Value *value_ptr);

		llvm::Value *EmitMemoryRead(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int interface, int size_in_bytes, llvm::Value *address);
		void EmitMemoryWrite(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int interface, int size_in_bytes, llvm::Value *address, llvm::Value *value);

		void EmitTakeException(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, llvm::Value *category, llvm::Value *data);

		void EmitAdcWithFlags(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int bits, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *carry);
		void EmitSbcWithFlags(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int bits, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *carry);

		void EmitIncrementCounter(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, archsim::util::Counter64 &counter, uint32_t value=1);
		void EmitIncrementHistogram(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, archsim::util::Histogram &counter, uint64_t key, uint32_t value=1);
	protected:
		void QueueDynamicBlock(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, std::map<uint16_t, llvm::BasicBlock*> &dynamic_blocks, std::list<uint16_t> &dynamic_block_queue, uint16_t queued_block);

	private:
		llvm::Value *GetRegisterPtr(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx, int size_in_bytes, int offset);
		llvm::Value *GetRegfilePtr(archsim::translate::translate_llvm::LLVMTranslationContext& ctx);
		llvm::Value *GetThreadPtr(Builder &builder, archsim::translate::translate_llvm::LLVMTranslationContext& ctx);
	};

	class BaseIJTranslate : public BaseTranslate
	{
	public:
		BaseIJTranslate(const gensim::Processor& cpu);

		virtual bool TranslateInstruction(archsim::ij::IJTranslationContext& ctx, const gensim::BaseDecode& insn, uint32_t offset, bool trace) = 0;
	};
}

#endif /* _GENSIM_TRANSLATE_H */
