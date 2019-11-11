/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */


/*
 * File:   ThreadManager.h
 * Author: harry
 *
 * Created on 10 April 2018, 16:14
 */

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <arch/ArchDescription.h>

namespace archsim
{


	/**
	 * This class manages the execution of threads for a single guest
	 * architecture. These threads might be from the same core or different
	 * cores, or from cores in different configurations.
	 *
	 * This class mainly exists to bridge the gap between guest thread
	 * instances and execution engines.
	 */
	class ThreadManager
	{
	public:



	private:
		std::vector<ThreadInstance*> thread_instances_;
		gensim::arch::ArchDescription &arch_descriptor_;
	};

}

#endif /* THREADMANAGER_H */

