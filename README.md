# ThreadSafeQueue

A queue that provides hard-to-mess-up thread safety.

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

`ThreadSafeQueue` uses a *shared mutex* to control access while allowing for multiple concurrent readers.

In general, whenever you want to access the queue in any way, you are required to pass a valid lock. 
This makes it super difficult to mess up. 
Further, its verified that the lock you are passing actually locks the queue's mutex.

Acceptable locks are `ReadLock` and `WriteLock`. 
They can be acquired from the queue with `queue.acquire_read_lock()` and `queue.acquire_write_lock()`. 
Keep in mind that these locks are "scoped", so the only way to release the lock is to destroy them. 
Because of this, you should make a tight scope for each write/read (like in the [usage example](#usage-example)).

It's **extremely important** not to forget that deadlocks can still occur. You have to ensure that you never *ever* have nested locks, and that you never have two locks to the same queue in the same scope.

In the following sections both locks are explained in greater detail.

#### ReadLock

`ReadLock` locks the queue for reading. 
This is a *shared* lock, so there is no limit to the amount of readers that hold a read-lock to the queue. 
Any amount of threads can concurrently read from the same queue, each with their own lock. 

It's not possible to acquire a non-const-reference to any element or modify the queue in any way.

It is guaranteed that the queue will not change while a `ReadLock` is held.

A `ReadLock` will block only when the queue is being written to (blocked by a write lock).

#### WriteLock

`WriteLock` locks the queue for writing. This lock is *exclusive*, so it will block until all other read- and write-locks have been released. 
This means that, once locked successfully, it will also cause all threads requesting a `ReadLock` to block. 

For reading operations like `ThreadSafeQueue<T>::size()` an overload exists that accepts a `WriteLock`. This means that a write lock is allowed as a lock for all operations, as it is by nature an "exclusive" lock, so this is safe. But, since there can only be one `WriteLock` at one time, it's recommended to only use a `WriteLock` when you're going to modify the queue *for sure*.

`WriteLock`s should be used extremely conservatively. An example follows where we want to pop from the queue if it's not empty, and check for emptiness every loop iteration. In the example code, we are extremely conservative on locks and thus quite verbose. This is the way `ThreadSafeQueue` is meant to be used and this is how it's the fastest. We also have to avoid that two locks to the same queue are alive in the same scope, as this will cause a deadlock.

```cpp
while (...) {
    bool empty;
    { // read scope
        ReadLock lock = queue.acquire_read_lock();
        empty = queue.empty(lock);
    }
    if (!empty) {
        // only acquire write lock if not empty, not before, as it blocks all other locks
        WriteLock lock = queue.acquire_write_lock();
        my_popped_value = queue.pop(lock);
    }
}
```

