#pragma once
#pragma once
#include<queue>
#include<mutex>
#include<condition_variable>

template<class T>
class BlockingQueue : public std::queue<T>
{
public:
   BlockingQueue(int size)
   {
      m_MaxSize = size;
   }

   // Put an element to the std::queue. Will wait if the std::queue is full.
   void Put(T item)
   {
      std::unique_lock<std::mutex> wlck(m_Mutex);
      while (Full())
      {
         m_FullCondition.wait(wlck);
      }
      std::queue<T>::push(item);
      m_EmptyCondition.notify_all();
   }

   // Take an element from the std::queue. Will wait if the std::queue is empty
   T Take()
   {
      std::unique_lock<std::mutex> lck(m_Mutex);
      while (std::queue<T>::empty())
      {
         m_EmptyCondition.wait(lck);
      }
      T element = std::queue<T>::front();
      std::queue<T>::pop();
      m_FullCondition.notify_all();
      return element;
   }
private:


   bool notEmpty()
   {
      return !std::queue<T>::empty();
   }

   bool Full()
   {
      return std::queue<T>::size() >= m_MaxSize;
   }



private:
   int m_MaxSize;
   std::mutex m_Mutex;
   std::condition_variable m_FullCondition;
   std::condition_variable m_EmptyCondition;
};


