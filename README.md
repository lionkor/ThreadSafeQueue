# ThreadSafeQueue

A wrapper around a queue type that provides hard-to-mess-up thread safety . 

## Usage Example

```cpp
ThreadSafeQueue<int> queue;

{ // write lock scope
    WriteLock lock = queue.acquire_write_lock();
    queue.push(123, lock);
    queue.push(456, lock);
    queue.push(5, lock);
}

{ // read lock scope
    ReadLock lock = queue.acquire_read_lock();
    std::cout << "first element: " << queue.front(lock) << std::endl;
}
```

## How to use

First, look at [the usage example](#usage-example).

### Locks

`ThreadSafeQueue` uses a mutex to control access. This mutex is a `std::shared_mutex`, to allow for multiple concurrent readers.

In general, whenever you want to access the queue in any way, you are required to pass a valid lock. 
This makes it super difficult to mess up. 
Further, its verified that the lock you are passing actually locks the queue's mutex.

Acceptable locks are `ReadLock` and `WriteLock`. 
They can be acquired from the queue with `queue.acquire_read_lock()` and `queue.acquire_write_lock()`. 
Keep in mind that these locks are "scoped", so the only way to release the lock is to destroy them. 
    Because of this, you should make a scope for each write/read (like in the [usage example](#usage-example)).

#### WriteLock






