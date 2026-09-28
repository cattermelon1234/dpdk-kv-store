class Worker {
public:
  std::uint16_t port_id;
  std::uint16_t rx_queue_id;
  std::uint16_t tx_queue_id;

  void run() {
    while (true) {
      // packet parsing hot path
    }
  }

  void cleanup() {}
}
