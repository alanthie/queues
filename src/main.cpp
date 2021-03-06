#include <iostream>
#include <thread>

#include "mpmc-bounded-queue.hpp"
#include "mpsc-queue.hpp"
#include "spsc-bounded-queue.hpp"
#include "spsc-queue.hpp"

//#define COUNT 100000000
#define COUNT 1000000

template <typename T> void consumer_func(T *queue) 
{
  size_t count = COUNT;
  size_t value = 0;
  
  while (count > 0) {
    if (queue->dequeue(value)) {
      --count;
    }
  }
}

template <typename T> void bounded_producer_func(T *queue) {
  size_t count = COUNT;
  while (count > 0) {
    if (queue->enqueue(count)) {
      --count;
    }
  }
}

template <typename T> void producer_func(T *queue) {
  for (int count = 0; count < COUNT; ++count) {
    queue->enqueue(count);
  }
}

template <typename T> long double run_test(T producer_func, T consumer_func) 
{
  typedef std::chrono::high_resolution_clock aclock_t;
  typedef std::chrono::time_point<aclock_t> atime_t;
  atime_t start;
  atime_t end;

  start = aclock_t::now();
  std::thread producer(producer_func);
  std::thread consumer(consumer_func);

  producer.join();
  consumer.join();
  end = aclock_t::now();

  return (end - start).count() * ((double)aclock_t::period::num / aclock_t::period::den);
}

int main() 
{
  {
    typedef spsc_bounded_queue_t<size_t> queue_t;
    queue_t queue(65536);
    long double seconds =
        run_test(std::bind(&bounded_producer_func<queue_t>, &queue),
                 std::bind(&consumer_func<queue_t>, &queue));

    std::cout << "SPSC bound queue completed " << COUNT << " iterations in "
              << seconds << " seconds. "
              << ((long double)COUNT / seconds) / 1000000
              << " million enqueue/dequeue pairs per second." << std::endl;
  }

  {
    typedef mpmc_bounded_queue_t<size_t> queue_t;
    queue_t queue(65536);
    long double seconds =
        run_test(std::bind(&bounded_producer_func<queue_t>, &queue),
                 std::bind(&consumer_func<queue_t>, &queue));

    std::cout << "MPMC bound queue completed " << COUNT << " iterations in "
              << seconds << " seconds. "
              << ((long double)COUNT / seconds) / 1000000
              << " million enqueue/dequeue pairs per second." << std::endl;
  }

  {
    typedef spsc_queue_t<size_t> queue_t;
    queue_t queue;
    long double seconds = run_test(std::bind(&producer_func<queue_t>, &queue),
                                   std::bind(&consumer_func<queue_t>, &queue));

    std::cout << "SPSC dynamic queue completed " << COUNT << " iterations in "
              << seconds << " seconds. "
              << ((long double)COUNT / seconds) / 1000000
              << " million enqueue/dequeue pairs per second." << std::endl;
  }

  {
    typedef mpsc_queue_t<size_t> queue_t;
    queue_t queue;
    long double seconds = run_test(std::bind(&producer_func<queue_t>, &queue),
                                   std::bind(&consumer_func<queue_t>, &queue));

    std::cout << "MPSC dynamic queue completed " << COUNT << " iterations in "
              << seconds << " seconds. "
              << ((long double)COUNT / seconds) / 1000000
              << " million enqueue/dequeue pairs per second." << std::endl;
  }

  system("pause");
  return 0;
}
