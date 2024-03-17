/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>

// it is like the "#include" for libraries, but this is for externs.
extern DR_PIFO_scheduler<T1,T2> {
    DR_PIFO_scheduler(bit<1> verbose); 
void my_scheduler(in T1 in_flow_id, in T1 number_of_levels_used, in T1 in_pred, in T1 in_arrival_time, in T2 in_shaping, in T2 in_enq, in T1 in_pkt_ptr, in T2 in_deq, in T2 in_use_updated_rank, in T2 in_force_deq, in T1 in_force_deq_flow_id, in T2 in_enable_error_correction, in T2 reset_time);
void pass_rank_values ( in T1 rank_value, in T1 level_id);
void pass_updated_rank_values ( in T1 rank_value, in T1 flow_id, in T1 level_id);
}

extern floor_extern<T1> {
   	floor_extern(bit<1> verbose2); 
void floor ( in T1 nom, in T1 dom, inout T1 result);
}


const bit<16> TYPE_IPV4 = 0x800;
register<bit<48>>(1) current_dequeue_cycle;


/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/

typedef bit<9>  egressSpec_t;
typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;

header ethernet_t {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16>   etherType;
}

header ipv4_t {
    bit<4>    version;
    bit<4>    ihl;
    bit<6>    diffserv;
    bit<2>    ecn;
    bit<16>   totalLen;
    bit<16>   identification;
    bit<3>    flags;
    bit<13>   fragOffset;
    bit<8>    ttl;
    bit<8>    protocol;
    bit<16>   hdrChecksum;
    ip4Addr_t srcAddr;
    ip4Addr_t dstAddr;
    bit<32>  options;
}

header ipv4_option_t {
    bit<1> copyFlag;
    bit<2> optClass;
    bit<5> option;
    bit<8> optionLength;
}

struct metadata {
}

struct headers {
    ethernet_t   ethernet;
    ipv4_t       ipv4;
}

/*************************************************************************
*********************** P A R S E R  ***********************************
*************************************************************************/

parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            TYPE_IPV4: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition accept;
    }
}


/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {  }
}



/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {
    action drop() {
        mark_to_drop(standard_metadata);
    }

	// forwarding Action
    action ipv4_forward(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

	// forwarding table
    table ipv4_lpm {
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        actions = {
            ipv4_forward;
            drop;
            NoAction;
        }
        size = 1024;
        default_action = NoAction();
    }

	// the hierarchical scheduler extern instantiation
    @userextern @name("my_DR_PIFO")
    DR_PIFO_scheduler<bit<48>,bit<1>>(1) my_DR_PIFO;


    @userextern @name("floor_extern_obj")
    floor_extern<bit<48>>(1) floor_extern_obj;

	// Input Data
    bit <48> level_0_rank;
    bit <48> level_1_rank;
    bit <48> eligible_time			= 200000;
    bit <48> in_arrival_time		= 0;
    bit <48> in_pkt_ptr;
    bit <48> in_force_deq_flow_id	= 0;
    bit <48> update_flow_id			= 0;
    bit <48> updated_rank			= 0;
    bit <48> number_of_levels_used 	= 2;
    bit <48> in_flow_id 			= 0;

	// Control Signals
    bit <1> in_shaping = 1; // shaping is ON, because of the nature of our environment, because we need to enforce some traffic congestion to test the scheduling policies performance. But, it should be OFF because there is no traffic shaping policies applied.
    bit <1> in_enq = 1;
    bit <1> in_deq = 0; // it is always 0, and it should be removed.
    bit <1> in_use_updated_rank = 0;
    bit <1> in_force_deq = 0;
    bit <1> in_enable_error_correction = 0;

    bit <1> reset_time = 0;

    bit <48> current_dequeue_cycle_value;

    register<bit<48>>(1) register_last_ptr;


	// Action to assign the Flow ID for each packet
    action assign_flow_id(bit <48> flow_id) {
        in_flow_id = flow_id;
    }

	// Lookup Table for the Flow ID assignment
    table lookup_flow_id {
        key = {
            hdr.ipv4.srcAddr: lpm;
        }
        actions = {
            assign_flow_id;
            NoAction;
        }
        size = 1024;
        default_action = NoAction();
    }

	// DRR parameters

	register<bit<48>>(8) all_received_bytes;
	bit <48> all_received_bytes_value;
	bit <48> quota_value = 3000; // bytes
	bit <48> max_possible_sent_pkts = 16; // = number_of_flows * floor(Quantum / min_pkt_size) = 8 * floor(3000/1500 bytes/bytes), 8 as 8 flow IDs in level 2
	bit <48> flow_id_at_level_1 = 0;

    apply {

		// assign a flow ID for each arrived packet
        lookup_flow_id.apply();

		// increment a counter to be used to point to each packet
        register_last_ptr.read(in_pkt_ptr,0);
        in_pkt_ptr = in_pkt_ptr + (bit<48>)(1);
        register_last_ptr.write(0,in_pkt_ptr);    
		
	// group multiple flows into 1 queue at the lowest level (leaf queues), OPTIONAL
	// because we have 72 flows, but we only have 8 leaf queues.
	floor_extern_obj.floor(in_flow_id, (bit<48>)(10), flow_id_at_level_1);
	
	reset_time = 0;
	in_arrival_time = standard_metadata.ingress_global_timestamp;
		
	// DRR : rank computations
	all_received_bytes.read(all_received_bytes_value, (bit<32>)(flow_id_at_level_1));
	current_dequeue_cycle.read(current_dequeue_cycle_value, 0);
		
	if(all_received_bytes_value < (current_dequeue_cycle_value * quota_value))
	{		
		all_received_bytes_value = current_dequeue_cycle_value * quota_value;
	}

	all_received_bytes_value = all_received_bytes_value + (bit<48>)(hdr.ipv4.options);
	all_received_bytes.write((bit<32>)(flow_id_at_level_1), all_received_bytes_value);
	floor_extern_obj.floor(all_received_bytes_value - (bit<48>)(1), quota_value, level_0_rank);
		
	// round id 
	standard_metadata.ingress_global_timestamp = level_0_rank; 
	// Saving the round ID in this standard metadata, because the replaced standard metadata won't be needed later, and it reserve space anyways.

	level_0_rank = (level_0_rank * max_possible_sent_pkts) + flow_id_at_level_1 + 1;

	// SP : rank computations
	// level_1_rank = hdr.ipv4.diffserv; // this is more generic, but here we use the flow ID to determine the Strict Priority
        if(in_flow_id < (bit<48>)(40))
        {
            level_1_rank = 1; // high priority, low rank
        }
        else
        {
            level_1_rank = 2; // low priority, high rank
        }
		
        // pass the ranks of the processed packet to the hierarchical scheduler.
	// each rank with the corresponding level ID.
	my_DR_PIFO.pass_rank_values(level_0_rank,0);
	my_DR_PIFO.pass_rank_values(level_1_rank,1);

        if(hdr.ipv4.dstAddr == 0)
        {
            drop();        
        }
        else
        {	
		in_flow_id = flow_id_at_level_1;
			
		// pass the input data and control signals to the hierarchical scheduler extern.
        	my_DR_PIFO.my_scheduler(in_flow_id, number_of_levels_used, eligible_time, in_pkt_ptr, in_shaping, in_enq, in_pkt_ptr, in_deq, in_use_updated_rank, in_force_deq, in_force_deq_flow_id, in_enable_error_correction, reset_time);
        }
        
        if (hdr.ipv4.isValid()) {   
           ipv4_lpm.apply();
        }  
    }
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {
    apply { 
		bit <48> current_dequeue_cycle_value;

		current_dequeue_cycle.read(current_dequeue_cycle_value, 0);
		if(current_dequeue_cycle_value < standard_metadata.ingress_global_timestamp)
		{
			current_dequeue_cycle_value = standard_metadata.ingress_global_timestamp;
			current_dequeue_cycle.write(0 , current_dequeue_cycle_value);
		}
	}
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control MyComputeChecksum(inout headers hdr, inout metadata meta) {
     apply {
	update_checksum(
	    hdr.ipv4.isValid(),
            { hdr.ipv4.version,
	      hdr.ipv4.ihl,
	      hdr.ipv4.diffserv,
	      hdr.ipv4.ecn,
              hdr.ipv4.totalLen,
              hdr.ipv4.identification,
              hdr.ipv4.flags,
              hdr.ipv4.fragOffset,
              hdr.ipv4.ttl,
              hdr.ipv4.protocol,
              hdr.ipv4.srcAddr,
              hdr.ipv4.dstAddr },
            hdr.ipv4.hdrChecksum,
            HashAlgorithm.csum16);
    }
}

/*************************************************************************
***********************  D E P A R S E R  *******************************
*************************************************************************/

control MyDeparser(packet_out packet, in headers hdr) {
    apply {
        packet.emit(hdr.ethernet);
        packet.emit(hdr.ipv4);
    }
}

/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

V1Switch(
MyParser(),
MyVerifyChecksum(),
MyIngress(),
MyEgress(),
MyComputeChecksum(),
MyDeparser()
) main;
