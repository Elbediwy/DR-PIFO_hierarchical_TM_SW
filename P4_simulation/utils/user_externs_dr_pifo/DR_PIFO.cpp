#include "DR_PIFO.h"
#include <bm/bm_sim/logger.h>
#include "floor.h"

namespace bm {


//std::vector<std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>> DR_PIFO_scheduler::FS = { NULL}; // one level
std::vector<std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>> DR_PIFO_scheduler::FS = { NULL, NULL, NULL}; // 2 levels
//std::vector<std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>> DR_PIFO_scheduler::FS = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // 3 levels
//std::vector<std::shared_ptr<DR_PIFO_scheduler::flow_scheduler>> DR_PIFO_scheduler::FS = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL }; // 5 levels


//std::vector<std::shared_ptr<DR_PIFO_scheduler::fifo_bank>> DR_PIFO_scheduler::FB = { NULL}; // one level
std::vector<std::shared_ptr<DR_PIFO_scheduler::fifo_bank>> DR_PIFO_scheduler::FB = { NULL, NULL}; // 2 levels
//std::vector<std::shared_ptr<DR_PIFO_scheduler::fifo_bank>> DR_PIFO_scheduler::FB = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // 3 levels
//std::vector<std::shared_ptr<DR_PIFO_scheduler::fifo_bank>> DR_PIFO_scheduler::FB = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // 3 levels

unsigned int DR_PIFO_scheduler::time_now = 0;
//unsigned int DR_PIFO_scheduler::number_levels = 1; // one level
unsigned int DR_PIFO_scheduler::number_levels = 2; // 2 levels
//unsigned int DR_PIFO_scheduler::number_levels = 3; // 3 levels
//unsigned int DR_PIFO_scheduler::number_levels = 5; // 5 levels


//std::vector<unsigned int> DR_PIFO_scheduler::number_of_queues_per_level = {1}; // one level
std::vector<unsigned int> DR_PIFO_scheduler::number_of_queues_per_level = {2,1}; // 2 levels
//std::vector<unsigned int> DR_PIFO_scheduler::number_of_queues_per_level = {8,2,1}; // 3 levels
//std::vector<unsigned int> DR_PIFO_scheduler::number_of_queues_per_level = {16,8,4,2,1}; // 5 levels

//std::vector<unsigned int> DR_PIFO_scheduler::number_of_pkts_per_queue_each_level = {80}; // one level
std::vector<unsigned int> DR_PIFO_scheduler::number_of_pkts_per_queue_each_level = {50 ,DR_PIFO_scheduler::number_of_queues_per_level[1]}; // 2 levels
//std::vector<unsigned int> DR_PIFO_scheduler::number_of_pkts_per_queue_each_level = {10 ,DR_PIFO_scheduler::number_of_queues_per_level[0]/DR_PIFO_scheduler::number_of_queues_per_level[1] ,DR_PIFO_scheduler::number_of_queues_per_level[1]}; // 3 levels
//std::vector<unsigned int> DR_PIFO_scheduler::number_of_pkts_per_queue_each_level = {80 ,DR_PIFO_scheduler::number_of_queues_per_level[0]/DR_PIFO_scheduler::number_of_queues_per_level[1],DR_PIFO_scheduler::number_of_queues_per_level[1]/DR_PIFO_scheduler::number_of_queues_per_level[2],DR_PIFO_scheduler::number_of_queues_per_level[2]/DR_PIFO_scheduler::number_of_queues_per_level[3] ,DR_PIFO_scheduler::number_of_queues_per_level[3]}; // 5 levels


//unsigned int sum_all_queues = DR_PIFO_scheduler::number_of_queues_per_level[0]; // one level
unsigned int sum_all_queues = DR_PIFO_scheduler::number_of_queues_per_level[0] + DR_PIFO_scheduler::number_of_queues_per_level[1]; // 2 levels
//unsigned int sum_all_queues = DR_PIFO_scheduler::number_of_queues_per_level[0] + DR_PIFO_scheduler::number_of_queues_per_level[1] + DR_PIFO_scheduler::number_of_queues_per_level[2]; // 3 levels
//unsigned int sum_all_queues = DR_PIFO_scheduler::number_of_queues_per_level[0] + DR_PIFO_scheduler::number_of_queues_per_level[1] + DR_PIFO_scheduler::number_of_queues_per_level[2] + DR_PIFO_scheduler::number_of_queues_per_level[3] + DR_PIFO_scheduler::number_of_queues_per_level[4]; // 5 levels

std::vector<unsigned int> DR_PIFO_scheduler::error_detected_each_level(sum_all_queues);
std::vector<unsigned int> DR_PIFO_scheduler::internal_force_flow_id_each_level(sum_all_queues);

//unsigned int number_of_update_ranks_all_level = (DR_PIFO_scheduler::number_of_pkts_per_queue_each_level[0]*DR_PIFO_scheduler::number_of_queues_per_level[0] * DR_PIFO_scheduler::number_levels); // one level
unsigned int number_of_update_ranks_all_level = DR_PIFO_scheduler::number_of_queues_per_level[0]  + (DR_PIFO_scheduler::number_of_pkts_per_queue_each_level[0]*DR_PIFO_scheduler::number_of_queues_per_level[0] * DR_PIFO_scheduler::number_levels); // 2 levels
//unsigned int number_of_update_ranks_all_level = DR_PIFO_scheduler::number_of_queues_per_level[1] + DR_PIFO_scheduler::number_of_queues_per_level[0]  + (DR_PIFO_scheduler::number_of_pkts_per_queue_each_level[0]*DR_PIFO_scheduler::number_of_queues_per_level[0] * DR_PIFO_scheduler::number_levels); // 3 levels
//unsigned int number_of_update_ranks_all_level = DR_PIFO_scheduler::number_of_queues_per_level[3] + DR_PIFO_scheduler::number_of_queues_per_level[2] + DR_PIFO_scheduler::number_of_queues_per_level[1] + DR_PIFO_scheduler::number_of_queues_per_level[0]  + (DR_PIFO_scheduler::number_of_pkts_per_queue_each_level[0]*DR_PIFO_scheduler::number_of_queues_per_level[0] * DR_PIFO_scheduler::number_levels); // 5 levels

std::vector<unsigned int> DR_PIFO_scheduler::new_ranks_each_level(number_of_update_ranks_all_level);

std::vector<unsigned int> DR_PIFO_scheduler::flow_id_not_empty_each_level(number_of_update_ranks_all_level);

std::queue<unsigned int> DR_PIFO_scheduler::pkt_ptr_queue;

unsigned int DR_PIFO_scheduler::use_updated_rank = 0;
unsigned int DR_PIFO_scheduler::last_force_deq = 0;
unsigned int DR_PIFO_scheduler::force_deq_flow_id = 0;
unsigned int DR_PIFO_scheduler::shaping = 0;
unsigned int DR_PIFO_scheduler::enable_error_correction = 0;
unsigned int DR_PIFO_scheduler::number_of_enqueue_packets = 0;
//std::vector<unsigned int> DR_PIFO_scheduler::pkt_levels_ranks = {0}; // 1 level
std::vector<unsigned int> DR_PIFO_scheduler::pkt_levels_ranks = {0,0}; // 2 levels
//std::vector<unsigned int> DR_PIFO_scheduler::pkt_levels_ranks = {0,0,0}; // 3 levels
//std::vector<unsigned int> DR_PIFO_scheduler::pkt_levels_ranks = {0,0,0,0,0}; // 5 levels

unsigned int DR_PIFO_scheduler::number_of_read_packets = 0;
unsigned int DR_PIFO_scheduler::number_of_dequeue_packets = 0;
unsigned int DR_PIFO_scheduler::switch_is_ready = 1;

int DR_PIFO_scheduler::start_time = 0;
int DR_PIFO_scheduler::last_time = 0;

BM_REGISTER_EXTERN(DR_PIFO_scheduler)
BM_REGISTER_EXTERN_METHOD(DR_PIFO_scheduler, my_scheduler, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&, const Data&);

BM_REGISTER_EXTERN_METHOD(DR_PIFO_scheduler, pass_rank_values, const Data&, const Data&);

BM_REGISTER_EXTERN_METHOD(DR_PIFO_scheduler, pass_updated_rank_values, const Data&, const Data&, const Data&);


BM_REGISTER_EXTERN(floor_extern)
BM_REGISTER_EXTERN_METHOD(floor_extern, floor, const Data&, const Data&, Data&);

}  // namespace bm

int import_DR_PIFO(){
  return 0;
}
