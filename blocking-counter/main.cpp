//
//  main.cpp
//  counter
//
//  Created by Veronika on 10/7/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>


unsigned int NUM = 1024 * 1024;
unsigned int N_THREADS = 16;
std::vector<std::thread> threads;
std::vector<unsigned char> array(NUM, 0);
unsigned int shared_counter = 0;
std::mutex mtx;


void thread_task_mutex() {
    std::unique_lock<std::mutex> lock(mtx);
    while (shared_counter < NUM){
        array[shared_counter] += 1;
        shared_counter++;
        // std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}

int main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_THREADS; i++) {
        threads.push_back(std::thread(thread_task_mutex));
    }
    for (auto &t: threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << elapsed.count() << " ms for "
            << N_THREADS << " threads \n";
//    for (auto a: array)
//        std::cout << a << " ";

    return 0;
}
