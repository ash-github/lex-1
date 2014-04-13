#ifndef STACK_H
#define STACK_H
#include <string.h>

#include "Limits.h"

template <class T>
class Stack
{
public:
	Stack();
	~Stack();
	bool isEmpty();
	bool isFull();
	void clear();
	T& getAt(int n);
	int top();//top position, not value
	void pop();
	bool push(const T& x);
	const T& peek();
private:
	T element[PARSESTACK_SIZE];
	int _top;
};

template <class T>
Stack<T>::Stack()
{
	memset(element,0,sizeof(element));
	_top=-1;
}

template <class T>
Stack<T>::~Stack()
{
}

template <class T>
bool Stack<T>::isEmpty()
{
	return _top==-1;
}

template <class T>
bool Stack<T>::isFull()
{
	return _top>=PARSESTACK_SIZE-1;
}

template <class T>
const T& Stack<T>::peek()
{
	if(!isEmpty())
		return element[_top];
}

template <class T>
void Stack<T>::pop()
{
	if(!isEmpty())
		--_top;
}

template <class T>
T& Stack<T>::getAt(int n)
{
	if(n>=0&&n<=_top)
		return element[n];
}

template <class T>
int Stack<T>::top()
{
	return _top;
}

template <class T>
void Stack<T>::clear()
{
	memset(element,0,sizeof(element));
	_top=-1;
}

template <class T>
bool Stack<T>::push(const T& x)
{
	if(isFull()) return false;

	_top++;	
	element[_top]=x;
	return true;
}

#endif

