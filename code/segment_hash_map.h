#ifndef SEGMENT_HASHMAP_H
#define SEGMENT_HASHMAP_H


template <typename T>
class Segment {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;
        const int NUM_KEY_VERSION_COUNTERS = 8192;

        Segment(int num_buckets);
        ~Segment();
        T get(std::string key, uint32_t h1, uint32_t h2);
        void put(std::string key, T val, uint32_t h1, uint32_t h2);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };

        HashEntry **m_table;
        int m_num_buckets;
        uint32_t* m_key_version_counters;
        std::mutex m_write_mutex;

};

template <typename T>
class SegmentHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;
        const int NUM_KEY_VERSION_COUNTERS = 8192;

        SegmentHashMap(int num_buckets=64, int concurrency=32);
        ~SegmentHashMap();

        T get(std::string key);
        void put(std::string key, T val);

    private:
        Segment<T>** m_segments;
        int m_concurrency;
};

#include "segment_hash_map.cpp"

#endif
