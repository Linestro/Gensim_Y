/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */
/*
 * TraceManager.cpp
 *
 *  Created on: 8 Aug 2014
 *      Author: harry
 */

/*
#include "gensim/gensim_decode.h"
#include "gensim/gensim_processor.h"

#include "tracing/TraceTypes.h"
*/

#include "libtrace/TraceSink.h"
#include "libtrace/TraceSource.h"
#include "libtrace/ArchInterface.h"

#include <cstdint>
#include <cassert>
#include <errno.h>
#include <map>
#include <string>
#include <streambuf>
#include <string.h>
#include <stdio.h>
#include <vector>

using namespace libtrace;

TraceSource::TraceSource(uint32_t BufferSize)
	:
	is_terminated_(false),
	sink_(nullptr),
	aggressive_flushing_(true),
	packet_open_(false),
	skip_(0)
{
	packet_buffer_ = (TraceRecord*)malloc(PacketBufferSize * sizeof(TraceRecord));
	packet_buffer_end_ = packet_buffer_+PacketBufferSize;
	packet_buffer_pos_ = packet_buffer_;
}

TraceSource::~TraceSource()
{
	assert(is_terminated_);
}

void TraceSource::SetSink(TraceSink* sink)
{
	sink_ = sink;
	id_ = sink->Open();
}

void TraceSource::EmitPackets()
{
	sink_->SinkPackets(id_, packet_buffer_, packet_buffer_pos_);
	packet_buffer_pos_ = packet_buffer_;
}

void TraceSource::Terminate()
{
	is_terminated_ = true;
}



void TraceSource::Flush()
{
	EmitPackets();
	sink_->Flush();
}
