/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

ENUM_ENTRY(HaltCpu)
ENUM_ENTRY(ReadPc)
ENUM_ENTRY(WritePc)

ENUM_ENTRY(Popcount32)
ENUM_ENTRY(Clz32)
ENUM_ENTRY(Clz64)
ENUM_ENTRY(Ctz32)
ENUM_ENTRY(Ctz64)
ENUM_ENTRY(BSwap32)
ENUM_ENTRY(BSwap64)

ENUM_ENTRY(MemLock)
ENUM_ENTRY(MemUnlock)
ENUM_ENTRY(MemMonitorAcquire)
ENUM_ENTRY(MemMonitorWrite8)
ENUM_ENTRY(MemMonitorWrite16)
ENUM_ENTRY(MemMonitorWrite32)
ENUM_ENTRY(MemMonitorWrite64)

ENUM_ENTRY(SetCpuMode)
ENUM_ENTRY(GetCpuMode)

ENUM_ENTRY(Trap)
ENUM_ENTRY(TakeException)

ENUM_ENTRY(SetExecutionRing)
ENUM_ENTRY(EnterUserMode)
ENUM_ENTRY(EnterKernelMode)

ENUM_ENTRY(ProbeDevice)
ENUM_ENTRY(WriteDevice)
ENUM_ENTRY(WriteDevice64)

ENUM_ENTRY(SetFeature)
ENUM_ENTRY(GetFeature)

ENUM_ENTRY(PushInterrupt)
ENUM_ENTRY(PopInterrupt)
ENUM_ENTRY(PendIRQ)
ENUM_ENTRY(TriggerIRQ)

ENUM_ENTRY(InvalidateDCache)
ENUM_ENTRY(InvalidateDCacheEntry)
ENUM_ENTRY(InvalidateICache)
ENUM_ENTRY(InvalidateICacheEntry)
ENUM_ENTRY(FlushContextID)
ENUM_ENTRY(SetContextID)

ENUM_ENTRY(FloatIsSnan)
ENUM_ENTRY(FloatIsQnan)

ENUM_ENTRY(DoubleIsSnan)
ENUM_ENTRY(DoubleIsQnan)

ENUM_ENTRY(DoubleSqrt)
ENUM_ENTRY(FloatSqrt)

ENUM_ENTRY(DoubleAbs)
ENUM_ENTRY(FloatAbs)


ENUM_ENTRY(Adc8WithFlags)
ENUM_ENTRY(Adc16WithFlags)
ENUM_ENTRY(AdcWithFlags)
ENUM_ENTRY(Adc64WithFlags)
ENUM_ENTRY(SbcWithFlags)
ENUM_ENTRY(Sbc8WithFlags)
ENUM_ENTRY(Sbc16WithFlags)
ENUM_ENTRY(Sbc64WithFlags)
ENUM_ENTRY(Adc)
ENUM_ENTRY(Sbc)
ENUM_ENTRY(Adc64)
ENUM_ENTRY(Sbc64)

ENUM_ENTRY(UMULL)
ENUM_ENTRY(UMULH)
ENUM_ENTRY(SMULL)
ENUM_ENTRY(SMULH)

ENUM_ENTRY(UpdateZN32)
ENUM_ENTRY(UpdateZN64)

ENUM_ENTRY(FPGetRounding)
ENUM_ENTRY(FPSetRounding)
ENUM_ENTRY(FPGetFlush)
ENUM_ENTRY(FPSetFlush)

