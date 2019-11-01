//
//  main.cpp
//  nonblockingCounter
//
//  Created by Veronika on 10/7/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>


void lockfree_counter(int numTasks, int numThreads, bool verbose = false) {
    std::vector<uint8_t> array(numTasks, 0);
    std::vector<std::thread> threads(numThreads);
    std::mutex mtx;
    std::atomic<unsigned int> atomic_counter {0};
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        threads[i] = std::thread([&] {
            int copy;
            while (copy = atomic_counter.fetch_add(1) < numTasks) {
                array[copy] += 1;
                //std::this_thread::sleep_for(std::chrono::nanoseconds(10));
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
        lockfree_counter(1024 * 1024, i);

    return 0;
}
