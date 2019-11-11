/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * PubSubType.h
 *
 *  Created on: 16 Oct 2015
 *      Author: harry
 */

#ifndef DeclarePubType
#define DeclarePubType(x)
#endif

DeclarePubType(PrivilegeLevelChange)
DeclarePubType(ITlbFullFlush)
DeclarePubType(ITlbEntryFlush)
DeclarePubType(DTlbFullFlush)
DeclarePubType(DTlbEntryFlush)

DeclarePubType(RegionDispatchedForTranslationPhysical)
DeclarePubType(RegionDispatchedForTranslationVirtual)
DeclarePubType(RegionTranslationComleted)

DeclarePubType(RegionInvalidatePhysical)

DeclarePubType(L1ICacheFlush)
DeclarePubType(L1DCacheFlush)
DeclarePubType(L2CacheFlush)

DeclarePubType(InstructionExecute)
DeclarePubType(BlockExecute)

DeclarePubType(FloatResult)

DeclarePubType(FunctionCall)
DeclarePubType(FunctionReturnFrom)

DeclarePubType(RegionTranslationStatsIRCount)

// Perform garbage collection on dead/invalid translations. Should be called if
// code is known to be modified (i.e. after a write to code and then a cache
// flush/fence)
DeclarePubType(FlushTranslations)

// Delete all existing translations
DeclarePubType(FlushAllTranslations)
DeclarePubType(FeatureChange)

DeclarePubType(UserEvent1)
DeclarePubType(UserEvent2)



DeclarePubType(_END)
