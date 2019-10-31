//
//  main.cpp
//  lockFreeQueue
//
//  Created by Veronika on 10/31/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
using namespace std;

struct LockFreeQueue {
private:
    struct Node {
        Node *next;
        uint8_t value;
    };

    std::atomic<Node*> m_head;
    std::atomic<Node*> m_tail;
    
public:
    std::atomic_int overall_counter{0};
    std::atomic_int count{0};
    
    LockFreeQueue(): m_head(NULL), m_tail(NULL) { }
    
    ~LockFreeQueue() {
        while(!is_empty()){
            Node* m_temp = m_head.load();
            m_head = m_temp->next;
            delete m_temp;
        }
    }
    
    bool is_empty() {
        Node *head = m_head.load();
        return (head == NULL ? true : false);
    }

    void push(uint8_t value) {
        while(true) {
            Node *new_node = new Node();
            new_node->next = NULL;
            new_node->value = value;

            Node *head = m_head.load();
            Node *tail = m_tail.load();
            if (head == NULL){
                if (m_head.compare_exchange_weak(head, new_node)
                    && m_tail.compare_exchange_weak(tail, new_node)){
                    return;
                }
            } else {
                Node *temp = tail;
                temp->next = new_node;
                if (m_tail.compare_exchange_weak(tail, new_node)){
                    return;
                }
            }
        }
    }
    
    bool pop(uint8_t &value) {
        while(true) {
            Node *head = m_head.load();
            if(head == NULL) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                head = m_head.load();
                if(head == NULL)
                    return false;
            }
            value = head->value;
            Node *temp = head;
            temp = temp->next;
            if(m_head.compare_exchange_weak(head, temp)){
                count.fetch_sub(1);
                return true;
            }
        }
    }
    
};

void producer(LockFreeQueue &queue, int num_tasks, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_tasks; i++) {
        queue.push(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    printf("%f ms for producer %d\n", elapsed.count(), thread_num);
}

void consumer(LockFreeQueue &queue, int &local_counter, int all_tasks, int thread_num) {
    auto start = std::chrono::high_resolution_clock::now();
    while(true) {
        uint8_t val = 0;
        bool is_pop = queue.pop(val);
        if(!is_pop) {
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
    LockFreeQueue queue;
    int producer_threads = 1;
    int consumer_threads = 1;
    int num_tasks = 4 * 1024 * 1024  / producer_threads;
    int sum = 0;
    vector<thread> producers(producer_threads);
    vector<thread> consumers(consumer_threads);
    // consumers local counters
    vector<int> local_counters(consumer_threads);
    
    for (int i = 0; i < producer_threads; i++) {
        producers[i] = thread(producer, ref(queue), num_tasks, i);
    }
    for (int i = 0; i < consumer_threads; i++) {
        consumers[i] = thread(consumer, ref(queue), ref(local_counters[i]), num_tasks * producer_threads, i);
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
