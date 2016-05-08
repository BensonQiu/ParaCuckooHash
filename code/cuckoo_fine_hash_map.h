#ifndef CUCKOO_FINE_HASHMAP_H
#define CUCKOO_FINE_HASHMAP_H


template <typename T>
class CuckooFineHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;

        CuckooFineHashMap(int num_buckets=64);
        ~CuckooFineHashMap();

        T get(std::string key);
        void put(std::string key, T val);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };
        HashEntry **m_table;
        int m_num_buckets;
        std::mutex* m_bucket_locks;

        void lock_helper(uint32_t h1, uint32_t h2);
        void unlock_helper(uint32_t h1, uint32_t h2);
};

#include "cuckoo_fine_hash_map.cpp"

#endif
