/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 */

#ifndef lint
static const char rcsid[] =
		"@(#) $Header: /cvsroot/nsnam/ns-2/tools/tskcomsink.cc,v 1.18 2000/09/01 03:04:06 haoboy Exp $ (LBL)";
#endif

#include <tclcl.h>

#include "agent.h"
#include "config.h"
#include "packet.h"
#include "ip.h"
#include "rtp.h"
#include "bytecounter.h"
#include "tskcomsink.h"

static class TskComSinkClass : public TclClass {
public:
	TskComSinkClass() : TclClass("Agent/TskComSink") {}
	TclObject* create(int, const char*const*) {
		return (new TskComSink());
	}
} class_tsk_comsink;

TskComSink::TskComSink() : Agent(PT_NTYPE)
{	
	bytes_ = 0;
	bytes_since_ = 0;
	nlost_ = 0;
	npkts_ = 0;
	expected_ = -1;
	last_packet_time_ = 0.;
	last_bytes_since_ = 0.;
	seqno_ = 0;
	bind("nlost_", &nlost_);
	bind("npkts_", &npkts_);
	bind("bytes_", &bytes_);
	bind("lastPktTime_", &last_packet_time_);
	bind("expected_", &expected_);
}

TskComSink::~TskComSink()
{
	res_provider_ = NULL;
}

void TskComSink::recv(Packet* pkt, Handler*)
{
	/* Get TskObject and start its execution */
	ResourceConsumer *recvTskObj = (ResourceConsumer*)hdr_cmn::access(pkt)->pt_obj_addr();
//	std::cerr << "Pointer recieved:" << recvTskObj << "\n";

	if (recvTskObj) { /* Valid pointer and can be executed */
//		std::cerr << "Task id:" << ((CloudTask*)recvTskObj)->id_ << "\n";
		if (res_provider_) res_provider_->recv(recvTskObj);
		else printf("Error: task is received but no ResourceProvider is attached\n");
	}

	bytes_ += hdr_cmn::access(pkt)->size();
	bytes_since_ += hdr_cmn::access(pkt)->size();
	++npkts_;

	last_packet_time_ = Scheduler::instance().clock();
	Packet::free(pkt);
}

void TskComSink::addResourceProvider(ResourceProvider *newrp)
{
	res_provider_  = newrp;
	res_provider_->setTskComSink(this);
}

/*
 * $proc interval $interval
 * $proc size $size
 */
int TskComSink::command(int argc, const char*const* argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "clear") == 0) {
			expected_ = -1;
			return (TCL_OK);
		}
	}
	if (argc == 3) {
		if (strcmp(argv[1], "connect-resprovider") == 0) {
			ResourceProvider *hst = dynamic_cast<ResourceProvider*> (TclObject::lookup(argv[2]));
			if(hst){
				addResourceProvider(hst);
				return (TCL_OK);
			}
			return (TCL_ERROR);
		}
	}

	return (Agent::command(argc, argv));
}


