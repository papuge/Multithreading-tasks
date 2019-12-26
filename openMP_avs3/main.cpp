//
//  main.cpp
//  openMP_avs3
//
//  Created by Veronika on 10/30/19.
//  Copyright © 2019 Veronika. All rights reserved.
//

#include <iostream>
#include <omp.h>
#include <vector>

#define a_rows_num 1024
#define a_cols_num 1024
#define b_elem_num 1024

using namespace std;

int main(int argc, const char * argv[]) {
    assert (a_cols_num == b_elem_num);
    
    vector<vector<int>> a_matrix;
    a_matrix.resize(a_rows_num, vector<int>(a_cols_num, 1));
    vector<int> b_vector(b_elem_num, 1);
    vector<int> c_vector(a_rows_num);
    
    alignas(128) int i;
    alignas(128) int j;
    
    double total_begin = omp_get_wtime();
    
    #pragma omp parallel for default(shared) private(i, j)
    for (i = 0; i < a_rows_num; i++) {
        int row_sum = 0;
        for (j = 0; j < a_cols_num; j++) {
            row_sum += a_matrix[i][j] * b_vector[j];
        }
        c_vector[i] = row_sum;
    }
    
    double total_end = omp_get_wtime();

    
    double total_begin1 = omp_get_wtime();
    for (int i = 0; i < a_rows_num; i++) {
        for (int j = 0; j < a_cols_num; j++) {
            c_vector[i] += a_matrix[i][j] * b_vector[j];
        }
    }
    double total_end1 = omp_get_wtime();
    
    printf("Speed up - %f\n", (total_end1 - total_begin1) / (total_end - total_begin));
//    for (auto elem: c_vector) {
//        cout << elem << "\n";
//    }
    return 0;
}
