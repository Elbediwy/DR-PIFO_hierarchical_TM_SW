
//////////////////////////////////////////////////// todo list
//////////////////////////////////////////////////// 1) force dequeue and error correction are not unsigned short introduced properly yet.
//////////////////////////////////////////////////// 2) add the arrival time in each packet (I am using the pkt_ptr instead in the enqueue_FS now)

#ifndef SIMPLE_SWITCH_PSA_DIV_H_
#define SIMPLE_SWITCH_PSA_DIV_H_
#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/extern.h>
#define _STDC_WANT_LIB_EXT1_ 1
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <numeric>
#include <chrono>
#include <iostream>
#include <queue>
#include <ctime>
using namespace std::chrono;
#pragma once

namespace bm {

// if the rank = 0, that means this level is not used (pkts will be handled in a FIFO order in this level), the lowest rank in any level is "1".

// this is the main class of the AR-PIFO scheduler that will be used later in any usage of the scheduler.
class DR_PIFO_scheduler : public bm::ExternType {
 public:
  BM_EXTERN_ATTRIBUTES {
    BM_EXTERN_ATTRIBUTE_ADD(verbose);
  }


void init() override {  // Attributes
  static constexpr std::uint32_t QUIET = 0u;
  // Init variables
  verbose_ = verbose.get<std::uint32_t>() != QUIET;};

// the packet struct that presents any packet inside the scheduler.
	struct packet {
		unsigned int level3_flow_id;
		unsigned int flow_id;
		unsigned int rank;
		unsigned int pred;
		unsigned int pkt_ptr;
		std::vector<unsigned int> levels_ranks;
		unsigned int arrival_time;
	};
// The flow scheduler struct : which is a queue that sorts 1 head packet from each flow.
	struct flow_scheduler {
		std::shared_ptr<packet> object;
		std::shared_ptr<flow_scheduler> next;
	};
// The Fifo bank struct : which consists of multiple FIFO queues, each one is dedicated to one flow, and stores all packets from this flow except the head packet.
	struct fifo_bank {
		unsigned int flow_id;
		std::shared_ptr<packet> object;
		std::shared_ptr<fifo_bank> bottom;
		std::shared_ptr<fifo_bank> left;
	};

	static unsigned int time_now; // the current time, increment by 1 each time we call the scheduler for a dequeue (which is continous)

// level 3 of the hierarchy variables
	static std::vector<unsigned int> number_of_queues_per_level;
	static std::vector<unsigned int> number_of_pkts_per_queue_each_level;
	static std::vector<unsigned int> error_detected_each_level;
	static std::vector<unsigned int> internal_force_flow_id_each_level;

	static std::vector<std::shared_ptr<flow_scheduler>> FS; 

	static std::vector<std::shared_ptr<fifo_bank>> FB; // the fifo bank queues, each flow scheduler has its own FIFO bank which stores the rest of packets of the flow handled in this flow scheduler.

	static std::vector<unsigned int> new_ranks_each_level;
// level 2 of the hierarchy variables

// level 1 of the hierarchy variables (root)
	 

	std::shared_ptr<packet> deq_packet_ptr = NULL; // the pointer to the dequeued packet
	static unsigned int number_of_enqueue_packets; // the total number of enqueued packets until now.

	static unsigned int number_of_read_packets; // the total number of captured packets by the TM_buffer.h until now.

	static unsigned int number_of_dequeue_packets; // the total number of dequeued packets until now.

	static unsigned int switch_is_ready; 

	static unsigned int number_levels;

// these variables will contain the inputs that will be inserted by the user, to be used later. 
	unsigned int flow_id;
	//std::vector<unsigned int> pkt_levels_ranks = std::vector<unsigned int>(number_levels);
	static std::vector<unsigned int> pkt_levels_ranks;

	std::vector<unsigned int> enq_flow_id_each_level = std::vector<unsigned int>(number_levels);

	unsigned int pred;
	unsigned int arrival_time;
	static unsigned int shaping;   //new static
	unsigned int enq;
	unsigned int pkt_ptr;  //new static
	unsigned int deq;
	static unsigned int use_updated_rank;  //new static
	static unsigned int last_force_deq;  //new static
	unsigned int force_deq;
	static unsigned int force_deq_flow_id;  //new static
	static unsigned int enable_error_correction;   //new static
	static std::vector<unsigned int> flow_id_not_empty_each_level;
	static std::queue<unsigned int> pkt_ptr_queue;

	static int start_time; 
	static int last_time;
	static unsigned int use_queues_rank; 

	void pass_rank_values(const Data& rank_value, const Data& level_id)
	{
		pkt_levels_ranks.erase(pkt_levels_ranks.begin() + level_id.get<uint32_t>());
		pkt_levels_ranks.insert(pkt_levels_ranks.begin() + level_id.get<uint32_t>(), rank_value.get<uint32_t>());
	}
	void pass_updated_rank_values(const Data& rank_value, const Data& flow_id, const Data& level_id)
	{
		new_ranks_each_level.erase(new_ranks_each_level.begin() + flow_id.get<uint32_t>() + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]*level_id.get<uint32_t>()));
		new_ranks_each_level.insert(new_ranks_each_level.begin() + flow_id.get<uint32_t>() + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]*level_id.get<uint32_t>()), rank_value.get<uint32_t>());
	}

	void pass_updated_rank_values_for_queues(const Data& rank_value, const Data& queue_id, const Data& level_id)
	{
		unsigned int offset = 0;
		for(unsigned int i = 0; i < level_id.get<uint32_t>();i++)
		{
			offset = offset + number_of_queues_per_level[i];
		}
		new_ranks_each_level.erase(new_ranks_each_level.begin() + queue_id.get<uint32_t>() + offset + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]*number_levels));
		new_ranks_each_level.insert(new_ranks_each_level.begin() + queue_id.get<uint32_t>() + offset + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]*number_levels), rank_value.get<uint32_t>());
	}

	void my_scheduler(const Data& in_flow_id, const Data& number_of_levels_used, const Data& in_pred, const Data& in_arrival_time, const Data& in_shaping, const Data& in_enq, const Data& in_pkt_ptr, const Data& in_use_queues_rank, const Data& in_use_updated_rank, const Data& in_force_deq, const Data& in_force_deq_flow_id, const Data& in_enable_error_correction, const Data& reset_time)
	{

// copy the inputs values :: Todo : they should be removed later and just use the inputs directly.
	
	if(reset_time.get<uint32_t>() == 1)
	{
	time_now = 0;
	}
		flow_id = in_flow_id.get<uint32_t>();

		// pkt_levels_ranks contains the ranks of this packet at each level, levels_ranks[number_levels] for the root, and levels_ranks[0] for the leaves
		for (int i = number_of_levels_used.get<int>(); i < int(number_levels); i++)
		{
			pkt_levels_ranks.erase(pkt_levels_ranks.begin() + i);
			pkt_levels_ranks.insert(pkt_levels_ranks.begin() + i, pkt_levels_ranks[number_of_levels_used.get<int>()-1]);
		}

		pred = in_pred.get<uint32_t>();
		arrival_time = in_arrival_time.get<uint32_t>();
		shaping = in_shaping.get<uint32_t>();
		enq = in_enq.get<uint32_t>();
		pkt_ptr = in_pkt_ptr.get<uint32_t>();
		pkt_ptr_queue.push(pkt_ptr);
		deq = 0;
		use_updated_rank = in_use_updated_rank.get<uint32_t>();
		last_force_deq = in_force_deq.get<uint32_t>();
		force_deq = 0;
		if(last_force_deq == 1)
		{
			force_deq_flow_id = in_force_deq_flow_id.get<uint32_t>();
		}			
		enable_error_correction = in_enable_error_correction.get<uint32_t>();
		use_queues_rank = in_use_queues_rank.get<uint32_t>(); 
// the core code of the AR-PIFO scheduler, that enqueue, dequeue or force dequeue packets.
		run_core();
	}

// the function for enqueue/dequeue to/from the third level of the hierarchy.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void level_controller(std::shared_ptr<packet>& level_packet_ptr, unsigned int level_enq, unsigned int level_deq, unsigned int level_force_deq, unsigned int level_force_deq_flow_id, std::vector<unsigned int> level_flows_rank, unsigned int level_id)
	{
		unsigned int error_detected;
		unsigned int internal_force_flow_id;
		std::shared_ptr<flow_scheduler> head_FS = NULL;
		std::shared_ptr<packet> out_deq_pkt_ptr;
		std::shared_ptr<fifo_bank> head_FB =  NULL;
		unsigned int queue_id = 0;
		unsigned int next_flow_id_empty = 0;
		unsigned int sum_number_all_queues = 0;
		unsigned int sum_all_update_rank_flows = 0;
		unsigned int offset_flow_id_this_level = 0;

		for(int i = 0; i < int(level_id); i++)
		{
			if(i ==0)
			{
				sum_all_update_rank_flows = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]);
			}
			else
			{
				sum_all_update_rank_flows = sum_all_update_rank_flows + number_of_queues_per_level[i-1];
			}
			sum_number_all_queues = sum_number_all_queues + number_of_queues_per_level[i];

		}
		if (level_enq == 1)
		{	
			if(level_id < (number_levels - 1))
			{
				queue_id = int(level_packet_ptr->flow_id / number_of_pkts_per_queue_each_level[level_id]);
			}
		
			if(level_id == 0)
			{
				head_FB = FB[queue_id];
			}

			error_detected = error_detected_each_level[queue_id + sum_number_all_queues];
			internal_force_flow_id = internal_force_flow_id_each_level[queue_id + sum_number_all_queues];
			head_FS = FS[queue_id + sum_number_all_queues];

			if(level_id !=0)
			{
				sum_all_update_rank_flows = sum_all_update_rank_flows + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]) * (number_levels-1);
				offset_flow_id_this_level = (queue_id *(number_of_pkts_per_queue_each_level[level_id])) + sum_all_update_rank_flows;
				flow_id_not_empty_each_level.erase(flow_id_not_empty_each_level.begin() + level_packet_ptr->flow_id + sum_all_update_rank_flows);
				flow_id_not_empty_each_level.insert(flow_id_not_empty_each_level.begin() + level_packet_ptr->flow_id + sum_all_update_rank_flows, 1);
			}
			else
			{
				flow_id_not_empty_each_level.erase(flow_id_not_empty_each_level.begin() + level_packet_ptr->flow_id);
				flow_id_not_empty_each_level.insert(flow_id_not_empty_each_level.begin() + level_packet_ptr->flow_id, 1);
			}

			DR_PIFO(level_packet_ptr, shaping, level_enq, level_deq, use_updated_rank, \
			level_force_deq, level_force_deq_flow_id, level_flows_rank, head_FS, head_FB, \
			time_now, enable_error_correction, error_detected, internal_force_flow_id, out_deq_pkt_ptr,next_flow_id_empty,(queue_id *(number_of_pkts_per_queue_each_level[level_id])) + sum_all_update_rank_flows, (((queue_id+1) * number_of_pkts_per_queue_each_level[level_id])-1) + sum_all_update_rank_flows, offset_flow_id_this_level, flow_id_not_empty_each_level);

			error_detected_each_level[queue_id + sum_number_all_queues] = error_detected;
			internal_force_flow_id_each_level[queue_id + sum_number_all_queues] = internal_force_flow_id;
			FS[queue_id + sum_number_all_queues] = head_FS;
			if(level_id == 0)
			{
				FB[queue_id] = head_FB;	
			}
		}
		else if (level_deq == 1)
		{
			if(level_id < (number_levels - 1))
			{
				queue_id = level_packet_ptr->flow_id;
			}
				if(level_id == 0)
				{
					head_FB = FB[queue_id];
				}

			if(level_id !=0)
			{
				sum_all_update_rank_flows = sum_all_update_rank_flows + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]) * (number_levels-1);
				//offset_flow_id_this_level = (queue_id *(number_of_pkts_per_queue_each_level[level_id])) + sum_all_update_rank_flows;
				offset_flow_id_this_level = sum_all_update_rank_flows;
			}

			DR_PIFO(level_packet_ptr, shaping, level_enq, level_deq, use_updated_rank, \
			level_force_deq, level_force_deq_flow_id, level_flows_rank, FS[queue_id + sum_number_all_queues], head_FB, \
			time_now, enable_error_correction, error_detected_each_level[queue_id + sum_number_all_queues], internal_force_flow_id_each_level[queue_id + sum_number_all_queues], out_deq_pkt_ptr,next_flow_id_empty,(queue_id *(number_of_pkts_per_queue_each_level[level_id])) + sum_all_update_rank_flows, (((queue_id+1) * number_of_pkts_per_queue_each_level[level_id])-1) + sum_all_update_rank_flows, offset_flow_id_this_level, flow_id_not_empty_each_level);

			level_packet_ptr = out_deq_pkt_ptr;

			if((next_flow_id_empty == 1)&&(level_id == 0))
			{
				flow_id_not_empty_each_level.erase(flow_id_not_empty_each_level.begin() + out_deq_pkt_ptr->flow_id);
				flow_id_not_empty_each_level.insert(flow_id_not_empty_each_level.begin() + out_deq_pkt_ptr->flow_id, 0);
			}
			else if ((level_id !=0)&&(out_deq_pkt_ptr != NULL))
			{
				flow_id_not_empty_each_level.erase(flow_id_not_empty_each_level.begin() + out_deq_pkt_ptr->flow_id + sum_all_update_rank_flows);
				flow_id_not_empty_each_level.insert(flow_id_not_empty_each_level.begin() + out_deq_pkt_ptr->flow_id + sum_all_update_rank_flows, 0);
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// determine the queues IDs for each packet corresponding to the first and the second level of the hierarchy.
	void levels_queues_id(unsigned int flow_id)
	{
		unsigned int flow_id_at_this_level = int(flow_id/number_of_pkts_per_queue_each_level[0]);
		enq_flow_id_each_level.erase(enq_flow_id_each_level.begin() + 0);
		enq_flow_id_each_level.insert(enq_flow_id_each_level.begin() + 0, flow_id_at_this_level);
		for(int i = 1; i < int(number_levels); i++)
		{
			enq_flow_id_each_level.erase(enq_flow_id_each_level.begin() + i);
			enq_flow_id_each_level.insert(enq_flow_id_each_level.begin() + i, flow_id_at_this_level);
			flow_id_at_this_level = int(flow_id/((number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0])/number_of_queues_per_level[i]));
		}
	}

// I used check_levels for dequeue and for "Force dequeue"
	void check_levels(std::vector<unsigned int>& flow_id_found, unsigned int force_deq_operation)
	{
		flow_id_found.erase(flow_id_found.begin() + 0);
		flow_id_found.insert(flow_id_found.begin() + 0, 0);
		std::shared_ptr<flow_scheduler> cur_ptr_FS;
		unsigned int sum_number_all_queues = 0;
		for (int j = 1; j < int(number_levels); j++)
		{
			flow_id_found.erase(flow_id_found.begin() + j);
			flow_id_found.insert(flow_id_found.begin() + j, 0);
			sum_number_all_queues = sum_number_all_queues + number_of_queues_per_level[j-1];
			for (int i = 0; i < int(number_of_queues_per_level[j]);i++)
			{
				cur_ptr_FS = FS[i + sum_number_all_queues];
				while (cur_ptr_FS != NULL)
				{
					if(force_deq_operation == 1)
					{
						if (cur_ptr_FS->object->level3_flow_id == force_deq_flow_id) 
						{
							flow_id_found.erase(flow_id_found.begin() + j);
							flow_id_found.insert(flow_id_found.begin() + j, 1);
							break;
						}
					}
					else
					{
						if (cur_ptr_FS->object->flow_id == enq_flow_id_each_level[j]) 
						{
							flow_id_found.erase(flow_id_found.begin() + j);
							flow_id_found.insert(flow_id_found.begin() + j, 1);
							break;
						}
					}
					cur_ptr_FS = cur_ptr_FS->next;
				}
				if(flow_id_found[j] == 1)
				{
				break;
				}
			}
		}
	}

	void update_ranks_level_all_levels()
	{
		unsigned int sum_number_all_queues = 0;
		unsigned int sum_all_update_rank_flows = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0])*(number_levels);
		unsigned int offset_of_new_ranks_each_level = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0]);

		for(int i = 0; i < int(number_of_queues_per_level[0]); i++)
		{	
			if (FS[i] != NULL)
			{
				new_ranks_each_level.erase(new_ranks_each_level.begin() + i + sum_all_update_rank_flows);

				if ((use_updated_rank == 1) && (enable_error_correction == 1)&&(error_detected_each_level[i]==1))
				{
					new_ranks_each_level.insert(new_ranks_each_level.begin() + i + sum_all_update_rank_flows, new_ranks_each_level[internal_force_flow_id_each_level[i] + offset_of_new_ranks_each_level]);
				
				}
				else if ((use_updated_rank == 1) && (new_ranks_each_level[FS[i]->object->flow_id + offset_of_new_ranks_each_level]!=0))
				{
					new_ranks_each_level.insert(new_ranks_each_level.begin() + i + sum_all_update_rank_flows, new_ranks_each_level[FS[i]->object->flow_id + offset_of_new_ranks_each_level]);
				}			
				else
				{
					new_ranks_each_level.insert(new_ranks_each_level.begin() + i + sum_all_update_rank_flows, FS[i]->object->rank);
				}
			}
		}

		for(int j = 2; j < int(number_levels); j++)
		{
			sum_number_all_queues = sum_number_all_queues + number_of_queues_per_level[j-2];
			sum_all_update_rank_flows = sum_all_update_rank_flows + number_of_queues_per_level[j-2];  
			unsigned int offset_of_new_ranks_each_level = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0])*(j-1);
			for(int i = 0; i < int(number_of_queues_per_level[j-1]); i++)
			{	
				if (FS[i + sum_number_all_queues] != NULL)
				{
					new_ranks_each_level.erase(new_ranks_each_level.begin() + i + sum_all_update_rank_flows);

					if ((use_updated_rank == 1) && (new_ranks_each_level[FS[i + sum_number_all_queues]->object->level3_flow_id + offset_of_new_ranks_each_level]!=0))
					{
						new_ranks_each_level.insert(new_ranks_each_level.begin() + i + sum_all_update_rank_flows, new_ranks_each_level[FS[i + sum_number_all_queues]->object->level3_flow_id + offset_of_new_ranks_each_level]);					
					}				
					else
					{
						new_ranks_each_level.insert(new_ranks_each_level.begin() + i + sum_all_update_rank_flows, FS[i + sum_number_all_queues]->object->rank);
					}
				}
			}
		}
	}

// the core function of the DR_PIFO scheduler, applies enqueue and dequeue operations, to & from each level of the hirarchy, 
//and each level is responsible of enqueue and dequeue in each queue inside this level 
	void run_core()
	{
		deq_packet_ptr = NULL;

		if (enq == 1)
		{
			if((start_time == 0)||(time_now ==0))
			{
				//start_time = std::time(0);
				start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			}

			number_of_enqueue_packets = number_of_enqueue_packets + 1;
			std::shared_ptr<packet> enq_packet_ptr;
			enq_packet_ptr = std::make_shared<packet>();
			enq_packet_ptr->level3_flow_id = flow_id;
			enq_packet_ptr->flow_id = flow_id;
			enq_packet_ptr->rank = pkt_levels_ranks[0];
			enq_packet_ptr->pred = pred;
			enq_packet_ptr->pkt_ptr = pkt_ptr;
			enq_packet_ptr->levels_ranks = pkt_levels_ranks;
			enq_packet_ptr->arrival_time = arrival_time;


			levels_queues_id(flow_id);
			std::vector<unsigned int> flow_id_found_all_levels = std::vector<unsigned int>(number_levels);
			check_levels(flow_id_found_all_levels, 0);
			// levels_ranks should be the updated flows ranks
			
			unsigned int enq_level_id = 0;
			for(int i = number_levels - 1; i > 0; i--)
			{
				if (flow_id_found_all_levels[i] == 0)
				{
					enq_packet_ptr->rank = pkt_levels_ranks[i];
					enq_packet_ptr->flow_id = enq_flow_id_each_level[i];
					enq_level_id = i;
					break;
				}
			}

			level_controller(enq_packet_ptr, enq, 0, 0, 0, new_ranks_each_level, enq_level_id);

		}

		if ((deq == 1)&&(switch_is_ready == 1))
		{
			if((number_levels > 1)&&(enable_error_correction != 0)&&(use_queues_rank==0))
			{
				update_ranks_level_all_levels();
			}

			//last_time = std::time(0);
			last_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

			if(last_time > start_time)
			{
				time_now = last_time - start_time;
			}
			else
			{
				time_now = 0;
			}

			//std::this_thread::sleep_for(std::chrono::microseconds(810));  // equivalent to 1ms (with adding the overhead of the code)
			std::this_thread::sleep_for(std::chrono::microseconds(10));  // equivalent to 1ms (with adding the overhead of the code)

			deq_packet_ptr = std::shared_ptr<packet>(std::make_shared<packet>());

			level_controller(deq_packet_ptr, 0, deq, 0, 0, new_ranks_each_level, number_levels - 1);
			std::shared_ptr<packet> intermediate_pkt_ptr = std::shared_ptr<packet>(std::make_shared<packet>());

			if(deq_packet_ptr != NULL)
			{
			unsigned int prev_level_dequeued_flow_id = deq_packet_ptr->flow_id;
			unsigned int number_of_update_ranks_all_level = number_of_queues_per_level[1] + number_of_queues_per_level[0]  + (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0] * number_levels);
				for(int i = number_levels - 2; i >= 0; i--)
				{
					intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
					level_controller(intermediate_pkt_ptr, 0, deq, 0, 0, new_ranks_each_level, i);
					if ((intermediate_pkt_ptr == NULL)&&(shaping))
					{
						unsigned int original_shaping_value = shaping;
						shaping = 0;
						intermediate_pkt_ptr = std::shared_ptr<packet>(std::make_shared<packet>());
						intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
						level_controller(intermediate_pkt_ptr, 0, deq, 0, 0, new_ranks_each_level, i);
						shaping = original_shaping_value;
					}
					if(intermediate_pkt_ptr != NULL)
					{
						unsigned int offset_of_new_ranks_each_level;
						unsigned int id_for_new_rank;
						if(use_queues_rank == 1)
						{
							number_of_update_ranks_all_level = number_of_update_ranks_all_level - number_of_queues_per_level[i];
							offset_of_new_ranks_each_level = number_of_update_ranks_all_level;
							id_for_new_rank = prev_level_dequeued_flow_id;
						}
						else
						{
							offset_of_new_ranks_each_level = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0])*(i+1);
							id_for_new_rank = intermediate_pkt_ptr->level3_flow_id;
						}
						unsigned int last_dequeued_pkt_flow_id = intermediate_pkt_ptr->flow_id;
						intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
						prev_level_dequeued_flow_id = last_dequeued_pkt_flow_id;

						if ((use_updated_rank == 1) && (new_ranks_each_level[ id_for_new_rank + offset_of_new_ranks_each_level]!=0))
						{
							intermediate_pkt_ptr->rank = new_ranks_each_level[id_for_new_rank + offset_of_new_ranks_each_level];
						}
						else
						{
							intermediate_pkt_ptr->rank = intermediate_pkt_ptr->levels_ranks[i+1];
						}
						level_controller(intermediate_pkt_ptr, 1, 0, 0, 0, new_ranks_each_level, i+1);
					}
					else
					{
						break;
					}
				}
			}

		}
		else if ((force_deq)&&(switch_is_ready == 1))
		{

			if(last_time > start_time)
			{
				time_now = last_time - start_time;
			}
			else
			{
				time_now = 0;
			}
			
			if((number_levels > 1)&&(enable_error_correction != 0)&&(use_queues_rank==0))
			{
				update_ranks_level_all_levels();
			}
			//time_now = time_now + 1;
			//std::this_thread::sleep_for(std::chrono::microseconds(810));  // equivalent to 1ms (with adding the overhead of the code)
			std::this_thread::sleep_for(std::chrono::microseconds(10));  // equivalent to 1ms (with adding the overhead of the code) speed.
			//std::this_thread::sleep_for(std::chrono::nanoseconds(1)); 

			deq_packet_ptr = std::shared_ptr<packet>(std::make_shared<packet>());
			levels_queues_id(force_deq_flow_id);

			std::vector<unsigned int> force_deq_flow_id_found_all_levels = std::vector<unsigned int>(number_levels);
			check_levels(force_deq_flow_id_found_all_levels, 1);

			unsigned int level_id_force_deq = 0;
			unsigned int force_deq_flow_id_this_level = force_deq_flow_id;
			for(int i = number_levels - 1; i > 0; i--)
			{
				if(force_deq_flow_id_found_all_levels[i] == 1)
				{
					level_id_force_deq = i;
					force_deq_flow_id_this_level = enq_flow_id_each_level[i];
					break;
				}
			}

			deq_packet_ptr->flow_id = enq_flow_id_each_level[level_id_force_deq];		

			level_controller(deq_packet_ptr, 0, 1, force_deq, force_deq_flow_id_this_level, new_ranks_each_level, level_id_force_deq);
			std::shared_ptr<packet> intermediate_pkt_ptr = std::shared_ptr<packet>(std::make_shared<packet>());

			if(deq_packet_ptr != NULL)
			{
				unsigned int prev_level_dequeued_flow_id = deq_packet_ptr->flow_id;
				for(int i = level_id_force_deq - 1; i >= 0; i--)
				{
					intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
					level_controller(intermediate_pkt_ptr, 0, 1, 0, 0, new_ranks_each_level, i);
					if((intermediate_pkt_ptr == NULL)&&(shaping ==1))
					{
						unsigned int original_shaping_value = shaping;
						shaping = 0;
						intermediate_pkt_ptr = std::shared_ptr<packet>(std::make_shared<packet>());
						intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
						level_controller(intermediate_pkt_ptr, 0, 1, 0, 0, new_ranks_each_level, i);
						shaping = original_shaping_value;
					}
					if(intermediate_pkt_ptr != NULL)
					{
						unsigned int last_dequeued_pkt_flow_id = intermediate_pkt_ptr->flow_id;
						intermediate_pkt_ptr->flow_id = prev_level_dequeued_flow_id;
						prev_level_dequeued_flow_id = last_dequeued_pkt_flow_id;
						unsigned int offset_of_new_ranks_each_level = (number_of_pkts_per_queue_each_level[0]*number_of_queues_per_level[0])*(i);
						if ((use_updated_rank == 1) && (new_ranks_each_level[intermediate_pkt_ptr->level3_flow_id + offset_of_new_ranks_each_level]!=0))
						{
							intermediate_pkt_ptr->rank = new_ranks_each_level[intermediate_pkt_ptr->level3_flow_id + offset_of_new_ranks_each_level];
						}
						else
						{
							intermediate_pkt_ptr->rank = intermediate_pkt_ptr->levels_ranks[i+1];
						}
						level_controller(intermediate_pkt_ptr, 1, 0, 0, 0, new_ranks_each_level, i+1);
					}
					else
					{
						break;
					}
				}
			}
		}		
	}


// This is the AR-PIFO queue function which handles 1 flow_scheduler and 1 FIFO at a time, this function is used by each level function
	void DR_PIFO(std::shared_ptr<DR_PIFO_scheduler::packet> pkt_ptr, unsigned int in_shaping, unsigned int in_enq, unsigned int in_deq, unsigned int in_use_updated_rank, \
		unsigned int in_force_deq, unsigned int in_force_deq_flow_id, std::vector<unsigned int> in_flows_rank, std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>& in_head_FS, std::shared_ptr<DR_PIFO_scheduler::fifo_bank>& in_head_FB, \
		unsigned int in_time_now, unsigned int in_enable_error_correction, unsigned int& in_error_detected, unsigned int& in_internal_force_flow_id, std::shared_ptr<packet>& out_deq_pkt_ptr, unsigned int& next_flow_id_empty, unsigned int start_pt_flows_ranks, unsigned int end_pt_flows_ranks, unsigned int offset_flow_id_this_level, std::vector<unsigned int> flow_id_not_empty_each_level)
	{
		std::shared_ptr<DR_PIFO_scheduler::packet> deq_packet_ptr = NULL;
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> cur_ptr_FS;
		unsigned int flow_id_FS_found;
		next_flow_id_empty = 0;

// in case of enqueue (enq ==1), a packet will be enqueued to flow scheduler first enqueue_FS (if its flow already existed there), it will be enqueued in the FIFO bank instead.
// in case of dequeue (deq ==1), a packet will be dequeued from the flow scheduler dequeue_FS, then the next packet from the same flow will be dequeued from the FIFO bank dequeue_FB,
// then enqueue this next packet to the flow scheduler enqueue_FS.
		if (in_enq == 1)
		{
			std::shared_ptr<DR_PIFO_scheduler::packet> enq_packet_ptr;
			enq_packet_ptr = std::shared_ptr<DR_PIFO_scheduler::packet>(std::make_shared<DR_PIFO_scheduler::packet>(*pkt_ptr));

			cur_ptr_FS = in_head_FS;
			flow_id_FS_found = 0;
			while (cur_ptr_FS != NULL)
			{
				if (cur_ptr_FS->object->flow_id == pkt_ptr->flow_id) {
					flow_id_FS_found = 1;
				}
				cur_ptr_FS = cur_ptr_FS->next;
			}

			if (flow_id_FS_found == 1)
			{
				enqueue_FB(enq_packet_ptr, in_head_FB);
			}
			else
			{
				enqueue_FS(enq_packet_ptr, in_head_FS);
			}
		}
		if (in_deq == 1)
		{
			if (in_force_deq == 1)
			{
				force_dequeue_FS(deq_packet_ptr, in_force_deq_flow_id, in_head_FS, in_time_now, in_shaping);
			}
			else if (in_error_detected == 1)
			{
				force_dequeue_FS(deq_packet_ptr, in_internal_force_flow_id, in_head_FS, in_time_now, in_shaping);
				in_error_detected = 0;
			}
			else
			{
				dequeue_FS(deq_packet_ptr, in_shaping, in_head_FS, in_time_now);

				if ((in_enable_error_correction == 1) && (deq_packet_ptr != NULL))
				{
					unsigned int iter_min_rank;
					unsigned int iter_flow_id;
					if((in_use_updated_rank == 1) && (flow_id_not_empty_each_level[(deq_packet_ptr->flow_id)+offset_flow_id_this_level] != 0) && (in_flows_rank[(deq_packet_ptr->flow_id)+offset_flow_id_this_level] != 0))
					{iter_min_rank = in_flows_rank[(deq_packet_ptr->flow_id)+offset_flow_id_this_level];}
					else
					{iter_min_rank = deq_packet_ptr->rank;}
					
					iter_flow_id = deq_packet_ptr->flow_id;

					for (unsigned int i = start_pt_flows_ranks; i < end_pt_flows_ranks + 1; i++)
					{
				// arrival time is not considered here !!, should we keep an arrival time array for each flow ?!
						if ((((iter_min_rank > in_flows_rank[i]) && ((i-offset_flow_id_this_level) != deq_packet_ptr->flow_id))||((iter_min_rank == in_flows_rank[i]) && ((i-offset_flow_id_this_level) < iter_flow_id))) && (flow_id_not_empty_each_level[i] != 0) && (in_flows_rank[i] != 0))
						{
							in_error_detected = 1;
							in_internal_force_flow_id = (i-offset_flow_id_this_level);
							iter_min_rank = in_flows_rank[i];
							iter_flow_id = (i-start_pt_flows_ranks);
						}
					}
				}
			}

			if (deq_packet_ptr != NULL)
			{
				std::shared_ptr<DR_PIFO_scheduler::packet> FB_FS_enq_packet_ptr;
				FB_FS_enq_packet_ptr = std::shared_ptr<DR_PIFO_scheduler::packet>(std::make_shared<DR_PIFO_scheduler::packet>());
				dequeue_FB(FB_FS_enq_packet_ptr, deq_packet_ptr->flow_id, in_head_FB);
				if (FB_FS_enq_packet_ptr != NULL)
				{
					if ((in_use_updated_rank == 1)&&(flow_id_not_empty_each_level[(FB_FS_enq_packet_ptr->level3_flow_id)] != 0)&&(in_flows_rank[(FB_FS_enq_packet_ptr->level3_flow_id)] != 0))
					{
						FB_FS_enq_packet_ptr->rank = in_flows_rank[(FB_FS_enq_packet_ptr->level3_flow_id)];
					}
					enqueue_FS(FB_FS_enq_packet_ptr, in_head_FS);
				}
				else
				{
				next_flow_id_empty = 1;
					if((in_enable_error_correction == 1) && (in_error_detected != 1))
					{
						unsigned int this_flow_rank;
						unsigned int rank_to_compare;
						unsigned int iter_flow_id;

						rank_to_compare = deq_packet_ptr->rank;

						this_flow_rank = rank_to_compare;
						iter_flow_id = number_of_pkts_per_queue_each_level[0];
						for (unsigned int i = start_pt_flows_ranks; i < end_pt_flows_ranks + 1; i++)
						{
						if ((((this_flow_rank == in_flows_rank[i]) && ((i-offset_flow_id_this_level) < iter_flow_id))) && (flow_id_not_empty_each_level[i] != 0)  && (in_flows_rank[i] != 0) && ((i-offset_flow_id_this_level) != deq_packet_ptr->flow_id))
							{
								in_error_detected = 1;
								in_internal_force_flow_id = (i-offset_flow_id_this_level);
								this_flow_rank = in_flows_rank[i];
								iter_flow_id = (i-start_pt_flows_ranks);	
							}
						}
					}				
				}
			}

		}

		if (in_deq == 1)
			out_deq_pkt_ptr = deq_packet_ptr;
		else
			out_deq_pkt_ptr = NULL;
	}

// used by AR-PIFO function to enqueue inside a certain flow scheduler
	void enqueue_FS(std::shared_ptr<DR_PIFO_scheduler::packet> new_packet_ptr, std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>& head_FS)
	{
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> cur_ptr_FS;
		cur_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> prev_ptr_FS;
		prev_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		if (head_FS == NULL)
		{
			head_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
			head_FS->object = new_packet_ptr;
			head_FS->next = NULL;
			cur_ptr_FS = head_FS;
		}
		else
		{
			cur_ptr_FS = head_FS;
			prev_ptr_FS = NULL;
			std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> temp_ptr;
			temp_ptr = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
			temp_ptr->object = new_packet_ptr;
			while ((cur_ptr_FS != NULL))
			{
				if((cur_ptr_FS->object->rank < new_packet_ptr->rank)||((cur_ptr_FS->object->rank == new_packet_ptr->rank)&&(cur_ptr_FS->object->arrival_time < new_packet_ptr->arrival_time)))
				{
				prev_ptr_FS = cur_ptr_FS;
				cur_ptr_FS = cur_ptr_FS->next;
				}
				else
				{
				break;
				}
			}
			temp_ptr->next = cur_ptr_FS;
			if (prev_ptr_FS == NULL)
			{
				head_FS = temp_ptr;
			}
			else
			{
				prev_ptr_FS->next = temp_ptr;
			}
		}
	}

// used by AR-PIFO function to enqueue inside a certain FIFO bank
	void enqueue_FB(std::shared_ptr<DR_PIFO_scheduler::packet> new_packet_ptr, std::shared_ptr<DR_PIFO_scheduler::fifo_bank>& head_FB)
	{
		std::shared_ptr<DR_PIFO_scheduler::fifo_bank> cur_ptr_FB;
		cur_ptr_FB = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
		std::shared_ptr<DR_PIFO_scheduler::fifo_bank> prev_ptr_FB;
		prev_ptr_FB = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
		std::shared_ptr<DR_PIFO_scheduler::fifo_bank> temp_ptr;
		temp_ptr = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
		temp_ptr->object = new_packet_ptr;
		temp_ptr->left = NULL;
		temp_ptr->bottom = NULL;
		if (head_FB == NULL)
		{
			head_FB = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
			head_FB->flow_id = new_packet_ptr->flow_id;
			head_FB->bottom = NULL;
			head_FB->left = temp_ptr;
			cur_ptr_FB = head_FB;
		}
		else
		{
			cur_ptr_FB = head_FB;
			prev_ptr_FB = NULL;
			unsigned int flow_id_FB_found = 0;
			while (cur_ptr_FB != NULL)
			{
				if (cur_ptr_FB->flow_id == new_packet_ptr->flow_id)
				{
					flow_id_FB_found = 1;
					while (cur_ptr_FB->left != NULL)
					{
						cur_ptr_FB = cur_ptr_FB->left;
					}
					cur_ptr_FB->left = temp_ptr;
					break;
				}
				prev_ptr_FB = cur_ptr_FB;
				cur_ptr_FB = cur_ptr_FB->bottom;
			}
			if (flow_id_FB_found == 0)
			{
				std::shared_ptr<DR_PIFO_scheduler::fifo_bank> temp_ptr2;
				temp_ptr2 = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
				temp_ptr2->flow_id = new_packet_ptr->flow_id;
				temp_ptr2->left = temp_ptr;
				temp_ptr2->bottom = NULL;
				prev_ptr_FB->bottom = temp_ptr2;
			}
		}

	}

// used by AR-PIFO function to dequeue from a certain flow scheduler
	void dequeue_FS(std::shared_ptr<DR_PIFO_scheduler::packet>& deq_packet_ptr, unsigned int in_shaping, std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>& head_FS, unsigned int time_now)
	{
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> cur_ptr_FS;
		cur_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> prev_ptr_FS;
		prev_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		deq_packet_ptr = NULL;

		if ((in_shaping == 1) && (head_FS != NULL))
		{
			cur_ptr_FS = head_FS;
			prev_ptr_FS = NULL;

			if ((cur_ptr_FS->object->pred <= time_now)&& (cur_ptr_FS->object->pkt_ptr <= number_of_read_packets)) // Elbediwy : I add this, to synchronize with the switch multithreading
			{
				deq_packet_ptr = head_FS->object;
				head_FS = head_FS->next;
			}
			else
			{
				while (cur_ptr_FS != NULL)
				{
					if ((cur_ptr_FS->object->pred <= time_now) && (cur_ptr_FS->object->pkt_ptr <= number_of_read_packets)) // Elbediwy : I add this, to synchronize with the switch multithreading
					{
						deq_packet_ptr = cur_ptr_FS->object;
						prev_ptr_FS->next = cur_ptr_FS->next;
						break;
					}
					prev_ptr_FS = cur_ptr_FS;
					cur_ptr_FS = cur_ptr_FS->next;
				}
			}
		}
		else if (head_FS != NULL)
		{
			if ((head_FS->object->pkt_ptr <= number_of_read_packets)) // Elbediwy : I add this, to synchronize with the switch multithreading
			{
				deq_packet_ptr = head_FS->object;
				head_FS = head_FS->next;
			}
		}
	}

// used by AR-PIFO function to force dequeue from a certain Flow ID from a certain flow scheduler
	void force_dequeue_FS(std::shared_ptr<DR_PIFO_scheduler::packet>& deq_packet_ptr, unsigned int flow_id, std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>& head_FS, unsigned int time_now, unsigned int in_shaping)
	{
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> cur_ptr_FS;
		cur_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		std::shared_ptr<DR_PIFO_scheduler::flow_scheduler> prev_ptr_FS;
		prev_ptr_FS = std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>(std::make_shared<DR_PIFO_scheduler::flow_scheduler>());
		deq_packet_ptr = NULL;
		cur_ptr_FS = head_FS;
		prev_ptr_FS = NULL;
		if(cur_ptr_FS != NULL)
		{
			if (((in_shaping == 1)&&(cur_ptr_FS->object->flow_id == flow_id)&&(cur_ptr_FS->object->pred <= time_now))||((in_shaping == 0)&&(cur_ptr_FS->object->flow_id == flow_id)))
			{
				deq_packet_ptr = head_FS->object;
				head_FS = head_FS->next;
			}
			else if ((cur_ptr_FS->object->flow_id != flow_id))
			{
				while (cur_ptr_FS != NULL)
				{
					if ((cur_ptr_FS->object->flow_id == flow_id) && (cur_ptr_FS->object->pred <= time_now) && (shaping == 1))
					{
						deq_packet_ptr = cur_ptr_FS->object;
						prev_ptr_FS->next = cur_ptr_FS->next;
						break;
					}
					else if ((cur_ptr_FS->object->flow_id == flow_id) && (shaping ==0))
					{
						deq_packet_ptr = cur_ptr_FS->object;
						prev_ptr_FS->next = cur_ptr_FS->next;
						break;
					}
					prev_ptr_FS = cur_ptr_FS;
					cur_ptr_FS = cur_ptr_FS->next;
				}
			}
		}
	}

// used by AR-PIFO function to dequeue from a certain FIFO bank
	void dequeue_FB(std::shared_ptr<DR_PIFO_scheduler::packet>& deq_packet_ptr, unsigned int flow_id, std::shared_ptr<DR_PIFO_scheduler::fifo_bank>& head_FB)
	{
		std::shared_ptr<DR_PIFO_scheduler::fifo_bank> cur_ptr_FB;
		cur_ptr_FB = std::shared_ptr<DR_PIFO_scheduler::fifo_bank>(std::make_shared<DR_PIFO_scheduler::fifo_bank>());
		deq_packet_ptr = NULL;
		cur_ptr_FB = head_FB;
		while (cur_ptr_FB != NULL)
		{
			if ((cur_ptr_FB->flow_id == flow_id) && (cur_ptr_FB->left != NULL))
			{
				deq_packet_ptr = cur_ptr_FB->left->object;
				if (deq_packet_ptr != NULL) {
					cur_ptr_FB->left = cur_ptr_FB->left->left;
				}

				break;
			}
			cur_ptr_FB = cur_ptr_FB->bottom;
		}
	}

// return the last enqueued packet pointer to be used in the buffer inside the "Simple_switch" target
	unsigned int get_last_pkt_ptr()
	{
		if(!pkt_ptr_queue.empty())
		{
			unsigned int current_pkt_ptr = pkt_ptr_queue.front();
			pkt_ptr_queue.pop();
			number_of_read_packets = number_of_read_packets + 1;
			return current_pkt_ptr;			
		}
		else
		{
			return 0;
		}
	}

// Apply dequeue operation in the scheduler, will be used inside the "Simple_Switch" target
	unsigned int dequeue_my_scheduler()
	{
	flow_id = 0;
	pred = 0;
	enq = 0;
	unsigned int null_ptr = 0;
	force_deq = last_force_deq;
	if(force_deq ==0)
	deq = 1;
	else
	deq = 0;

	run_core();

	if(deq_packet_ptr != NULL)
	{
		switch_is_ready = 0;

		if(last_force_deq !=0)
		last_force_deq = 0;
		if(force_deq_flow_id !=0)
		force_deq_flow_id = 0;
		//if(pkt_ptr !=0)
		//pkt_ptr = 0;
		return deq_packet_ptr->pkt_ptr;
	}	
	else
	{
	return null_ptr;
	}
	}

// return the number of enqueued packets until now to the "Simple_Switch".
	unsigned int number_of_enq_pkts()
	{
		return number_of_enqueue_packets;
	}


	unsigned int number_of_deq_pkts()
	{
		return number_of_dequeue_packets;
	}

// return the number of captured packets until now by the TM_buffer.h.
	unsigned int num_of_read_pkts()
	{
		return number_of_read_packets;
	}

// A flag from the target switch, indicating that the switch is ready to receive a new dequeued pkt from the TM
	void start_dequeue(unsigned int start)
	{
		switch_is_ready = start;
	}

// reset the number of enqueued packets to zero by the "Simple_Switch"
	void reset_number_of_enq_pkts()
	{
		number_of_enqueue_packets = 0;
	}

	void increment_deq_count()
	{
		number_of_dequeue_packets = number_of_dequeue_packets + 1;
	}

 private:
  // Attribute
  Data verbose{};

  // Data members
  bool verbose_{false};
};

}  // namespace bm
#endif
