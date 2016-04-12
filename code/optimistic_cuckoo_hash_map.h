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

        inline int fastrand() const {
            int g_seed = (214013*g_seed+2531011);
            return (g_seed>>16)&0x7FFF;
        }
};

#include "optimistic_cuckoo_hash_map.cpp"

#endif
