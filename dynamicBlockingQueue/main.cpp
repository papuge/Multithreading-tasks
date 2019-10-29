//
//  main.cpp
//  dynamicQueue
//
//  Created by Veronika on 10/23/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <queue>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <cassert>
using namespace std;

mutex q_mtx;
atomic_int global_counter {0};


void producer(queue<uint8_t> &q, int task_number, int thr_number) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < task_number; i++) {
        lock_guard<mutex> lock(q_mtx);
        q.push(1);
        global_counter.fetch_add(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << elapsed.count() << " ms for producer " << thr_number <<" thread \n";
}

bool pop(queue<uint8_t> &q, uint8_t &task) {
    lock_guard<mutex> lock(q_mtx);
    if (q.empty()) {
        this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (q.empty())
        return false;
    
    task = q.front();
    q.pop();
    return true;
}

void consumer(queue<uint8_t> &q, int &local_counter, int all_tasks, int thr_num) {
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        uint8_t task_value = 0;
        bool is_pop = pop(q, task_value);
        // consumer finishes task
        if (!is_pop && global_counter == all_tasks) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << elapsed.count() << " ms for consumer " << thr_num << "\n";
            return;
        }
        else if (is_pop) {
            local_counter += task_value;
        }
    }
}

int main(int argc, const char * argv[]) {
    cout << "Start\n";
    queue<uint8_t> blocking_queue;
    int producer_threads = 1;
    int consumer_threads = 1;
    int num_tasks = 4 * 1024 * 1024 / producer_threads;
    int sum = 0;
    vector<thread> producers(producer_threads);
    vector<thread> consumers(consumer_threads);
    // consumers local counters
    vector<int> local_counters(consumer_threads);
    
    for (int i = 0; i < producer_threads; i++) {
        producers[i] = thread(producer, ref(blocking_queue), num_tasks, i);
    }
    for (int i = 0; i < consumer_threads; i++) {
        consumers[i] = thread(consumer, ref(blocking_queue), ref(local_counters[i]), num_tasks * producer_threads, i);
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
