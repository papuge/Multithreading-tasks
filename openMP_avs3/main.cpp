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

#define a_rows_num 5
#define a_cols_num 6
#define b_elem_num 6

using namespace std;

int main(int argc, const char * argv[]) {
    assert (a_cols_num == b_elem_num);
    
    vector<vector<int>> a_matrix;
    a_matrix.resize(a_rows_num, vector<int>(a_cols_num, 1));
    vector<int> b_vector(b_elem_num, 1);
    vector<int> c_vector(a_rows_num);
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < a_rows_num; i++) {
            int row_sum = 0;
            for (int j = 0; j < a_cols_num; j++) {
                row_sum += a_matrix[i][j] * b_vector[j];
            }
            c_vector[i] = row_sum;
        }
    }
    for (auto elem: c_vector) {
        cout << elem << " ";
    }
    return 0;
}
