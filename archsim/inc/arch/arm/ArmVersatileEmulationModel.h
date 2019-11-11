/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * File:   ArmRealviewEmulationModel.h
 * Author: harry
 *
 * Created on 16 December 2013, 11:02
 */

#ifndef ARMVERSATILEEMULATIONMODEL_H
#define	ARMVERSATILEEMULATIONMODEL_H

#include "abi/LinuxSystemEmulationModel.h"
#include "abi/devices/generic/block/FileBackedBlockDevice.h"

namespace archsim
{
	namespace arch
	{
		namespace arm
		{
			class ArmVersatileEmulationModel : public archsim::abi::LinuxSystemEmulationModel
			{
			public:
				ArmVersatileEmulationModel();
				~ArmVersatileEmulationModel();

				bool Initialise(System& system, uarch::uArch& uarch) override;
				void Destroy() override;

				archsim::abi::ExceptionAction HandleException(archsim::core::thread::ThreadInstance *thread, uint64_t category, uint64_t data) override;
				gensim::DecodeContext* GetNewDecodeContext(archsim::core::thread::ThreadInstance& cpu) override;


			protected:
				bool CreateCoreDevices(archsim::core::thread::ThreadInstance* thread) override;
				bool CreateMemoryDevices() override;
				bool PreparePlatform(abi::loader::BinaryLoader& loader) override;

				bool PrepareCore(archsim::core::thread::ThreadInstance& core) override;

			private:
				Address entry_point;

				bool InstallBootloader(archsim::abi::loader::BinaryLoader& loader);

				bool HackyMMIORegisterDevice(abi::devices::MemoryComponent& device);
				bool AddGenericPrimecellDevice(Address base_addr, uint32_t size, uint32_t peripheral_id);

				void HandleSemihostingCall();

			};
		}
	}
}

#endif	/* ARMSYSTEMEMULATIONMODEL_H */

