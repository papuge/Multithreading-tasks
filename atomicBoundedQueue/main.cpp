//
//  main.cpp
//  atomicQueue
//
//  Created by Veronika on 10/29/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
using namespace std;

struct BoundedBuffer {
    int *buffer;
    int capacity;
    /*
        The buffer is empty if front_count == rear_count;
        The buffer is full if front_count + capacity == rear_count
     */
    atomic_int front_count;
    atomic_int rear_count;
    atomic_int overall_count {0};
    
    mutex lock;
    condition_variable not_full_cv;
    condition_variable not_empty_cv;
    
    BoundedBuffer(int capacity): capacity(capacity), head(0), tail(0), count(0) {
        buffer = new int[capacity];
    }
    
    ~BoundedBuffer() {
        delete [] buffer;
    }
    
    void push(uint8_t value) {
        while(true) {
            int rear = rear_count;
            int x = buffer[rear % capacity];
            // if smth changed or buffer is full
            if ((rear != rear_count) || (rear == front_count + capacity))
                continue;
            if (x == null) {
                // slot is empty
                if (buffer[rear % capacity].compare_exchange())
            }
        }
        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        count.fetch_add(1);
        overall_count.fetch_add(1);
        u_lock.unlock();
        not_empty_cv.notify_one();
    }
    
    bool pop(uint8_t &value) {
        unique_lock<mutex> u_lock(lock);
        if (not_empty_cv.wait_for(u_lock, std::chrono::milliseconds(1),  [this]() {return count != 0;})){
            not_full_cv.notify_one();
            u_lock.unlock();
            value = buffer[head];
            head = (head + 1) % capacity;
            count.fetch_sub(1);
            u_lock.unlock();
            not_full_cv.notify_one();
            return true;
        } else {
            return false;
        }
    }
};

void producer(BoundedBuffer &buffer, int num_tasks, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_tasks; i++) {
        buffer.push(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << elapsed.count() << " ms for producer " << thread_num << "\n";
}

void consumer(BoundedBuffer &buffer, int &local_counter, int all_tasks, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();
    while(true) {
        uint8_t val = 0;
        bool is_pop = buffer.pop(val);
        if(!is_pop && buffer.overall_count == all_tasks) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << elapsed.count() << " ms for consumer " << thread_num << "\n";
            return;
        }
        else if (is_pop) {
            local_counter += val;
        }
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
