template <typename T>
CircularQueue<T>::CircularQueue(int capacity) {

	// m_tail points to where the next element should be pushed.
	// Therefore, internally set m_capacity to capacity+1
	// to store "capacity" items.
	m_capacity = capacity+1;
	m_data = new T[capacity+1];
	m_head = 0;
	m_tail = 0;
}

template <typename T>
CircularQueue<T>::~CircularQueue() {

	delete[] m_data;
}

template <typename T>
bool CircularQueue<T>::push(T val) {

	// std::cout << "Start of push: head = " << m_head << " tail = " << m_tail << std::endl;

	// If tail is right before head, the queue is full.
	if (m_tail == this->mod_capacity(m_head-1))
		return false;

	m_data[m_tail] = val;
	m_tail = this->mod_capacity(m_tail+1);
	return true;
}

template <typename T>
bool CircularQueue<T>::pop(T* val) {

	// std::cout << "Start of pop: head = " << m_head << " tail = " << m_tail << std::endl;

	// Check if queue is empty.
	if (m_head == m_tail)
		return false;

	*val = m_data[m_head];
	m_head = this->mod_capacity(m_head+1);
	return true;
}

template <typename T>
bool CircularQueue<T>::is_empty() {

	return m_head == m_tail;
}

template <typename T>
bool CircularQueue<T>::is_full() {

	return m_tail == this->mod_capacity(m_head-1);
}

template <typename T>
int CircularQueue<T>::mod_capacity(int n) {

	n = n % m_capacity;
	if (n < 0)
		n = m_capacity - abs(n);
	return n;
}
