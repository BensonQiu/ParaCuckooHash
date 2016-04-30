#ifndef OPTIMISTIC_CUCKOO_HASHMAP_H
#define OPTIMISTIC_CUCKOO_HASHMAP_H


template <typename T>
class OptimisticCuckooHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;
        const int NUM_KEY_VERSION_COUNTERS = 8192;

        OptimisticCuckooHashMap(int num_buckets=64);
        ~OptimisticCuckooHashMap();

        T get(std::string key);
        void put(std::string key, T val);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };
        HashEntry **m_table;
        int m_num_buckets;
        uint32_t* m_key_version_counters;
        std::mutex m_write_mutex;
        int* m_visited_bitmap;
};

#include "optimistic_cuckoo_hash_map.cpp"

#endif
