/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * ElfSystemEmulationModel.cpp
 *
 *  Created on: 8 Sep 2014
 *      Author: harry
 */

#include "arch/arm/AngelSyscallHandler.h"

#include "abi/ElfSystemEmulationModel.h"
#include "abi/loader/BinaryLoader.h"

#include "core/thread/ThreadInstance.h"

#include "util/ComponentManager.h"
#include "util/SimOptions.h"
#include "util/LogContext.h"

#define STACK_SIZE	0x40000

UseLogContext(LogSystemEmulationModel);
DeclareChildLogContext(LogElfSystemEmulationModel, LogSystemEmulationModel, "ELF");

using namespace archsim::abi;

//RegisterComponent(EmulationModel, ElfSystemEmulationModel, "elf-system", "An emulation model for running ELF binaries in a bare metal context")

ElfSystemEmulationModel::ElfSystemEmulationModel() : initial_sp(0), SystemEmulationModel(false)
{
	heap_base = Address(0x20000000);
	heap_limit = Address(0x30000000);
	stack_base = Address(0x50000000);
	stack_limit = Address(0x40000000);
}

ElfSystemEmulationModel::~ElfSystemEmulationModel()
{

}

bool ElfSystemEmulationModel::Initialise(System &system, archsim::uarch::uArch &uarch)
{
	bool rc = SystemEmulationModel::Initialise(system, uarch);
	if (!rc)
		return false;

	// Initialise "physical memory".
	GetMemoryModel().GetMappingManager()->MapRegion(Address(0), 0xffff0000, (archsim::abi::memory::RegionFlags)7, "phys-mem");

	return true;
}

void ElfSystemEmulationModel::Destroy()
{
	SystemEmulationModel::Destroy();
}

ExceptionAction ElfSystemEmulationModel::HandleException(archsim::core::thread::ThreadInstance *cpu, uint64_t category, uint64_t data)
{
	if(category == 3 && data == 0x123456) {
		if(HandleSemihostingCall()) {
			return ResumeNext;
		} else {
			// This is just a debug message as the semihosting interface should produce
			// a meaningful LC_ERROR message if there was an actual error
			LC_DEBUG1(LogElfSystemEmulationModel) << "A semihosting call caused a simulation abort.";
			return AbortSimulation;
		}
	}
	//We should not take any exceptions in this context
	LC_ERROR(LogElfSystemEmulationModel) << "An exception was generated so aborting simulation. Category " << category << ", Data " << data;
	return AbortSimulation;
}

void ElfSystemEmulationModel::DestroyDevices()
{

}

bool ElfSystemEmulationModel::InstallPlatform(loader::BinaryLoader& loader)
{
	binary_entrypoint = loader.GetEntryPoint();

	if (!archsim::options::Bootloader.IsSpecified()) {
		LC_ERROR(LogElfSystemEmulationModel) << "Bootloader must be specified for ELF system emulation.";
		return false;
	}

	if (!InstallBootloader(archsim::options::Bootloader)) {
		LC_ERROR(LogElfSystemEmulationModel) << "Bootloader installation failed";
		return false;
	}

	if (!InstallKernelHelpers()) {
		LC_ERROR(LogElfSystemEmulationModel) << "Kernel helper installation failed";
		return false;
	}

	if (!PrepareStack()) {
		LC_ERROR(LogElfSystemEmulationModel) << "Error initialising stack";
		return false;
	}

	return true;
}

bool ElfSystemEmulationModel::PrepareCore(archsim::core::thread::ThreadInstance &cpu)
{
	LC_DEBUG1(LogElfSystemEmulationModel) << "Binary entry point: " << std::hex << binary_entrypoint;

	// Load r12 with the entry-point to the binary being executed.
	uint32_t *regs = (uint32_t *)cpu.GetRegisterFileInterface().GetEntry<uint32_t>("RB");
	regs[12] = binary_entrypoint.Get();

	LC_DEBUG1(LogElfSystemEmulationModel) << "Initial SP: " << std::hex << initial_sp;

	// Load sp with the top of the stack.
	regs = (uint32_t *)cpu.GetRegisterFileInterface().GetEntry<uint32_t>("RB");
	regs[13] = initial_sp.Get();

	return true;
}

bool ElfSystemEmulationModel::HandleSemihostingCall()
{
	UNIMPLEMENTED;
//	archsim::arch::arm::AngelSyscallHandler angel (main_thread_, heap_base, heap_limit, stack_base, stack_limit);
//	return angel.HandleSyscall(*GetBootCore());
}

bool ElfSystemEmulationModel::InstallDevices()
{
	archsim::abi::devices::Device *coprocessor;
	if (!GetComponentInstance("armcoprocessor", coprocessor)) {
		LC_ERROR(LogElfSystemEmulationModel) << "Unable to instantiate ARM coprocessor";
		return false;
	}

	main_thread_->GetPeripherals().RegisterDevice("coprocessor", coprocessor);
	main_thread_->GetPeripherals().AttachDevice("coprocessor", 15);

	archsim::abi::devices::Device *mmu;
	if (!GetComponentInstance("ARM926EJSMMU", mmu)) {
		LC_ERROR(LogElfSystemEmulationModel) << "Unable to instantiate MMU";
		return false;
	}

	main_thread_->GetPeripherals().RegisterDevice("mmu", mmu);
	main_thread_->GetPeripherals().InitialiseDevices();

	return true;
}

bool ElfSystemEmulationModel::InstallBootloader(std::string filename)
{
	loader::SystemElfBinaryLoader<archsim::abi::loader::ElfClass32> loader(*this, true);

	LC_DEBUG1(LogElfSystemEmulationModel) << "Installing Bootloader from '"	<< filename << "'";

	if (!loader.LoadBinary(filename)) {
		LC_ERROR(LogElfSystemEmulationModel) << "Bootloader loading failed";
		return false;
	}

	return true;
}

bool ElfSystemEmulationModel::PrepareStack()
{
	initial_sp = Address(0xc0000000 + STACK_SIZE);

	return true;
}

bool ElfSystemEmulationModel::InstallKernelHelpers()
{
	LC_DEBUG1(LogElfSystemEmulationModel) << "Initialising ARM Kernel Helpers";

	memory::guest_addr_t kernel_helper_region = Address(0xffff0000);
	GetMemoryModel().GetMappingManager()->MapRegion(kernel_helper_region, 0x4000, (memory::RegionFlags)(memory::RegFlagRead), "[eabi]");

	return true;
}
