/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * File:   Region.h
 * Author: s0457958
 *
 * Created on 18 July 2014, 13:26
 */

#ifndef REGION_H
#define	REGION_H

#include "core/thread/ThreadInstance.h"
#include "translate/profile/RegionArch.h"
#include "util/SimOptions.h"
#include "util/Lifetime.h"
#include "util/Zone.h"

#include <cassert>
#include <cstdint>

#include <atomic>
#include <unordered_map>
#include <mutex>
#include <unordered_set>

namespace gensim
{
	class Processor;
}

namespace archsim
{
	namespace translate
	{
		class Translation;
		class TranslationManager;

		namespace profile
		{
			class Block;

			/**
			 * Describes a region of PHYSICAL memory.
			 * This contains control flow information for a region of physical memory,
			 * with each block in the region being VIRTUALLY addressed. This also tracks
			 * the 'heat' of the region in order to determine if it is suitable for translation.
			 */
			class Region : public util::ReferenceCounted
			{
				friend class Block;

			public:
				friend std::ostream& operator<< (std::ostream& out, const archsim::translate::profile::Region& rgn);

				enum RegionStatus {
					NotInTranslation,
					QueuedForTranslation,
					InTranslation,
				};

				Region(const Region&) = delete;
				Region(TranslationManager& mgr, Address phys_base_addr);
				virtual ~Region();

				/**
				 * Get a block by its VIRTUAL address. This will insert a new block if none could
				 * be found
				 */
				Block& GetBlock(Address virt_addr, uint8_t isa_mode);

				/**
				 * Determines whether or not this physical region has any translations.
				* @return Returns true if this physical region has any translations, false otherwise.
				*/
				bool HasTranslations() const;

				inline void SetStatus(RegionStatus new_status)
				{
					status = new_status;
				}

				inline const RegionStatus &GetStatus() const
				{
					return status;
				}

				/**
				 * Records the execution of a block, given its VIRTUAL address in this PHYSICAL region.
				 * @param virt_addr The virtual address of the block that has been executed.
				*/
				void TraceBlock(archsim::core::thread::ThreadInstance *thread, Address virt_addr);

				void Invalidate();

				void EraseBlock(Address virt_addr);

				void InvalidateHeat();

				inline void IncrementGeneration()
				{
					max_generation++;
				}

				inline uint32_t GetMaxGeneration() const
				{
					return max_generation;
				}

				inline uint32_t GetCurrentGeneration() const
				{
					return current_generation;
				}

				inline void SetCurrentGeneration(uint32_t generation)
				{
					current_generation = generation;
				}

				inline Address GetPhysicalBaseAddress() const
				{
					return phys_base_addr;
				}

				inline bool IsHot(uint32_t hotspot_threshold) const
				{
					return max_block_interp_count_ >= hotspot_threshold;
				}

				inline bool IsValid() const
				{
					return !invalid_;
				}

				void dump();
				void dump_dot();

				uint64_t GetTotalInterpCount() const
				{
					return total_interp_count_;
				}

			public:
				size_t GetApproximateMemoryUsage() const;

				std::unordered_set<Address> virtual_images;

				/*
				 * Map of page offsets to block interpretation counts
				 */
				typedef std::map<Address, Block*> block_map_t;

				/**
				 * Map of page offsets to blocks
				 */
				block_map_t blocks;

				util::Zone<Block, 2048> block_zone;

				/*
				 * Pointer to a translation for this region if one exists
				 */
				Translation *txln;

			private:
				uint64_t max_block_interp_count_;
				uint64_t total_interp_count_;

				TranslationManager& mgr;

				Address phys_base_addr;

				uint32_t max_generation;
				uint32_t current_generation;

				RegionStatus status;

				bool invalid_;

			};
		}
	}
}

#endif	/* REGION_H */
