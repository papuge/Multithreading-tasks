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
            int copy = 0;
            while (true) {
                copy = atomic_counter.fetch_add(1);
                if (copy >= numTasks) {
                    return;
                }
                array.at(copy)++;
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
    // std::cout << "at.c " << atomic_counter << "\n";
    if (verbose)
        for (auto a: array)
            std::cout << unsigned(a) << " ";
    
    if (std::all_of(array.cbegin(), array.cend(), [](int i) {
        return i == 1;
    })) {
        std::cout << "All are 1" << std::endl;
    } else {
        std::cout << "Not all are 1" << std::endl;
    }
    
}

int main(int argc, const char * argv[]) {
    for (int i = 4; i <= 32; i *= 2)
        lockfree_counter(1024 * 1024, i);

    return 0;
}
