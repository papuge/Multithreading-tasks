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

struct Task {
    bool cease = false;
    uint8_t value;
    Task(): cease(true) { }
    Task(uint8_t val): value(val) { }
    Task(const Task &t): cease(t.cease), value(t.value) { }
};

void producer(queue<Task> &q, int task_number, int thr_number) {
    for (int i = 0; i < task_number; i++) {
        lock_guard<mutex> lock(q_mtx);
        q.push(Task(1));
        cout << thr_number << " pushing\n";
    }
    // signalTask
    q.push(Task());
    cout << thr_number << " finished\n";
}

bool pop(queue<Task> &q, Task &task) {
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

void consumer(queue<Task> &q, int &local_counter, int thr_num) {
    while (true) {
        Task task;
        bool is_pop = pop(q, task);
        // consumer finishes task
        if (is_pop && task.cease) {
            cout << thr_num << " consumer finishes with" << local_counter <<"\n";
            return;
        }
        else if (is_pop) {
            local_counter += task.value;
            cout << thr_num << " consumer pops\n";
        }
    }
}

int main(int argc, const char * argv[]) {
    queue<Task> blocking_queue;
    int num_tasks = 5;
    int num_threads = 3;
    int sum = 0;
    vector<thread> producers(num_threads);
    vector<thread> consumers(num_threads);
    vector<int> local_counters(num_threads);
    for (int i = 0; i < num_threads; i++) {
        producers[i] = thread(producer, ref(blocking_queue), num_tasks, i);
    }
    for (int i = 0; i < num_threads; i++) {
        consumers[i] = thread(consumer, ref(blocking_queue), ref(local_counters[i]), i);
    }
    for (int i = 0; i < num_threads; i++) {
        producers[i].join();
    }
    for (int i = 0; i < num_threads; i++) {
        consumers[i].join();
    }
    for (auto elem: local_counters) {
        sum += elem;
    }
    cout << "Sum is " << sum << "\n";
    assert(sum == num_tasks * num_threads);
    return 0;
}
