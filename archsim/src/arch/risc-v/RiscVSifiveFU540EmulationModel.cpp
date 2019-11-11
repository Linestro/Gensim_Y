/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "abi/devices/SerialPort.h"
#include "abi/devices/EmptyDevice.h"
#include "arch/risc-v/RiscVDecodeContext.h"
#include "arch/risc-v/RiscVMMU.h"
#include "arch/risc-v/RiscVSystemCoprocessor.h"
#include "arch/risc-v/RiscVSifiveFU540EmulationModel.h"
#include "abi/devices/riscv/SifiveCLINT.h"
#include "abi/devices/riscv/SifivePLIC.h"
#include "abi/devices/riscv/SifiveUART.h"
#include "abi/devices/virtio/VirtIOBlock.h"
#include "module/ModuleManager.h"
#include "util/ComponentManager.h"
#include "system.h"

using namespace archsim::abi;
using namespace archsim::arch::riscv;

UseLogContext(LogEmulationModelRiscVSystem)

RiscVSifiveFU540EmulationModel::RiscVSifiveFU540EmulationModel() : RiscVSystemEmulationModel(64)
{

}

bool RiscVSifiveFU540EmulationModel::Initialise(System& system, archsim::uarch::uArch& uarch)
{
	if (!SystemEmulationModel::Initialise(system, uarch))
		return false;

	// Initialise "physical memory".
	if (!GetMemoryModel().GetMappingManager()) {
		return false;
	}

	GetMemoryModel().GetMappingManager()->MapRegion(Address(0x10000), 0x8000, archsim::abi::memory::RegFlagReadWriteExecute, "ROM");
	GetMemoryModel().GetMappingManager()->MapRegion(Address(0x10000000), 0x1000, archsim::abi::memory::RegFlagReadWriteExecute, "PRCI");
	GetMemoryModel().GetMappingManager()->MapRegion(Address(0x80000000), 512 * 1024 * 1024, archsim::abi::memory::RegFlagReadWriteExecute, "DRAM");

	// load device tree
	if(archsim::options::DeviceTreeFile.IsSpecified()) {
		std::ifstream str(archsim::options::DeviceTreeFile.GetValue(), std::ios::binary | std::ios::ate);
		auto size = str.tellg();
		std::vector<char> data (size);
		str.seekg(0);
		str.read(data.data(), size);

		GetMemoryModel().Write(Address(0x10000), (uint8_t*)data.data(), size);
	}

	InstantiateThreads(2);

	CreateMemoryDevices();

	return true;
}

bool RiscVSifiveFU540EmulationModel::CreateCoreDevices(archsim::core::thread::ThreadInstance* thread)
{
	auto mmu = new archsim::arch::riscv::RiscVMMU();
	auto coprocessor = new archsim::arch::riscv::RiscVSystemCoprocessor(thread, mmu);

	thread->GetPeripherals().RegisterDevice("mmu", mmu);
	thread->GetPeripherals().RegisterDevice("coprocessor", coprocessor);

	thread->GetPeripherals().AttachDevice("coprocessor", 0);

	thread->GetPeripherals().InitialiseDevices();

	return true;
}

bool RiscVSifiveFU540EmulationModel::CreateMemoryDevices()
{
	using namespace archsim::module;

	std::vector<archsim::core::thread::ThreadInstance*> harts;
	for(int i = 0; i < GetNumThreads(); ++i) {
		harts.push_back(&GetThread(i));
	}

	std::string plic_config = "MS";
	for(int i = 1; i < GetNumThreads(); ++i) {
		plic_config += ",MS";
	}

	// CLINT
	auto clint = new archsim::abi::devices::riscv::SifiveCLINT(*this, Address(0x2000000));
	clint->SetListParameter("Harts", harts);
	clint->Initialise();
	RegisterMemoryComponent(*clint);

	// PLIC
	auto plic = new archsim::abi::devices::riscv::SifivePLIC(*this, Address(0x0c000000));
	plic->SetListParameter("Harts", harts);
	plic->SetParameter("HartConfig", plic_config);
	plic->Initialise();
	RegisterMemoryComponent(*plic);


	auto uart0 = new archsim::abi::devices::riscv::SifiveUART(*this, Address(0x10010000));
	uart0->SetParameter("SerialPort", new archsim::abi::devices::ConsoleSerialPort());
	uart0->SetParameter("IRQLine", plic->RegisterSource(4));
	RegisterMemoryComponent(*uart0);

	auto uart1 = new archsim::abi::devices::riscv::SifiveUART(*this, Address(0x10011000));
	uart1->SetParameter("SerialPort", new archsim::abi::devices::ConsoleOutputSerialPort());
	uart1->SetParameter("IRQLine", plic->RegisterSource(5));
	RegisterMemoryComponent(*uart1);

	auto gpio = new archsim::abi::devices::EmptyDevice(*this, Address(0x10060000), 0x1000);
	RegisterMemoryComponent(*gpio);

	auto spi0 = new archsim::abi::devices::EmptyDevice(*this, Address(0x10040000), 0x1000);
	RegisterMemoryComponent(*spi0);
	auto spi1 = new archsim::abi::devices::EmptyDevice(*this, Address(0x10041000), 0x1000);
	RegisterMemoryComponent(*spi1);
	auto spi2 = new archsim::abi::devices::EmptyDevice(*this, Address(0x10050000), 0x1000);
	RegisterMemoryComponent(*spi2);
	auto ethernet = new archsim::abi::devices::EmptyDevice(*this, Address(0x10090000), 0x2000);
	RegisterMemoryComponent(*ethernet);
	auto i2c = new archsim::abi::devices::EmptyDevice(*this, Address(0x10030000), 0x1000);
	RegisterMemoryComponent(*i2c);

	auto pwm0 = new archsim::abi::devices::EmptyDevice(*this, Address(0x10020000), 0x1000);
	RegisterMemoryComponent(*pwm0);
	auto pwm1 = new archsim::abi::devices::EmptyDevice(*this, Address(0x10021000), 0x1000);
	RegisterMemoryComponent(*pwm1);

	// VirtIO Block device
	if(GetSystem().HasBlockDevice("vda")) {
		devices::virtio::VirtIOBlock *vbda = new devices::virtio::VirtIOBlock(*this, *plic->RegisterSource(50), Address(0x20000000), "virtio-block-a", *GetSystem().GetBlockDevice("vda"));
		RegisterMemoryComponent(*vbda);
	}

	uart0->Initialise();
	uart1->Initialise();

	return true;
}

bool RiscVSifiveFU540EmulationModel::PreparePlatform(loader::BinaryLoader& loader)
{
//	if (!LinuxSystemEmulationModel::InstallPlatform(loader))
//		return false;

	return InstallBootloader(loader);
}

bool RiscVSifiveFU540EmulationModel::InstallBootloader(loader::BinaryLoader &loader)
{
	return true;
}

bool RiscVSifiveFU540EmulationModel::PrepareCore(archsim::core::thread::ThreadInstance& core)
{
	// invoke reset exception
	core.GetArch().GetISA("riscv").GetBehaviours().GetBehaviour("riscv_reset").Invoke(&core, {});

	// run 'bootloader'
	auto pc_ptr = core.GetRegisterFileInterface().GetEntry<uint64_t>("PC");
	*pc_ptr = 0x80000000;

	auto &a0 = core.GetRegisterFileInterface().GetEntry<uint64_t>("GPR")[10];
	auto &a1 = core.GetRegisterFileInterface().GetEntry<uint64_t>("GPR")[11];
	a0 = 0x10000;
	a1 = 0x10000;

	return true;
}

RegisterComponent(archsim::abi::EmulationModel, RiscVSifiveFU540EmulationModel, "riscv64-sifive-fu540", "SiFive FU540-C000");
