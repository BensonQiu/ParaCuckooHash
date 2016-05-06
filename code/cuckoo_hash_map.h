#ifndef CUCKOO_HASHMAP_H
#define CUCKOO_HASHMAP_H


template <typename T>
class CuckooHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;

        CuckooHashMap(int num_buckets=64);
        ~CuckooHashMap();

        T get(std::string key);
        void put(std::string key, T val);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };
        HashEntry **m_table;
        int m_num_buckets;

};

#include "cuckoo_hash_map.cpp"

#endif
