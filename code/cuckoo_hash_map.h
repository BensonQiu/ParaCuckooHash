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
        bool remove(std::string key);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };
        HashEntry **m_table;
        int m_num_buckets;

        inline int fastrand() const {
            int g_seed = (214013*g_seed+2531011);
            return (g_seed>>16)&0x7FFF;
        }


};

#include "cuckoo_hash_map.cpp"

#endif
