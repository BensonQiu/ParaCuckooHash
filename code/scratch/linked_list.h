#include <iostream>

#ifndef LINKEDLIST_H
#define LINKEDLIST_H


template <typename T>
class LinkedList {

	public:
		LinkedList();
		~LinkedList();
		void insert(T val);
		bool remove(T val);
		void print();

	private:
		struct Node {
			T val;
		    Node *next;
		};
		Node *head;

};

template <typename T>
LinkedList<T>::LinkedList() {

	head = NULL;
}

template <typename T>
LinkedList<T>::~LinkedList() {

	Node *temp;

	while (head != NULL) {
		temp = head;
		head = head->next;
		delete temp;
	}
}

template <typename T>
void LinkedList<T>::insert(T val) {

	Node *temp = new Node();
	temp->val = val;
	temp->next = head;
	head = temp;
}

template <typename T>
bool LinkedList<T>::remove(T val) {

	// Empty linked list.
	if (head == NULL)
		return false;

	// Remove from head.
	if (head->val == val) {
		Node *temp = head;
		head = head->next;
		delete temp;
		return true;
	}

	// General case.
	Node *prev = head;
	Node *curr = head->next;

	while (curr != NULL) {
		if (curr->val == val) {
			prev->next = curr->next;
			delete curr;
			return true;
		}
		prev = curr;
		curr = curr->next;
	}

	return false;
}

template <typename T>
void LinkedList<T>::print() {

	Node *curr = head;

	while (curr) {
		std::cout << curr->val << "\n";
		curr = curr->next;
	}
}

#endif
