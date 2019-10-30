//
//  main.cpp
//  blockingCounter
//
//  Created by Veronika on 10/7/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>


void lock_counter(int numTasks, int numThreads, bool verbose = false) {
    std::vector<uint8_t> array(numTasks, 0);
    std::vector<std::thread> threads(numThreads);
    std::mutex mtx;
    unsigned int shared_counter = 0;
    unsigned int loops = numTasks / numThreads;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        threads[i] = std::thread([&] {
            for (int i = 0; i < loops; i++) {
                mtx.lock();
                array[shared_counter++] += 1;
                //std::this_thread::sleep_for(std::chrono::nanoseconds(10));
                mtx.unlock();
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << elapsed.count() << " ms for "
            << numThreads << " threads \n";
    if (verbose)
        for (auto a: array)
            std::cout << unsigned(a) << " ";
}

int main(int argc, const char * argv[]) {
    for (int i = 4; i <= 32; i *= 2)
        lock_counter(1024 * 1024, i);

    return 0;
}
