#include <limits.h>
#include <stdio.h>

void print_matrix(int *tab, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%4d", tab[(i * size) + j]);
        }
        printf("\n");
    }
}

void copy_array(int *src, int *dist, int size) {
    for (int i = 0; i < size * size; i++) {
        dist[i] = src[i];
    }
}

void fill_array(int *arr, int value, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

void fill_basis(int *basis, int *tab, int size) {
    int min;

    for (int i = 0; i < size; i++) {
        min = 0;

        for (int j = 1; j < size; j++) {
            if (tab[(i * size) + min] > tab[(i * size) + j]) {
                min = j;
            }
        }

        basis[i] = min;
    }
}

int get_multi_basis_col(int *basis, int size) {
    int count[size];

    fill_array(count, 0, size);

    for (int i = 0; i < size; i++) {
        count[basis[i]] += 1;

        if (count[basis[i]] > 1) {
            return basis[i];
        }
    }

    return 0;
}

void init_sets(int *set_a, int *set_b, int size) {
    fill_array(set_a, 1, size);
    fill_array(set_b, 0, size);
}

void transfer(int *src, int *dist, int pos) {
    src[pos] = 0;
    dist[pos] = 1;
}

int contains_basis(int *basis, int col, int size) {
    for (int i = 0; i < size; i++) {
        if (basis[i] == col) {
            return 1;
        }
    }

    return 0;
}

void solve(int *matrix, int *solution, int size) {
    int basis[size];
    int *ptr_basis = basis;
    int set_a[size], set_b[size]; // A' & A

    fill_basis(basis, matrix, size);

    while (1) {
        // step 1
        int col = get_multi_basis_col(basis, size);

        if (!col) {
            break;
        }

        init_sets(set_a, set_b, size);
        transfer(set_a, set_b, col);

        int has_basis = 0;
        int min, c_row, c_col, min_a, temp, d_col;

        do {
            min = INT_MAX;
            c_row = -1;
            c_col = -1;

            // step 2
            for (int i = 0; i < size; i++) {
                if (!set_b[basis[i]]) {
                    continue;
                }

                min_a = -1;

                for (int j = 0; j < size; j++) {
                    if (set_b[j]) {
                        continue;
                    }

                    if (min_a == -1 ||
                        matrix[(i * size) + min_a] > matrix[(i * size) + j]) {
                        min_a = j;
                    }
                }

                int temp =
                    matrix[(i * size) + min_a] - matrix[(i * size) + basis[i]];

                if (min > temp) {
                    min = temp;
                    c_row = i;
                    c_col = min_a;
                    d_col = basis[i];
                }
            }

            // step 3
            for (int i = 0; i < size; i++) {
                if (!set_b[i]) {
                    continue;
                }

                for (int j = 0; j <= size; j++) {
                    matrix[(j * size) + i] += min;
                }
            }

            // step 5
            has_basis = contains_basis(basis, c_col, size);

            if (has_basis) {
                transfer(set_a, set_b, c_col);
                continue; // back to step 2
            }

            // step 6
            basis[c_row] = c_col;

            // step 8
            if (contains_basis(basis, d_col, size)) {
                break; // back to step 1
            }

        } while (has_basis);
    }

    for (int i = 0; i < size; i++) {
        solution[i] = basis[i];
    }
}

void print_solution(int *matrix, int *solution, int size) {
    int sum = 0, tmp;

    for (int i = 0; i < size; i++) {
        tmp = matrix[(i * size) + solution[i]];
        sum += tmp;
        printf("M%d -> T%d (%d)\n", i + 1, solution[i] + 1, tmp);
    }

    printf("Total: %d", sum);
}

int main() {
    int size = 5;

    // clang-format off
    int matrix[5][5] = {
        {10, 5, 9,18,11},
        {13,19, 6,12,14},
        { 3, 2, 4, 4, 5},
        {18, 9,12,17,15},
        {11, 6,14,19,10},
    };

    // clang-format on
    int matrix_cp[5][5];
    int solution[5];

    int *ptr = (int *)matrix;
    int *ptr_cp = (int *)matrix_cp;
    int *ptr_s = solution;

    copy_array(ptr, ptr_cp, size);

    printf("---- Initial matrix ----\n");
    print_matrix(ptr, size);

    solve(ptr_cp, solution, size);

    printf("---- Final matrix ----\n");
    print_matrix(ptr_cp, size);
    printf("---- Solution ----\n");
    print_solution(ptr, solution, size);

    return 0;
}
