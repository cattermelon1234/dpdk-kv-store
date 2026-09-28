#pragma once
#include <cstdint>
#include "kv/linear_hash_map.hpp"

class Worker {
public:
  Worker(std::uint16_t port_id, std::uint16_t rx_queue_id,
         std::uint16_t tx_queue_id) {}
  void run() {}

  void cleanup() {}

private:
  std::uint16_t port_id;
  std::uint16_t rx_queue_id;
  std::uint16_t tx_queue_id;
  rte_mempool* packet_pool;
  LinearHashMap<std::uint32_t, std::uint32_t> map_;
  std::span<std::span<rte_ring* const> inboxes;
};

// function ptr to pass into dpdk rte_eal_remote_launch() api
int worker_entry(void *arg);

