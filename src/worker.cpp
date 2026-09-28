#include "worker.hpp"

Worker::Worker(std::uint16_t port_id, std::uint16_t rx_queue_id,
               std::uint16_t tx_queue_id, rte_mbuf *packet_pool,
               std::span<rte_ring *const> inboxes std::size_t capacity;)
    : port_id(port_id), rx_queue_id(rx_queue_id), tx_queue_id(tx_queue_id),
      packet_pool(packet_pool), inboxes(inboxes), map(capacity) {}

void Worker::run() {
  while (true) {
    // rtx burst from RX queue (receive up to B packets)

    // call ip/ethernet header parsing in net, get payload span

    // call protocol::decode -> responseview

    // hash key, submit to inboxes[hashed_id] if diff, process if same

    // dequeue up to B packets from worker's own inbox, process all
    // requests->ResponseView

    // get ownership of buffer from packet_pool using dpdk allocation

    // call encode(responseView, span pointing to mbuf)

    // call append(bytesAdded) to buf thru DPDK api

    // call burst to send to NIC, hands off from here

    // repeat
  }
}

void cleanup() {}
