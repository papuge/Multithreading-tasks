//
//  main.cpp
//  boundedQueue
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
    uint8_t *buffer;
    int capacity;
    uint8_t head;
    uint8_t tail;
    int count;
    atomic_int overall_count {0};
    
    mutex lock;
    condition_variable not_full_cv;
    condition_variable not_empty_cv;
    
    
    BoundedBuffer(int capacity): capacity(capacity), head(0), tail(0), count(0) {
        buffer = new uint8_t[capacity];
    }
    
    ~BoundedBuffer() {
        delete [] buffer;
    }
    
    void push(uint8_t value) {
        unique_lock<mutex> u_lock(lock);
        not_full_cv.wait(u_lock, [this](){return count != capacity;});
        // acquiring lock
        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        count++;
        overall_count.fetch_add(1);
        not_empty_cv.notify_one();
    }
    
    bool pop(uint8_t &value) {
        unique_lock<mutex> u_lock(lock);
        if (not_empty_cv.wait_for(u_lock, std::chrono::milliseconds(1),  [this]() {return count != 0;})){
            value = buffer[head];
            head = (head + 1) % capacity;
            count--;
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
    BoundedBuffer buffer(16);
    int producer_threads = 2;
    int consumer_threads = 2;
    int num_tasks = 4 * 1024 * 1024 / producer_threads;
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
    
    return 0;
}
