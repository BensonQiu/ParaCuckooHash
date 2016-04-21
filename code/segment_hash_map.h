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
        T get(std::string key, int h1, int h2);
        void put(std::string key, T val, int h1, int h2);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };

        HashEntry **m_table;
        int m_num_buckets;
        uint32_t* m_key_version_counters;
        std::mutex m_write_mutex;

        inline int fastrand() const {
            int g_seed = (214013*g_seed+2531011);
            return (g_seed>>16)&0x7FFF;
        }

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
        // class Segment {

        //     public:
        //         const int SLOTS_PER_BUCKET = 4;
        //         const int MAX_ITERS = 500;
        //         const int NUM_KEY_VERSION_COUNTERS = 8192;

        //         Segment(int num_buckets);
        //         ~Segment();
        //         T get(std::string key, int h1, int h2);
        //         void put(std::string key, T val, int h1, int h2);

        //     private:
        //         struct HashEntry {
        //             std::string key;
        //             T val;
        //         };

        //         HashEntry **m_table;
        //         int m_num_buckets;
        //         uint32_t* m_key_version_counters;
        //         std::mutex m_write_mutex;
        // };

        // HashEntry **m_table;
        // int m_num_buckets;
        // uint32_t* m_key_version_counters;
        // std::mutex m_write_mutex;
        Segment<T>** m_segments;
        int m_concurrency;
};

#include "segment_hash_map.cpp"

#endif
