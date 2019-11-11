/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * File:   ArmLinuxUserEmulationModel.h
 * Author: s0457958
 *
 * Created on 26 August 2014, 14:50
 */

#ifndef ARMLINUXUSEREMULATIONMODEL_H
#define	ARMLINUXUSEREMULATIONMODEL_H

#include "abi/LinuxUserEmulationModel.h"

namespace archsim
{
	namespace arch
	{
		namespace arm
		{
			class ArmLinuxUserEmulationModel : public archsim::abi::LinuxUserEmulationModel
			{
			public:
				enum ArmLinuxABIVersion {
					ABI_EABI,
					ABI_OABI,
				};

				ArmLinuxUserEmulationModel();
				ArmLinuxUserEmulationModel(ArmLinuxABIVersion abi_version);
				~ArmLinuxUserEmulationModel();

				bool Initialise(System& system, uarch::uArch& uarch) override;
				void Destroy() override;

				archsim::abi::ExceptionAction HandleException(archsim::core::thread::ThreadInstance *cpu, uint64_t category, uint64_t data) override;
				bool InvokeSignal(int signum, uint32_t next_pc, archsim::abi::SignalData* data);

				bool PrepareBoot(System& system);

				gensim::DecodeContext* GetNewDecodeContext(archsim::core::thread::ThreadInstance &cpu) override;


			private:
				bool InstallKernelHelpers();

				inline bool IsEABI() const
				{
					return abi_version == ABI_EABI;
				}

				inline bool IsOABI() const
				{
					return abi_version == ABI_OABI;
				}

				inline void SetABIVersion(ArmLinuxABIVersion new_version)
				{
					abi_version = new_version;
				}

				ArmLinuxABIVersion abi_version;
			};
		}
	}
}

#endif	/* ARMLINUXUSEREMULATIONMODEL_H */

