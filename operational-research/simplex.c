#include <stdio.h>

void print_tab(float *tab, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            printf("%5.2f  ", tab[(i * col) + j]);
        }
        printf("\n");
    }
}

int pivot_col(float *tab, int col) {
    int min = -1;

    for (int i = 0; i < col - 1; i++) {
        if (tab[i] < 0 && (min == -1 || tab[min] > tab[i])) {
            min = i;
        }
    }

    return min;
}

int pivot_row(float *tab, int row, int col, int pivot_col) {
    int min_index = -1;
    float min_ratio, ratio;

    for (int i = 1; i < row; i++) {
        ratio = tab[(i * col) + (col - 1)] / tab[(i * col) + pivot_col];

        if (ratio > 0 && (min_index == -1 || min_ratio > ratio)) {
            min_ratio = ratio;
            min_index = i;
        }
    }

    return min_index;
}

void pivot(float *tab, int row, int col, int pivot_row, int pivot_col) {
    float pivot_val = tab[(pivot_row * col) + pivot_col];

    for (int i = 0; i < col; i++) {
        tab[(pivot_row * col) + i] /= pivot_val;
    }

    for (int i = 0; i < row; i++) {
        if (i != pivot_row) {
            float factor = tab[(i * col) + pivot_col];
            for (int j = 0; j < col; j++) {
                tab[(i * col) + j] -= factor * tab[(pivot_row * col) + j];
            }
        }
    }
}

int has_negative(float *tab, int col) {
    for (int i = 0; i < col; i++) {
        if (tab[i] < 0) {
            return 1;
        }
    }

    return 0;
}

void simplex(float *tab, int row, int col) {
    while (has_negative(tab, col)) {
        int pc = pivot_col(tab, col);
        int pr = pivot_row(tab, row, col, pc);

        pivot(tab, row, col, pr, pc);
    }
}

void fill_solution(float *tab, int row, int col, float *solutions) {
    for (int i = 1; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (tab[j] == 0 && tab[(i * col) + j] == 1) {
                solutions[j] = tab[(i * col) + col - 1];
            }
        }
    }
}

void print_solution(float *solutions, int col) {
    for (int i = 0; i < col; i++) {
        printf("x%d: %.2f\n", i + 1, solutions[i]);
    }
}

float get_optimum(float *solutions, float *fn, int fn_len) {
    float max = 0;

    for (int i = 0; i < fn_len; i++) {
        max += fn[i] * solutions[i];
    }

    return max;
}

int main() {
    int row = 4, col = 6, fn_len = 2;

    float solutions[6], optimum;
    float objective_fn[2] = {1, 2}; // the function to maximize

    // clang-format off
    float tableau[4][6] = {
        {-1,-2, 0, 0, 0, 0},
        {-3, 2, 1, 0, 0, 2},
        {-1, 2, 0, 1, 0, 4},
        { 1, 1, 0, 0, 1, 5},
    };

    // clang-format on
    float *ptr = (float *)tableau;
    float *ptr_s = (float *)solutions;

    printf("---- Before -----\n");
    print_tab(ptr, row, col);
    printf("---- After -----\n");
    simplex(ptr, row, col);
    print_tab(ptr, row, col);

    fill_solution(ptr, row, col, ptr_s);
    printf("---- Solutions -----\n");
    print_solution(ptr_s, col);
    printf("---- Optimum -----\n");
    optimum = get_optimum(ptr_s, (float *)objective_fn, fn_len);
    printf("Result: %.2f\n", optimum);

    return 0;
}
