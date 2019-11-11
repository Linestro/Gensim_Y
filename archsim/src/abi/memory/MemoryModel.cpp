/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * abi/memory/MemoryModel.cpp
 */
#include "define.h"

#include "abi/memory/MemoryTranslationModel.h"
#include "abi/memory/MemoryModel.h"
#include "abi/memory/MemoryEventHandler.h"

#include "util/LogContext.h"
#include "util/ComponentManager.h"
#include "abi/devices/MMU.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstddef>

#include <iomanip>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/stat.h>

DeclareLogContext(LogMemoryModel, "Memory");

#define HOST_PAGE_SIZE_BITS 12  // 4K Host Pages
#define HOST_PAGE_SIZE (1 << HOST_PAGE_SIZE_BITS)
#define HOST_PAGE_INDEX_MASK (~(HOST_PAGE_SIZE - 1))
#define HOST_PAGE_OFFSET_MASK (HOST_PAGE_SIZE - 1)

#define GUEST_PAGE_SIZE_BITS 12  // 4K Guest Pages
#define GUEST_PAGE_SIZE (1 << GUEST_PAGE_SIZE_BITS)
#define GUEST_PAGE_INDEX_MASK (~(GUEST_PAGE_SIZE - 1))
#define GUEST_PAGE_OFFSET_MASK (GUEST_PAGE_SIZE - 1)
#define GUEST_PAGE_INDEX(_addr) ((_addr &GUEST_PAGE_INDEX_MASK) >> GUEST_PAGE_SIZE_BITS)
#define GUEST_PAGE_ADDRESS(_addr) (_addr &GUEST_PAGE_INDEX_MASK)
#define GUEST_PAGE_OFFSET(_addr) (_addr &GUEST_PAGE_OFFSET_MASK)

#include <unistd.h>

using namespace archsim::abi::memory;

DefineComponentType(MemoryModel);

static ComponentDescriptor mem_model_descriptor("MemoryModel");

MappingManager::~MappingManager() {}

MemoryModel::MemoryModel() : Component(mem_model_descriptor) {}

MemoryModel::~MemoryModel() {}

bool MemoryModel::InsertFile(guest_addr_t addr, std::string filename, uint32_t& size)
{
	struct stat stat;

	// Open the supplied file for reading.
	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		LC_ERROR(LogMemoryModel) << "Unable to open file: " << filename;
		return false;
	}

	// Stat the file, so we know how big it is.
	fstat(fd, &stat);

	// Refuse to copy a file that's bigger than 'size'
	if (size && (stat.st_size > size)) {
		close(fd);
		return false;
	}

	// Map the file into memory.
	void *file_data = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	// If the mapping failed, exit.
	if (file_data == MAP_FAILED)
		return false;

	// Copy the data from the mapping into guest memory.
	Write(addr, (uint8_t *)file_data, stat.st_size);

	// Release the file mapping.
	munmap(file_data, stat.st_size);

	size = stat.st_size;
	return true;
}


bool MemoryModel::RaiseEvent(MemoryEventType type, guest_addr_t addr, uint8_t size)
{
	UNIMPLEMENTED;
}

bool MemoryModel::LockRegion(guest_addr_t guest_addr, guest_size_t guest_size, host_addr_t& host_addr)
{
	return false;
}

bool MemoryModel::LockRegions(guest_addr_t guest_addr, guest_size_t guest_size, LockedMemoryRegion& regions)
{
	return false;
}

bool MemoryModel::UnlockRegion(guest_addr_t guest_addr, guest_size_t guest_size, host_addr_t host_addr)
{
	return false;
}

void MemoryModel::FlushCaches()
{

}

void MemoryModel::EvictCacheEntry(Address virt_addr)
{

}

uint32_t MemoryModel::PerformTranslation(Address virt_addr, Address &out_phys_addr, const struct abi::devices::AccessInfo &info)
{
	out_phys_addr = virt_addr;
	return 0;
}

bool MemoryModel::GetMemoryUsage(MemoryUsageInfo& usage)
{
	return false;
}

bool MemoryModel::HandleSegFault(host_const_addr_t host_addr)
{
	return false;
}

MappingManager *MemoryModel::GetMappingManager()
{
	return NULL;
}

uint32_t MemoryModel::Read8(guest_addr_t addr, uint8_t &data)
{
	return Read(addr, &data, 1);
}

uint32_t MemoryModel::Read8_zx(guest_addr_t addr, uint32_t &data)
{
	uint8_t local_data;
	uint32_t rc = Read8(addr, local_data);
	data = (uint32_t)local_data;
	return rc;
}

uint32_t MemoryModel::Read8_sx(guest_addr_t addr, uint32_t &data)
{
	uint8_t local_data;
	uint32_t rc = Read8(addr, local_data);
	data = (uint32_t)(int32_t)(int8_t)local_data;
	return rc;
}

uint32_t MemoryModel::Read16(guest_addr_t addr, uint16_t &data)
{
	return Read(addr, (uint8_t*)&data, 2);
}

uint32_t MemoryModel::Read16_zx(guest_addr_t addr, uint32_t &data)
{
	uint16_t local_data;
	uint32_t rc = Read16(addr, local_data);
	data = (uint32_t)local_data;
	return rc;
}

uint32_t MemoryModel::Read16_sx(guest_addr_t addr, uint32_t &data)
{
	uint16_t local_data;
	uint32_t rc = Read16(addr, local_data);
	data = (uint32_t)(int32_t)(int16_t)local_data;
	return rc;
}

uint32_t MemoryModel::Read32(guest_addr_t addr, uint32_t &data)
{
	return Read(addr, (uint8_t*)&data, 4);
}

uint32_t MemoryModel::Read64(guest_addr_t addr, uint64_t &data)
{
	return Read(addr, (uint8_t*)&data, 8);
}

uint32_t MemoryModel::Read128(guest_addr_t addr, uint128_t &data)
{
	return Read(addr, (uint8_t*)&data, 16);
}

uint32_t MemoryModel::Fetch8(guest_addr_t addr, uint8_t &data)
{
	return Fetch(addr, &data, 1);
}

uint32_t MemoryModel::Fetch16(guest_addr_t addr, uint16_t &data)
{
	return Fetch(addr, (uint8_t*)&data, 2);
}

uint32_t MemoryModel::Fetch32(guest_addr_t addr, uint32_t &data)
{
	return Fetch(addr, (uint8_t*)&data, 4);
}

uint32_t MemoryModel::Write8(guest_addr_t addr, uint8_t data)
{
	uint32_t temp = Write(addr, &data, 1);
	return temp;
}

uint32_t MemoryModel::Write16(guest_addr_t addr, uint16_t data)
{
	return Write(addr, (uint8_t*)&data, 2);
}

uint32_t MemoryModel::Write32(guest_addr_t addr, uint32_t data)
{
	return Write(addr, (uint8_t*)&data, 4);
}

uint32_t MemoryModel::Write64(guest_addr_t addr, uint64_t data)
{
	return Write(addr, (uint8_t*)&data, 8);
}

uint32_t MemoryModel::Write128(guest_addr_t addr, uint128_t data)
{
	return Write(addr, (uint8_t*)&data, 16);
}

uint32_t MemoryModel::Read8User(guest_addr_t addr, uint32_t&data)
{
	return Read8_zx(addr, data);
}

uint32_t MemoryModel::Read32User(guest_addr_t addr, uint32_t&data)
{
	return Read32(addr, data);
}

uint32_t MemoryModel::Write8User(guest_addr_t addr, uint8_t data)
{
	return Write8(addr, data);
}

uint32_t MemoryModel::Write32User(guest_addr_t addr, uint32_t data)
{
	return Write32(addr, data);
}

uint32_t MemoryModel::ReadString(guest_addr_t addr, char *str, int size)
{
	int offset = 0;
	uint8_t c;

	do {
		if (Read8(addr + offset, c))
			return 1;

		*str++ = c;
		offset++;
	} while ((c != 0) && (offset < size));

	return 0;
}

uint32_t MemoryModel::WriteString(guest_addr_t addr, const char *str)
{
	return Write(addr, (uint8_t *)str, strlen(str) + 1);
}

uint32_t MemoryModel::ReadN(guest_addr_t addr, uint8_t *buffer, size_t size)
{
	while(size) {
		uint32_t c = Read8(addr, *buffer);
		if(c) return c;
		buffer++;
		addr += 1;
		size--;
	}
	return 0;
}

uint32_t MemoryModel::WriteN(guest_addr_t addr, uint8_t *buffer, size_t size)
{
	while(size) {
		uint32_t c = Write8(addr, *buffer);
		if(c) return c;
		buffer++;
		addr += 1;
		size--;
	}
	return 0;
}

uint32_t MemoryModel::Peek32(guest_addr_t addr, uint32_t &data)
{
	return Peek(addr, (uint8_t*)&data, 4);
}

uint32_t MemoryModel::Peek32Unsafe(guest_addr_t addr)
{
	uint32_t data;
	Peek32(addr, data);
	return data;
}

static struct GuestVMA null_vma = { 0, archsim::Address(0), 0, RegFlagNone, "NULL" };
RegionBasedMemoryModel::RegionBasedMemoryModel() : cached_vma(&null_vma) {}

RegionBasedMemoryModel::~RegionBasedMemoryModel()
{
	//Free guest VMA info
	for(auto v : guest_vmas) {
		delete(v.second);
	}
}

MappingManager *RegionBasedMemoryModel::GetMappingManager()
{
	return this;
}

bool RegionBasedMemoryModel::GetMemoryUsage(MemoryUsageInfo& usage)
{
	if (!MemoryModel::GetMemoryUsage(usage))
		return false;

	usage.guest.total = 0;
	for (const auto& xvma : guest_vmas) {
		usage.guest.total += xvma.second->size;
	}
	return true;
}

bool RegionBasedMemoryModel::DeleteVMA(guest_addr_t addr)
{
	for (GuestVMAMap::const_iterator VI = guest_vmas.begin(), VE = guest_vmas.end(); VI != VE; ++VI) {
		if (addr >= VI->second->base && addr < (VI->second->base + VI->second->size)) {
			if(VI->second == cached_vma) cached_vma = &null_vma;
			guest_vmas.erase(VI);
			return true;
		}
	}

	return false;
}

GuestVMA *RegionBasedMemoryModel::LookupVMA(guest_addr_t addr)
{
	if(guest_vmas.empty()) {
		return  nullptr;
	}

	GuestVMAMap::const_iterator iter = guest_vmas.upper_bound(addr);

	iter--;
	GuestVMA *vma = iter->second;
	if ((addr >= vma->base) && (addr < (vma->base + vma->size))) {
		cached_vma = vma;
		return vma;
	}

	return NULL;
}

bool RegionBasedMemoryModel::AllocateRegion(archsim::Address addr, uint64_t size)
{
	if(allocated_regions_.count(addr)) {
		throw std::logic_error("Attempting to reallocate a region");
	}
	LC_DEBUG1(LogMemoryModel) << "Allocating a region " << addr << " (" << std::hex << size << ")";
	allocated_regions_[addr] = size;
	MergeAllocatedRegions();

	return true;
}

bool RegionBasedMemoryModel::DeallocateRegion(archsim::Address addr, uint64_t size)
{
	auto iterator = allocated_regions_.lower_bound(addr);

	LC_DEBUG1(LogMemoryModel) << "Deallocating a region " << addr << " (" << std::hex << size << ")";

	// iterator points to either a region which has an address >= addr
	// therefore (addr, addr+size) might overlap it.
	if(addr + size > (iterator->first + iterator->second)) {
		UNIMPLEMENTED;
	} else {
		// we didn't find a region which was allocated
	}

	MergeAllocatedRegions();

	return true;
}

bool RegionBasedMemoryModel::MergeAllocatedRegions()
{
	std::vector<archsim::Address> removed_regions;
	if(allocated_regions_.empty()) {
		return true;
	}

	LC_DEBUG1(LogMemoryModel) << "Merging regions: ";
	for(auto i : allocated_regions_) {
		LC_DEBUG1(LogMemoryModel) << " - " << i.first << " (" << std::hex << i.second << ")";
	}

	auto iterator = allocated_regions_.begin();
	while(iterator != allocated_regions_.end()) {
		auto cur = iterator;
		auto next = iterator;
		next++;

		while(next != allocated_regions_.end() && cur->first + cur->second >= next->first) {
			LC_DEBUG1(LogMemoryModel) << "Merging region " << next->first << " into " << cur->first;
			auto next_end = (next->first + next->second).Get();
			cur->second = next_end - cur->first.Get();
			removed_regions.push_back(next->first);

			LC_DEBUG1(LogMemoryModel) << " - Final region: " << cur->first << " (" << std::hex << cur->second << ")";
			next++;
		}
		iterator = next;
	}

	for(auto i : removed_regions) {
		allocated_regions_.erase(i);
	}

	LC_DEBUG1(LogMemoryModel) << "Merged regions: ";
	for(auto i : allocated_regions_) {
		LC_DEBUG1(LogMemoryModel) << " - " << i.first << " (" << std::hex << i.second << ")";
	}

	return true;
}


bool RegionBasedMemoryModel::HasIntersectingRegions(guest_addr_t addr, guest_size_t size)
{
	guest_addr_t start1 = addr;
	guest_addr_t end1 = addr + size;

	for (auto region : allocated_regions_) {
		guest_addr_t start2 = region.first;
		guest_addr_t end2 = region.first + region.second;

		//(Start1 <= End2) and (Start2 <= End1)

		if (start1 < end2 && start2 < end1) {
			return true;
		}
	}

	return false;
}

bool RegionBasedMemoryModel::VMAIntersects(GuestVMA& vma, guest_size_t size)
{
	guest_addr_t start1 = vma.base;
	guest_addr_t end1 = start1 + vma.size;

	for (auto region : guest_vmas) {
		if (region.second == &vma)
			continue;

		guest_addr_t start2 = region.second->base;
		guest_addr_t end2 = start2 + region.second->size;

		//(Start1 <= End2) and (Start2 <= End1)

		if (start1 < end2 && start2 < end1) {
			return true;
		}
	}

	return false;
}


bool RegionBasedMemoryModel::MapAll(RegionFlags prot)
{
	assert(guest_vmas.empty());

	LC_DEBUG1(LogMemoryModel) << "Map All";

	GuestVMA *vma = new GuestVMA();

	vma->base = Address(0);
	vma->size = 0xffffffff;
	vma->protection = prot;
	vma->name = "all";

	if (!AllocateVMA(*vma)) {
		delete vma;
		return false;
	}

	guest_vmas[Address(0)] = vma;

	return true;
}

bool RegionBasedMemoryModel::MapRegion(guest_addr_t addr, guest_size_t size, RegionFlags prot, std::string name)
{
	size = AlignUp(Address(size)).Get();

	LC_DEBUG1(LogMemoryModel) << "Map Region: addr = " << std::hex << addr << ", size = " << std::hex << size << ", prot = " << prot << ", name = " << name;

	if (HasIntersectingRegions(addr, size)) {
		LC_DEBUG1(LogMemoryModel) << " - Overlap detected";
		UnmapSubregion(addr, size);
	}

	GuestVMA *vma = new GuestVMA();

	vma->base = addr;
	vma->size = size;
	vma->protection = prot;
	vma->name = name;

	if (!AllocateVMA(*vma)) {
		delete vma;
		return false;
	}

	guest_vmas[addr] = vma;
	AllocateRegion(addr, size);

	return true;
}

guest_addr_t RegionBasedMemoryModel::MapAnonymousRegion(guest_size_t size, RegionFlags prot)
{
	// Force size alignment.
	size = AlignUp(Address(size)).Get();

	LC_DEBUG1(LogMemoryModel) << "Map anonymous region: size = " << std::hex << size << ", prot = " << prot;

	// Start from a (random?) base address, and find the first address that doesn't
	// contain an intersecting region.
	guest_addr_t addr = Address(0x40000000);
	while (HasIntersectingRegions(addr, size)) {
		addr += 4096;
	}

	// Attempt to map it.
	if (MapRegion(addr, size, prot, "")) {
		return addr;
	} else {
		LC_ERROR(LogMemoryModel) << "Failed to map anonymous region!";
		return Address(-1);
	}
}

bool RegionBasedMemoryModel::RemapRegion(guest_addr_t addr, guest_size_t size)
{
	GuestVMA *vma = LookupVMA(addr);

	if (vma == NULL) {
		LC_ERROR(LogMemoryModel) << "Attempt to remap non-mapped region";
		return false;
	}

	// Force size alignment.
	size = AlignUp(Address(size)).Get();

	// Check for overlapping regions.
	if (VMAIntersects(*vma, size)) {
		return false;
	}

	DeallocateRegion(addr,vma->size);
	AllocateRegion(addr, size);

	LC_DEBUG1(LogMemoryModel) << "Remap Region: addr = " << std::hex << vma->base << ", real addr = " << std::hex << vma->host_base << ", old size = " << std::hex << vma->size << ", new size = " << std::hex << size;
	return ResizeVMA(*vma, size);
}

bool RegionBasedMemoryModel::UnmapRegion(guest_addr_t addr, guest_size_t size)
{
	GuestVMA *vma = LookupVMA(addr);

	if (vma == NULL) {
		LC_ERROR(LogMemoryModel) << "Attempt to unmap non-mapped region.";
		return false;
	}

	// Force size alignment.
	size = AlignUp(Address(size)).Get();

	LC_DEBUG1(LogMemoryModel) << "Unmap Region: addr = " << std::hex << vma->base << ", real addr = " << std::hex << vma->host_base << ", size = " << std::hex << vma->size;

	if (!DeallocateVMA(*vma)) return false;
	DeallocateRegion(addr, size);

	DeleteVMA(addr);
	return true;
}

bool RegionBasedMemoryModel::UnmapSubregion(guest_addr_t addr, guest_size_t size)
{
	Address u_start = addr;
	Address u_end = addr + size;

	LC_DEBUG1(LogMemoryModel) << "Subregion " << u_start << " to " << u_end;

	for(auto &i : guest_vmas) {
		auto i_start = Address(i.second->base);
		auto i_end = Address(i.second->base + i.second->size);

		LC_DEBUG2(LogMemoryModel) << "Checking against " << i_start << " to " << i_end;

		// 6 possible situations:
		// 1. i is completely before the unmapped subregion
		//    - do nothing
		// 2. i is completely covered by the unmapped subregion
		//    - unmap i
		// 3. i overlaps the start of the unmapped subregion
		//    - move the start of i to the end of the unmapped subregion
		// 4. i overlaps the middle of the unmapped subregion
		//    - resize i to end at the start of the unmapped region
		//    - create a new guest vma after the end of the unmapped region
		// 5. i overlaps the end of the unmapped subregion
		//    - move the start of i to the end of the unmapped region
		// 6. i is completely after the unmapped subregion
		//    - do nothing

		// 2:
		if(i_start >= u_start && u_end >= i_end) {
			LC_DEBUG1(LogMemoryModel) << "Unmapping entire region and recursing";
			UnmapRegion(addr, size);
			return UnmapSubregion(addr, size);
		}

		// 3:
		if(i_start < u_start && i_end > u_start && i_end < u_end) {
			LC_DEBUG1(LogMemoryModel) << "Shifting start of region";
			i.second->base = u_end;
		}

		// 4:
		if(u_start > i_start && u_end < i_end) {
			LC_DEBUG1(LogMemoryModel) << "Splitting existing vma and recursing";
			// shift the end of the current vma and also create a new one, then recurse
			auto new_size = u_start - i_start;
			i.second->size = new_size.Get();

			MapRegion(u_end, (i_end - u_end).Get(), i.second->protection, i.second->name);
			return UnmapSubregion(addr, size);
		}

		// 5:
		if(i_start > u_start && i_start < u_end) {
			UNIMPLEMENTED;
		}
	}

	return true;
}


bool RegionBasedMemoryModel::ProtectRegion(guest_addr_t addr, guest_size_t size, RegionFlags prot)
{
	// Force size alignment.
	size = AlignUp(Address(size)).Get();

	LC_DEBUG1(LogMemoryModel) << "Protect Region: addr = " << std::hex << addr << ", size = " << std::hex << size << ", prot = " << prot;

	// Locate the enclosing VMA, and update protections.
	auto vma = guest_vmas.find(addr);
	if (vma == guest_vmas.end()) return false;

	vma->second->protection = prot;

	return SynchroniseVMAProtection(*vma->second);
}

bool RegionBasedMemoryModel::GetRegionProtection(guest_addr_t addr, RegionFlags& prot)
{
	// Locate the enclosing VMA, and retrieve protections.
	GuestVMA *vma = LookupVMA(addr);
	if (vma == NULL) return false;

	prot = vma->protection;
	return true;
}

void RegionBasedMemoryModel::DumpRegions()
{
	fprintf(stderr, "Memory Regions:\n");

	// Enumerate each region, and print out region metadata.
	for (auto region : guest_vmas) {
		char pfr = '-';
		char pfw = '-';
		char pfx = '-';
		char pfp = '-';

		if (region.second->protection & RegFlagRead) pfr = 'r';
		if (region.second->protection & RegFlagWrite) pfw = 'w';
		if (region.second->protection & RegFlagExecute) pfx = 'x';

		fprintf(stderr, "%016lx %08x-%08x (%08x) %c%c%c%c %s\n", (unsigned long)region.second->host_base, region.second->base, region.second->base + region.second->size, region.second->size, pfr, pfw, pfx, pfp, region.second->name.c_str());
	}
}

bool RegionBasedMemoryModel::ResolveGuestAddress(host_const_addr_t host_addr, guest_addr_t &guest_addr)
{
	for (auto region : guest_vmas) {
		if (host_addr >= (host_const_addr_t)region.second->host_base && host_addr < (host_const_addr_t)((unsigned long)region.second->host_base + region.second->size)) {
			guest_addr = (guest_addr_t)((unsigned long)host_addr - (unsigned long)region.second->host_base);
			return true;
		}
	}
	return false;
}

#if CONFIG_LLVM
class NullMemoryTranslationModel : public MemoryTranslationModel
{
public:
	NullMemoryTranslationModel() {}
	~NullMemoryTranslationModel() {}
};
#endif

NullMemoryModel::NullMemoryModel()
{
#if CONFIG_LLVM
	translation_model = new NullMemoryTranslationModel();
#else
	translation_model = nullptr;
#endif
}

NullMemoryModel::~NullMemoryModel()
{

}

bool NullMemoryModel::Initialise()
{
	return true;
}

void NullMemoryModel::Destroy()
{

}

uint32_t NullMemoryModel::Read(guest_addr_t addr, uint8_t *data, int size)
{
#ifdef CONFIG_MEMORY_EVENTS
	RaiseEvent(MemoryModel::MemEventRead, addr, size);
#endif
	return 1;
}

uint32_t NullMemoryModel::Fetch(guest_addr_t addr, uint8_t *data, int size)
{
#ifdef CONFIG_MEMORY_EVENTS
	RaiseEvent(MemoryModel::MemEventFetch, addr, size);
#endif
	return 1;
}

uint32_t NullMemoryModel::Write(guest_addr_t addr, uint8_t *data, int size)
{
#ifdef CONFIG_MEMORY_EVENTS
	RaiseEvent(MemoryModel::MemEventWrite, addr, size);
#endif
	return 1;
}

uint32_t NullMemoryModel::Peek(guest_addr_t addr, uint8_t *data, int size)
{
	return 1;
}

uint32_t NullMemoryModel::Poke(guest_addr_t addr, uint8_t *data, int size)
{
	return 1;
}

bool NullMemoryModel::ResolveGuestAddress(host_const_addr_t host_addr, guest_addr_t &guest_addr)
{
	return false;
}

bool NullMemoryModel::HandleSegFault(host_const_addr_t host_addr)
{
	return false;
}

MemoryTranslationModel &NullMemoryModel::GetTranslationModel()
{
	return *translation_model;
}

uint32_t NullMemoryModel::PerformTranslation(Address virt_addr, Address &out_phys_addr, const struct archsim::abi::devices::AccessInfo &info)
{
	out_phys_addr = virt_addr;
	return 0;
}
