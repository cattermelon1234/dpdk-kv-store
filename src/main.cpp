rte_mempool *pool =
    rte_pktmbuf_pool_create("packet_pool", num_mbufs, cache_size, 0,
                            RTE_MBUF_DEFAULT_BUF_SIZE, socket_id;)
