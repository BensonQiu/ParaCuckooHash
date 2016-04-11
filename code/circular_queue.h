#ifndef CIRCULAR_QUEUE
#define CIRCULAR_QUEUE

template <typename T>
class CircularQueue {

	public:
		CircularQueue(int capacity=1024);
		~CircularQueue();

		bool push(T val);
		bool pop(T* val);
		bool is_full();
		bool is_empty();

	private:
		int m_capacity;
		T* m_data;
		int m_head;
		int m_tail;

		int mod_capacity(int n);
};

#include "circular_queue.cpp"

#endif