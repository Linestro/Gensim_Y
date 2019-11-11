/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */


#include "ArchSimBlockJITTest.h"

using namespace captive::shared;
using namespace captive::arch::jit;

class ArchSimBlockJITCmpBranchTest : public ArchSimBlockJITTest
{
public:
	template<typename LH, typename RH> using cmp_factory_t = std::function<IRInstruction(LH lh, RH rh, IRRegId output)>;

	template<typename A, typename B, typename C> void build_cmp_skeleton(cmp_factory_t<IRRegId, IRRegId> cmp_factory, A, B, C)
	{
		using namespace captive::shared;

		auto taken_block = Builder().alloc_block(), nottaken_block = Builder().alloc_block();

		IRRegId lh = Allocate(A(), 4);
		IRRegId rh = Allocate(B(), 4);
		IRRegId output = Allocate(C(), 1);

		Builder().ldreg(IROperand::const32(0), IROperand::vreg(lh, 4));
		Builder().ldreg(IROperand::const32(4), IROperand::vreg(rh, 4));

		Builder().add_instruction(cmp_factory(lh, rh, output));
		Builder().branch(IROperand::vreg(output, 1), IROperand::block(taken_block), IROperand::block(nottaken_block));

		Builder().SetBlock(taken_block);
		Builder().streg(IROperand::const32(1), IROperand::const32(0));
		Builder().ret();

		Builder().SetBlock(nottaken_block);
		Builder().streg(IROperand::const32(0), IROperand::const32(0));
		Builder().ret();
	}

	template<typename LHSValueGen, typename RHSValueGen> void build_and_test(std::function<bool(uint32_t, uint32_t)> result_fn)
	{
		transforms::AllocationWriterTransform awt(allocations_);
		awt.Apply(tc_);

		CompileResult cr (true, stack_frame_, wutils::vbitset<>(8, 0xff));

		auto fn = Lower(cr);
		ASSERT_NE(nullptr, fn);

		std::vector<char> regfile_mock(128, 0);
		uint32_t *regfile = (uint32_t*)regfile_mock.data();
		for(int i = 0; i < kIterations; ++i) {

			uint32_t x = LHSValueGen(), y = RHSValueGen();
			regfile[0] = x;
			regfile[1] = y;

			fn(regfile_mock.data(), nullptr);

			ASSERT_EQ(*(uint32_t*)regfile_mock.data(), result_fn(x,y));
		}
	}
};

template<typename T> IROperand make_operand(T t, uint32_t size);
template<> inline IROperand make_operand<>(IRRegId t, uint32_t size)
{
	return IROperand::vreg(t, size);
}
template<> inline IROperand make_operand<>(uint64_t t, uint32_t size)
{
	return IROperand::constant(t, size);
}

#define MAKE_TEST(name, a, b, c, instruction, comparator) TEST_F(ArchSimBlockJITCmpBranchTest, name) { \
	build_cmp_skeleton<a, b, c>([](IRRegId lh, IRRegId rh, IRRegId output){return IRInstruction::instruction(IROperand::vreg(lh, 4), IROperand::vreg(rh, 4), IROperand::vreg(output, 1));}, a(), b(), c()); \
	build_and_test<RandValueGen<>, RandValueGen<>>(comparator()); \
}
#define MAKE_TEST_CA(name, a, b, c, instruction, comparator) TEST_F(ArchSimBlockJITCmpBranchTest, name) { \
	build_cmp_skeleton<a, b, c>([](IRRegId lh, IRRegId rh, IRRegId output){return IRInstruction::instruction(IROperand::const32(ArchSimBlockJITCmpBranchTest::kUConstant), IROperand::vreg(rh, 4), IROperand::vreg(output, 1));}, a(), b(), c()); \
	build_and_test<ConstValueGen, RandValueGen<>>(comparator()); \
}
#define MAKE_TEST_CB(name, a, b, c, instruction, comparator) TEST_F(ArchSimBlockJITCmpBranchTest, name) { \
	build_cmp_skeleton<a, b, c>([](IRRegId lh, IRRegId rh, IRRegId output){return IRInstruction::instruction(IROperand::vreg(lh, 4), IROperand::const32(ArchSimBlockJITCmpBranchTest::kUConstant), IROperand::vreg(output, 1));}, a(), b(), c()); \
	build_and_test<RandValueGen<>, ConstValueGen>(comparator()); \
}

#define MAKE_TESTS_CMP(name_root, instruction, comparator) \
	MAKE_TEST(name_root ## SSS, StackTag, StackTag, StackTag, instruction, comparator) \
	MAKE_TEST(name_root ## SSR, StackTag, StackTag, RegTag, instruction, comparator) \
	MAKE_TEST(name_root ## SRS, StackTag, RegTag, StackTag, instruction, comparator) \
	MAKE_TEST(name_root ## SRR, StackTag, RegTag, RegTag, instruction, comparator) \
	MAKE_TEST(name_root ## RSS, RegTag, StackTag, StackTag, instruction, comparator) \
	MAKE_TEST(name_root ## RSR, RegTag, StackTag, RegTag, instruction, comparator) \
	MAKE_TEST(name_root ## RRS, RegTag, RegTag, StackTag, instruction, comparator) \
	MAKE_TEST(name_root ## RRR, RegTag, RegTag, RegTag, instruction, comparator)  \
	MAKE_TEST_CA(name_root ## CSS, RegTag, StackTag, StackTag, instruction, comparator) \
	MAKE_TEST_CA(name_root ## CSR, RegTag, StackTag, RegTag, instruction, comparator) \
	MAKE_TEST_CA(name_root ## CRS, RegTag, RegTag, StackTag, instruction, comparator) \
	MAKE_TEST_CA(name_root ## CRR, RegTag, RegTag, RegTag, instruction, comparator) \
	MAKE_TEST_CB(name_root ## SCS, StackTag, StackTag, StackTag, instruction, comparator) \
	MAKE_TEST_CB(name_root ## SCR, StackTag, StackTag, RegTag, instruction, comparator) \
	MAKE_TEST_CB(name_root ## RCS, RegTag, StackTag, StackTag, instruction, comparator) \
	MAKE_TEST_CB(name_root ## RCR, RegTag, StackTag, RegTag, instruction, comparator)


MAKE_TESTS_CMP(cmpeq, cmpeq, std::equal_to<uint32_t>)
MAKE_TESTS_CMP(cmpne, cmpne, std::not_equal_to<uint32_t>)
MAKE_TESTS_CMP(cmplt, cmplt, std::less<uint32_t>)
MAKE_TESTS_CMP(cmplte, cmplte, std::less_equal<uint32_t>)
MAKE_TESTS_CMP(cmpgt, cmpgt, std::greater<uint32_t>)
MAKE_TESTS_CMP(cmpgte, cmpgte, std::greater_equal<uint32_t>)
MAKE_TESTS_CMP(cmpslt, cmpslt, std::less<int32_t>)
MAKE_TESTS_CMP(cmpslte, cmpslte, std::less_equal<int32_t>)
MAKE_TESTS_CMP(cmpsgt, cmpsgt, std::greater<int32_t>)
MAKE_TESTS_CMP(cmpsgte, cmpsgte, std::greater_equal<int32_t>)
