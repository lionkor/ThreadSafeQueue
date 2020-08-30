#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <deque>
#include <shared_mutex>
#include <utility>
#include <cassert>
#ifndef ASSERT
#define ASSERT assert
#endif // ASSERT

using RWMutex = std::shared_mutex;
using ReadLock = std::shared_lock<RWMutex>;
using WriteLock = std::unique_lock<RWMutex>;

template<class T>
using SuperQueue = std::deque<T>;

template<class T>
class ThreadSafeQueue final : private SuperQueue<T>
{
private:
    mutable RWMutex m_mutex;

    bool is_valid_lock(ReadLock& lock) const {
        return lock.mutex() == &m_mutex;
    }

    bool is_valid_lock(WriteLock& lock) const {
        return lock.mutex() == &m_mutex;
    }

public:
    // --- read ---

    [[nodiscard]] const T& back(ReadLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::back();
    }

    [[nodiscard]] const T& front(ReadLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::front();
    }

    [[nodiscard]] T& back(WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::back();
    }

    [[nodiscard]] T& front(WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::front();
    }

    bool empty(ReadLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::empty();
    }

    bool empty(WriteLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::empty();
    }

    size_t size(ReadLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::size();
    }

    size_t size(WriteLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return SuperQueue<T>::size();
    }

    // -- write --

    void push(const T& value, WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        SuperQueue<T>::push_back(value);
    }

    void push(T&& value, WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        SuperQueue<T>::push_back(std::forward<T>(value));
    }

    [[nodiscard]] T pop(WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        auto front = std::move(SuperQueue<T>::front());
        SuperQueue<T>::pop_front();
        return front;
    }

    // --- locks ---

    [[nodiscard]] ReadLock acquire_read_lock() const {
        return ReadLock(m_mutex);
    }

    [[nodiscard]] WriteLock acquire_write_lock() const {
        return WriteLock(m_mutex);
    }

    using ConstIterator = typename SuperQueue<T>::const_iterator;
    using Iterator = typename SuperQueue<T>::iterator;

    class IterableView
    {
    private:
        ThreadSafeQueue& m_queue;
        ReadLock& m_lock;

    public:
        IterableView(ThreadSafeQueue& queue, ReadLock& lock)
            : m_queue(queue), m_lock(lock) {
            ASSERT(queue.is_valid_lock(lock));
        }

        ConstIterator begin() const { return m_queue.cbegin(); }

        ConstIterator end() const { return m_queue.cend(); }
    };

    class IterableWriteView
    {
    private:
        ThreadSafeQueue& m_queue;
        WriteLock& m_lock;

    public:
        IterableWriteView(ThreadSafeQueue& queue, WriteLock& lock)
            : m_queue(queue), m_lock(lock) {
            ASSERT(queue.is_valid_lock(lock));
        }

        ConstIterator begin() const { return m_queue.cbegin(); }
        ConstIterator end() const { return m_queue.cend(); }
        Iterator begin() { return m_queue.begin(); }
        Iterator end() { return m_queue.end(); }
    };

    IterableView acquire_iterable_view(ReadLock& lock) const {
        ASSERT(is_valid_lock(lock));
        return IterableView(*this, lock);
    }

    IterableWriteView acquire_iterable_write_view(WriteLock& lock) {
        ASSERT(is_valid_lock(lock));
        return IterableWriteView(*this, lock);
    }

    friend IterableView;
    friend IterableWriteView;
};

#endif // THREADSAFEQUEUE_H
