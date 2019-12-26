//
//  main.cpp
//  atomicBoundedQueue
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
    atomic<uint8_t> * buffer;
    int capacity;
    /*
        The buffer is empty if front_count == rear_count;
        The buffer is full if front_count + capacity == rear_count
     */
    atomic_int front_count;
    atomic_int rear_count;

    BoundedBuffer(int capacity): capacity(capacity), front_count{0}, rear_count{0} {
        buffer = new atomic<uint8_t>[capacity];
        for(int i = 0; i < capacity; i++)
            buffer[i] = 0;
    }
    
    ~BoundedBuffer() {
        delete [] buffer;
    }
    
    void push(uint8_t value) {
        while(true) {
            int rear = rear_count.load();
            // if buffer is full
            if (rear == front_count + capacity)
                continue;
            
            uint8_t x = buffer[rear % capacity].load();
            if (x == 0) {
                if(rear_count.compare_exchange_strong(rear, rear + 1)) {
                    if(buffer[rear % capacity].compare_exchange_strong(x, value)) {
                        return;
                    }
                }
            }
        }
    }
    
    bool pop(uint8_t &value) {
        while (true) {
            int front = front_count.load();
            // if buffer is empty
            if (rear_count == front) {
                this_thread::sleep_for(std::chrono::milliseconds(1));
                front = front_count.load();
                if (rear_count == front) {
                    return false;
                }
            }
            uint8_t x = buffer[front % capacity].load();
            if(x != 0) {
                if(front_count.compare_exchange_strong(front, front + 1)) {
                    if(buffer[front % capacity].compare_exchange_strong(x, 0)) {
                        value = x;
                        return true;
                    }
                }
            }
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
    printf("%f ms for producer %d\n", elapsed.count(), thread_num);
}

void consumer(BoundedBuffer &buffer, int &local_counter, int all_tasks, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();
    while(true) {
        uint8_t val = 0;
        bool is_pop = buffer.pop(val);
        if(!is_pop && buffer.rear_count == all_tasks) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            printf("%f ms for consumer %d\n", elapsed.count(), thread_num);
            return;
        }
        else if (is_pop) {
            local_counter += val;
        }
    }
}

int main(int argc, const char * argv[]) {
    BoundedBuffer buffer(16);
    int producer_threads = 4;
    int consumer_threads = 4;
    int num_tasks = 4 * 1024 * 1024  / producer_threads;
    int sum = 0;
    vector<thread> producers(producer_threads);
    vector<thread> consumers(consumer_threads);
    // consumers local counters
    vector<int> local_counters(consumer_threads);
    
    for (int i = 0; i < producer_threads; i++) {
        producers[i] = thread(producer, ref(buffer), num_tasks, i);
    }
    for (int i = 0; i < consumer_threads; i++) {
        consumers[i] = thread(consumer, ref(buffer), ref(local_counters[i]), num_tasks * producer_threads, i);
    }
    for (int i = 0; i < producer_threads; i++) {
        producers[i].join();
    }
    for (int i = 0; i < consumer_threads; i++) {
        consumers[i].join();
    }
    for (auto elem: local_counters) {
        sum += elem;
    }
    cout << "Sum is " << sum << "\n";
    assert(sum == num_tasks * producer_threads);
}
