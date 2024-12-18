#include <iostream>
#include <cstdint>
#include <cstring>
#include <cfloat>
#include <new>
#include <chrono>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "readData.h"
#include "Data.hpp"

#define REINSERTION 1
#define OR_OPT_2 	2
#define OR_OPT_3 	3
#define SWAP 		4
#define TWO_OPT		5
#define TABLE_SZ    26
#define DBL_SZ      8
#define INT_SZ      4

#define MATRIX

using std::chrono::steady_clock;

typedef unsigned uint;

// 3D array
typedef std::vector<std::vector<std::vector<double>>> tSubseq;

typedef struct tInfo {
    double ** cost;
    int dimen;
    //uint T;
    //uint C;
    //uint W;
    vector<int> rnd;
    uint rnd_index;
} tInfo;

typedef struct tSeqInfo {
    double T;
    double C;
    double W;
} tSeqInfo;

typedef struct tSolution {
    std::vector<int> s;
#ifdef FLAT
    //double * seq;
    tSeqInfo * seq;
#elif defined(MATRIX)
    tSeqInfo ** seq;
    //double *** seq;
#endif
    //tSubseq seq;
    double cost;
} tSolution;

inline unsigned to_1D(int x, int y, int size) {
//inline unsigned to_1D(int x, int y, int z, int size) {
    int a = x*(size+1) + y;
    //int b = 3*x*(size+1) + 3*y + z;
    return a;
}

tSolution Solution_init(tInfo info) {
    tSolution solut;
    solut.s = vector<int>(info.dimen+1);

#ifdef FLAT
    solut.seq = new tSeqInfo [(info.dimen+1) * (info.dimen+1)];
    //solut.seq = new double [(info.dimen+1) * (info.dimen+1) *3];
#elif defined(MATRIX)
    /*
    solut.seq = new double ** [info.dimen+1];
    for (int i = 0; i < info.dimen+1; i++) {
        solut.seq[i] = new double * [info.dimen+1];
        for (int j = 0; j < info.dimen+1; j++) {
            solut.seq[i][j] = new double [3];
        }
    }
    */
    solut.seq = new tSeqInfo * [info.dimen+1];
    for (int i = 0; i < info.dimen+1; i++) {
        solut.seq[i] = new tSeqInfo [info.dimen+1];
        memset(solut.seq[i], 0.0, (info.dimen+1)*sizeof(tSeqInfo));
    }

#endif
    /*
    solut.seq = std::vector<std::vector<std::vector<double>>> (
            info.dimen+1, std::vector<std::vector<double>> (
                info.dimen+1, std::vector<double> (3)));
                */
    solut.cost = DBL_MAX;

    return solut;
}

void Solution_cpy( tSolution & src, tSolution & tgt, const tInfo & info) {

    tgt.s = src.s;
    tgt.cost = src.cost;

    /*
    for (int i = 0; i < info.dimen+1; i++) {
        for (int j = 0; j < info.dimen+1; j++) {
            //memcpy(tgt.seq[i][j], src.seq[i][j], 3 * sizeof(double));
            std::copy(src.seq[i][j], src.seq[i][j] + 3, tgt.seq[i][j]);
        }
    }
    */

}

double R_table(int i){
    static const double table[] = {0.00, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 
                                    0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25};
    return table[i];
}

void print_s(std::vector<int> s) {

    for (int i = 0; i < s.size(); i++)
        std::cout << s[i]+1 << " ";
    std::cout << std::endl;
}

int partition(std::vector<int>& arr, int left, int right, const tInfo& info, int r) {
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (info.cost[r][arr[j]] < info.cost[r][pivot]) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[right]);
    return i + 1;
}

void quicksort(std::vector<int>& arr, int left, int right, const tInfo& info, int r) {
    if (left < right) {
        int pivot = partition(arr, left, right, info, r);
        quicksort(arr, left, pivot - 1, info, r);
        quicksort(arr, pivot + 1, right, info, r);
    }
}

void sort(std::vector<int>& arr, int r, const tInfo& info) {
    quicksort(arr, 0, arr.size() - 1, info, r);
}

std::vector<int> construct(const double alpha, tInfo & info){

    std::vector<int> s = {0};
    s.reserve(info.dimen+1);
    std::vector<int> cL(info.dimen-1);
    for(int i = 1; i < info.dimen; ++i){
        cL[i-1] = i;
    }

    int r = 0;
    while (!cL.empty()) {
        sort(cL, r, info);

        int index = info.rnd[info.rnd_index++];

        int c = cL[index];
        s.push_back(c);
        //print_s(cL);
        r = c;
        cL.erase(cL.begin() + index);
    }

    s.push_back(0);

    return s;
}	

void swap(std::vector<int> &vec, int i, int j){
    std::iter_swap(vec.begin() + i, vec.begin() + j);
}

void reverse(std::vector<int> &vec, int i, int j){
    std::reverse(vec.begin() + i, vec.begin() + j+1);
}

void reinsert(std::vector<int> &vec, int i, int j, int pos){
    std::vector<int> seq (vec.begin() + i, vec.begin() +j+1);
    if(pos < i){
        vec.erase(vec.begin() + i, vec.begin() + j+1);
        vec.insert(vec.begin() + pos, seq.begin(), seq.end());
    }else{
        vec.insert(vec.begin() + pos, seq.begin(), seq.end());
        vec.erase(vec.begin() + i, vec.begin() + j+1);
    }

}

#ifdef FLAT

void subseq_load(tSolution & solut, const tInfo & info, int index = 0){
    alignas(INT_SZ) int i, j, j_prev, k;
    alignas(INT_SZ) int from = index;
    alignas(1) bool t;
    for (i = 0; i < info.dimen+1; i++) {
        k = 1 - i - (!i);
        t = i == from;

        solut.seq[to_1D(i, i, info.dimen)].T = 0.0;
        solut.seq[to_1D(i, i, info.dimen)].C = 0.0;
        solut.seq[to_1D(i, i, info.dimen)].W = (double) !(i == 0);

        for (j = i+1; j < info.dimen+1; j++) {
            j_prev = j-1;
            
            solut.seq[to_1D(i, j, info.dimen)].T = info.cost[solut.s[j_prev]][solut.s[j]] + solut.seq[to_1D(i, j_prev, info.dimen)].T;
            solut.seq[to_1D(i, j, info.dimen)].C = solut.seq[to_1D(i, j, info.dimen)].T + solut.seq[to_1D(i, j_prev, info.dimen)].C;
            solut.seq[to_1D(i, j, info.dimen)].W = j + k;
        }
        from += t;
    }

    solut.cost = solut.seq[to_1D(0, info.dimen, info.dimen)].C;
}

bool search_swap(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2, cost_concat_3, cost_concat_4;
    alignas(DBL_SZ) double cost_best = DBL_MAX;
    alignas(INT_SZ) int i, j, j_prev, j_next, i_prev, i_next;
    //alignas(INT_SZ) int dim = dimension - 2;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;
        i_next = i + 1;

        //consecutive nodes
        cost_concat_1 =                 solut.seq[to_1D(0, i_prev, info.dimen)].T + info.cost[solut.s[i_prev]][solut.s[i_next]];
        cost_concat_2 = cost_concat_1 + solut.seq[to_1D(i, i_next, info.dimen)].T  + info.cost[solut.s[i]][solut.s[i_next+1]];

        cost_new = solut.seq[to_1D(0, i_prev, info.dimen)].C                                                    +           //       1st subseq
        solut.seq[to_1D(i, i_next, info.dimen)].W               * (cost_concat_1) + info.cost[solut.s[i_next]][solut.s[i]]  +           // concat 2nd subseq
        solut.seq[to_1D(i_next+1, info.dimen, info.dimen)].W   * (cost_concat_2) + solut.seq[to_1D(i_next+1, info.dimen, info.dimen)].C;   // concat 3rd subseq

        if (cost_new < cost_best) {
            cost_best = cost_new - DBL_EPSILON;
            I = i;
            J = i_next;
        }

        for (j = i_next+1; j < info.dimen; ++j) {
            j_next = j + 1;
            j_prev = j - 1;

            cost_concat_1 =                 solut.seq[to_1D(0, i_prev, info.dimen)].T       + info.cost[solut.s[i_prev]][solut.s[j]];
            cost_concat_2 = cost_concat_1                           + info.cost[solut.s[j]][solut.s[i_next]];
            cost_concat_3 = cost_concat_2 + solut.seq[to_1D(i_next, j_prev, info.dimen)].T  + info.cost[solut.s[j_prev]][solut.s[i]];
            cost_concat_4 = cost_concat_3                           + info.cost[solut.s[i]][solut.s[j_next]];

            cost_new = solut.seq[to_1D(0, i_prev, info.dimen)].C                                                 +      // 1st subseq
            cost_concat_1 +                                                             // concat 2nd subseq (single node)
            solut.seq[to_1D(i_next, j_prev, info.dimen)].W      * cost_concat_2 + solut.seq[to_1D(i_next, j_prev, info.dimen)].C +      // concat 3rd subseq
            cost_concat_3 +                                                             // concat 4th subseq (single node)
            solut.seq[to_1D(j_next, info.dimen, info.dimen)].W * cost_concat_4 + solut.seq[to_1D(j_next, info.dimen, info.dimen)].C;   // concat 5th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        swap(solut.s, I, J);
        subseq_load(solut, info, I);
        return true;
    }

    return false;
}

bool search_two_opt(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2;
    alignas(DBL_SZ) double cost_best = DBL_MAX;// cost_l1, cost_l2;
    alignas(DBL_SZ) double rev_seq_cost;
    alignas(INT_SZ) int i, j, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;

        rev_seq_cost = solut.seq[to_1D(i, i+1, info.dimen)].T;
        for (j = i + 2; j < info.dimen; ++j) {
            j_next = j + 1;

            rev_seq_cost += info.cost[solut.s[j-1]][solut.s[j]] * (solut.seq[to_1D(i, j, info.dimen)].W-1.0);

            cost_concat_1 =                 solut.seq[to_1D(0, i_prev, info.dimen)].T   + info.cost[solut.s[j]][solut.s[i_prev]];
            cost_concat_2 = cost_concat_1 + solut.seq[to_1D(i, j, info.dimen)].T        + info.cost[solut.s[j_next]][solut.s[i]];

            cost_new = solut.seq[to_1D(0, i_prev, info.dimen)].C                                                        +   //  1st subseq
                    solut.seq[to_1D(i, j, info.dimen)].W                * cost_concat_1 + rev_seq_cost                  +   // concat 2nd subseq (reversed seq)
                    solut.seq[to_1D(j_next, info.dimen, info.dimen)].W * cost_concat_2 + solut.seq[to_1D(j_next, info.dimen, info.dimen)].C;      // concat 3rd subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reverse(solut.s, I, J);
        subseq_load(solut, info);
        return true;
    }

    return false;
}

bool search_reinsertion(tSolution & solut, const tInfo & info, const int opt) {
    alignas(DBL_SZ) double cost_new, cost_concat_1, cost_concat_2, cost_concat_3;
    alignas(DBL_SZ) double cost_best = DBL_MAX;//, cost_l1, cost_l2, cost_l3;
    alignas(INT_SZ) int i, j, k, k_next, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;
    alignas(INT_SZ) int POS;

    for (i = 1, j = opt +i-1; i < info.dimen-opt+1; ++i, ++j) {
        j_next = j + 1;
        i_prev = i - 1;

        //k -> edges 
        for(k = 0; k < i_prev; ++k){
            k_next = k+1;

            cost_concat_1 =                 solut.seq[to_1D(0, k, info.dimen)].T            + info.cost[solut.s[k]][solut.s[i]];
            cost_concat_2 = cost_concat_1 + solut.seq[to_1D(i, j, info.dimen)].T            + info.cost[solut.s[j]][solut.s[k_next]];
            cost_concat_3 = cost_concat_2 + solut.seq[to_1D(k_next, i_prev, info.dimen)].T  + info.cost[solut.s[i_prev]][solut.s[j_next]];

            cost_new = solut.seq[to_1D(0, k, info.dimen)].C                                                                   +   //       1st subseq
                solut.seq[to_1D(i, j, info.dimen)].W               * cost_concat_1 + solut.seq[to_1D(i, j, info.dimen)].C                  +   //  concat 2nd subseq (reinserted seq)
                solut.seq[to_1D(k_next, i_prev, info.dimen)].W     * cost_concat_2 + solut.seq[to_1D(k_next, i_prev, info.dimen)].C        +   //  concat 3rd subseq
                solut.seq[to_1D(j_next, info.dimen, info.dimen)].W * cost_concat_3 + solut.seq[to_1D(j_next, info.dimen, info.dimen)].C;       // concat 4th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }

        for (k = i + opt; k < info.dimen; ++k) {
            k_next = k + 1;

            cost_concat_1 =                 solut.seq[to_1D(0, i_prev, info.dimen)].T  + info.cost[solut.s[i_prev]][solut.s[j_next]];
            cost_concat_2 = cost_concat_1 + solut.seq[to_1D(j_next, k, info.dimen)].T  + info.cost[solut.s[k]][solut.s[i]];
            cost_concat_3 = cost_concat_2 + solut.seq[to_1D(i, j, info.dimen)].T       + info.cost[solut.s[j]][solut.s[k_next]];

            cost_new = solut.seq[to_1D(0, i_prev, info.dimen)].C                                                                  +   //       1st subseq
                solut.seq[to_1D(j_next, k, info.dimen)].W          * cost_concat_1 + solut.seq[to_1D(j_next, k, info.dimen)].C             +   // concat 2nd subseq
                solut.seq[to_1D(i, j, info.dimen)].W               * cost_concat_2 + solut.seq[to_1D(i, j, info.dimen)].C                  +   // concat 3rd subseq (reinserted seq)
                solut.seq[to_1D(k_next, info.dimen, info.dimen)].W * cost_concat_3 + solut.seq[to_1D(k_next, info.dimen, info.dimen)].C;       // concat 4th subseq


            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reinsert(solut.s, I, J, POS+1);
        subseq_load(solut, info, I < POS+1 ? I : POS+1);
        return true;
    }

    return false;
}

#elif defined(MATRIX)

void subseq_load(tSolution & solut, const tInfo & info, int index = 0){
    alignas(INT_SZ) int i, j, j_prev, k;
    alignas(INT_SZ) int from = index;
    alignas(1) bool t;
    for (i = 0; i < info.dimen+1; i++) {
        k = 1 - i - (!i);
        t = i == from;

        solut.seq[i][i].T = 0.0;
        solut.seq[i][i].C = 0.0;
        solut.seq[i][i].W = (double) !(i == 0);

        for (j = i+1; j < info.dimen+1; j++) {
            j_prev = j-1;
            
            solut.seq[i][j].T = info.cost[solut.s[j_prev]][solut.s[j]] + solut.seq[i][j_prev].T;
            solut.seq[i][j].C = solut.seq[i][j].T + solut.seq[i][j_prev].C;
            solut.seq[i][j].W = j + k;

        }
        from += t;
    }

    solut.cost = solut.seq[0][info.dimen].C;
}

bool search_swap(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2, cost_concat_3, cost_concat_4;
    alignas(DBL_SZ) double cost_best = DBL_MAX;
    alignas(INT_SZ) int i, j, j_prev, j_next, i_prev, i_next;
    //alignas(INT_SZ) int dim = dimension - 2;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;
        i_next = i + 1;

        //consecutive nodes
        cost_concat_1 =                 solut.seq[0][i_prev].T + info.cost[solut.s[i_prev]][solut.s[i_next]];
        cost_concat_2 = cost_concat_1 + solut.seq[i][i_next].T  + info.cost[solut.s[i]][solut.s[i_next+1]];

        cost_new = solut.seq[0][i_prev].C                                                    +           //       1st subseq
        solut.seq[i][i_next].W               * (cost_concat_1) + info.cost[solut.s[i_next]][solut.s[i]]  +           // concat 2nd subseq
        solut.seq[i_next+1][info.dimen].W   * (cost_concat_2) + solut.seq[i_next+1][info.dimen].C;   // concat 3rd subseq

        if (cost_new < cost_best) {
            cost_best = cost_new - DBL_EPSILON;
            I = i;
            J = i_next;
        }

        for (j = i_next+1; j < info.dimen; ++j) {
            j_next = j + 1;
            j_prev = j - 1;

            cost_concat_1 =                 solut.seq[0][i_prev].T       + info.cost[solut.s[i_prev]][solut.s[j]];
            cost_concat_2 = cost_concat_1                           + info.cost[solut.s[j]][solut.s[i_next]];
            cost_concat_3 = cost_concat_2 + solut.seq[i_next][j_prev].T  + info.cost[solut.s[j_prev]][solut.s[i]];
            cost_concat_4 = cost_concat_3                           + info.cost[solut.s[i]][solut.s[j_next]];

            cost_new = solut.seq[0][i_prev].C                                                 +      // 1st subseq
            cost_concat_1 +                                                             // concat 2nd subseq (single node)
            solut.seq[i_next][j_prev].W      * cost_concat_2 + solut.seq[i_next][j_prev].C +      // concat 3rd subseq
            cost_concat_3 +                                                             // concat 4th subseq (single node)
            solut.seq[j_next][info.dimen].W * cost_concat_4 + solut.seq[j_next][info.dimen].C;   // concat 5th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        swap(solut.s, I, J);
        subseq_load(solut, info, I);
        return true;
    }

    return false;
}

bool search_two_opt(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2;
    alignas(DBL_SZ) double cost_best = DBL_MAX;// cost_l1, cost_l2;
    alignas(DBL_SZ) double rev_seq_cost;
    alignas(INT_SZ) int i, j, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;

        rev_seq_cost = solut.seq[i][i+1].T;
        for (j = i + 2; j < info.dimen; ++j) {
            j_next = j + 1;


          rev_seq_cost += info.cost[solut.s[j-1]][solut.s[j]] * (solut.seq[i][j].W-1.0);

          cost_concat_1 =                 solut.seq[0][i_prev].T   + info.cost[solut.s[j]][solut.s[i_prev]];
          cost_concat_2 = cost_concat_1 + solut.seq[i][j].T        + info.cost[solut.s[j_next]][solut.s[i]];

          cost_new = solut.seq[0][i_prev].C                                                        +   //  1st subseq
              solut.seq[i][j].W                * cost_concat_1 + rev_seq_cost                  +   // concat 2nd subseq (reversed seq)
              solut.seq[j_next][info.dimen].W * cost_concat_2 + solut.seq[j_next][info.dimen].C;      // concat 3rd subseq

            
            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reverse(solut.s, I, J);
        subseq_load(solut, info);
        return true;
    }

    return false;
}

bool search_reinsertion(tSolution & solut, const tInfo & info, const int opt) {
    alignas(DBL_SZ) double cost_new, cost_concat_1, cost_concat_2, cost_concat_3;
    alignas(DBL_SZ) double cost_best = DBL_MAX;//, cost_l1, cost_l2, cost_l3;
    alignas(INT_SZ) int i, j, k, k_next, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;
    alignas(INT_SZ) int POS;

    for (i = 1, j = opt +i-1; i < info.dimen-opt+1; ++i, ++j) {
        j_next = j + 1;
        i_prev = i - 1;

        //k -> edges 
        for(k = 0; k < i_prev; ++k){
            k_next = k+1;

          cost_concat_1 =                 solut.seq[0][k].T            + info.cost[solut.s[k]][solut.s[i]];
          cost_concat_2 = cost_concat_1 + solut.seq[i][j].T            + info.cost[solut.s[j]][solut.s[k_next]];
          cost_concat_3 = cost_concat_2 + solut.seq[k_next][i_prev].T  + info.cost[solut.s[i_prev]][solut.s[j_next]];

          cost_new = solut.seq[0][k].C                                                                   +   //       1st subseq
              solut.seq[i][j].W               * cost_concat_1 + solut.seq[i][j].C                  +   //  concat 2nd subseq (reinserted seq)
              solut.seq[k_next][i_prev].W     * cost_concat_2 + solut.seq[k_next][i_prev].C        +   //  concat 3rd subseq
              solut.seq[j_next][info.dimen].W * cost_concat_3 + solut.seq[j_next][info.dimen].C;       // concat 4th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }

        for (k = i + opt; k < info.dimen; ++k) {
            k_next = k + 1;

          cost_concat_1 =                 solut.seq[0][i_prev].T  + info.cost[solut.s[i_prev]][solut.s[j_next]];
          cost_concat_2 = cost_concat_1 + solut.seq[j_next][k].T  + info.cost[solut.s[k]][solut.s[i]];
          cost_concat_3 = cost_concat_2 + solut.seq[i][j].T       + info.cost[solut.s[j]][solut.s[k_next]];

          cost_new = solut.seq[0][i_prev].C                                                                  +   //       1st subseq
                  solut.seq[j_next][k].W          * cost_concat_1 + solut.seq[j_next][k].C             +   // concat 2nd subseq
                  solut.seq[i][j].W               * cost_concat_2 + solut.seq[i][j].C                  +   // concat 3rd subseq (reinserted seq)
                  solut.seq[k_next][info.dimen].W * cost_concat_3 + solut.seq[k_next][info.dimen].C;       // concat 4th subseq
          
            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reinsert(solut.s, I, J, POS+1);
        subseq_load(solut, info, I < POS+1 ? I : POS+1);
        return true;
    }

    return false;
}

#endif

void RVND(tSolution & solut, tInfo & info) {

    alignas(alignof(std::vector<int>)) std::vector<int> neighbd_list = {SWAP, TWO_OPT, REINSERTION, OR_OPT_2, OR_OPT_3};
    alignas(INT_SZ) uint index;
    alignas(INT_SZ) int neighbd;
    //int k = 0;
    bool improve_flag;

    //cout << "RVND" << endl;
    while (!neighbd_list.empty()) {
        //k++;

        index = info.rnd[info.rnd_index++];
        neighbd = neighbd_list[index];
        //std::cout <<"aq\n";
        improve_flag = false;

        switch(neighbd){
            case REINSERTION:
                //before();
                improve_flag = search_reinsertion(solut, info, REINSERTION);
                //after(REINSERTION);
                break;				
            case OR_OPT_2:
                //before();
                improve_flag = search_reinsertion(solut, info, OR_OPT_2);
                //after(OR_OPT2);
                break;				
            case OR_OPT_3:
                //before();
                improve_flag = search_reinsertion(solut, info, OR_OPT_3);
                //after(OR_OPT3);
                break;				
            case SWAP:
                //before();
                improve_flag = search_swap(solut, info);
                //after(SWAP);
                break;
            case TWO_OPT:
                //before();
                improve_flag = search_two_opt(solut, info);
                //after(TWO_OPT);
                break;				
        }
        //std::cout << (improve_flag ? "True" : "False") << std::endl;
        if (improve_flag) {
            neighbd_list = {SWAP, TWO_OPT, REINSERTION, OR_OPT_2, OR_OPT_3};
        } else {
            //std::cout << index << "  " << neighbd_list.size() << std::endl;
            //std::cout << solut.cost << std::endl;
            
            //std::cout << info.rnd_index << std::endl;
            neighbd_list.erase(neighbd_list.begin() + index);
        }

        //std::cout << "cost  " << solut.cost << std::endl ;


    }

    //exit(0);
    //std::cout << k << " RVND iteracoes" << std::endl;
}

std::vector<int> perturb(tSolution * solut, tInfo & info) {
    auto s = solut->s;
    int A_start = 1;
    int A_end = 1;
    int B_start = 1;
    int B_end = 1;

    int size_max = std::floor((info.dimen+1)/10);
    size_max = size_max >= 2 ? size_max : 2;
    int size_min = 2;
    //std::cout << "perturbing\n";
    //print_s(s);
    while ((A_start <= B_start && B_start <= A_end) || (B_start <= A_start && A_start <= B_end)) {

        A_start = info.rnd[info.rnd_index++];
        A_end = A_start + info.rnd[info.rnd_index++];

        B_start = info.rnd[info.rnd_index++];
        B_end = B_start + info.rnd[info.rnd_index++];
    }
    
    if (A_start < B_start) {
        reinsert(s, B_start, B_end-1, A_end);
        reinsert(s, A_start, A_end-1, B_end);
    } else {
        reinsert(s, A_start, A_end-1, B_end);
        reinsert(s, B_start, B_end-1, A_end);
    }

    return s;
}


void GILS_RVND(int Imax, int Iils, tInfo & info) {

    tSolution solut_partial = Solution_init(info);
    tSolution solut_crnt = Solution_init(info);
    tSolution solut_best = Solution_init(info);

    for(int i = 0; i < Imax; ++i){
        int aux = info.rnd[info.rnd_index++];

        double alpha = R_table(aux);

        printf("[+] Search %d\n", i+1);
        printf("\t[+] Constructing..\n");	


        solut_crnt.s = construct(alpha, info);
        subseq_load(solut_crnt, info);

        Solution_cpy(solut_crnt, solut_partial, info);
        printf("\t[+] Looking for the best Neighbor..\n");
        printf("\t    Construction Cost: %.3lf\n", solut_partial.cost);	

        int iterILS = 0;
        while (iterILS < Iils) {
            RVND(solut_crnt, info);
            if(solut_crnt.cost < solut_partial.cost - DBL_EPSILON){
                Solution_cpy(solut_crnt, solut_partial, info);
                iterILS = 0;
            }

            solut_crnt.s = perturb(&solut_partial, info);
            subseq_load(solut_crnt, info);
            iterILS++;
        }

        if (solut_partial.cost < solut_best.cost - DBL_EPSILON) {
            Solution_cpy(solut_partial, solut_best, info);
            //solut_best = solut_partial;
        }

        //after(7);

        //std::cout << "\tCurrent search cost: "<< cost_sl << std::endl;
        std::cout << "\tCurrent best cost: "<< solut_best.cost << std::endl;
        //std::cout << "\tCurrent search time: "<< search_t / 10e5<< std::endl;
        //std::cout << "\tCurrent search time average: "<< (search_t_average / (i+1)) / 10e5 << std::endl;
        //std::cout << k << "  Iteracoes " << std::endl;

        std::cout << "SOLUCAO: ";
        for(int i = 0; i < solut_best.s.size(); i++){
            std::cout << solut_best.s[i] << " ";
        }
        std::cout << std::endl;

    }
    //std::cout << "Dimension: " << dimension << std::endl;
    printf("COST: %.2lf\n", solut_best.cost);
}

int main(int argc, char **argv){
    int Imax = 10;
    int Iils;

    tInfo info = {
    };


    std::vector<int> rnd;

    info.dimen = loadData(&info.cost, rnd);
    info.rnd = rnd;
    //print_s(rnd);
    info.rnd_index = 0;

    tSolution solut = Solution_init(info);

    /*
    for (int i = 0; i <=n; i++) {
        for (int j = i+1; j <=n; j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
    */


    srand(clock());

    Iils = info.dimen < 100 ? info.dimen : 100;
    auto t1 = steady_clock::now();
    size_t start = clock();
    GILS_RVND(Imax, Iils, info);
    double cpu_time = (double)(clock() - start) / CLOCKS_PER_SEC ;
    auto t2 = steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    double res = (double)duration / 10e2;
    std::cout << "TIME: " << res << std::endl;
    std::cout << "time clock(): " << cpu_time << std::endl;

    std::cout << "Tamanho RND " << rnd.size() << std::endl;

    exit(0);
    /*
    flag = true;

    srand(duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
    readData(argc, argv, &dimension, &c);

    int ar[] = {100, dimension};
    
    Iils = ar[dimension < 100];

    auto t1 = high_resolution_clock::now();

    GILS_RVND(Imax, Iils);

    auto t2 = high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    double res = (double)duration / 10e2;
    std::cout << "TIME: " << res << std::endl;

    if(flag){
        std::cout << "Construction time: " << construct_t/10e5 << std::endl;
        std::cout << "Swap time: " << swap_t/10e5 << std::endl;
        std::cout << "two_opt time: " << two_opt_t/10e5 << std::endl;
        std::cout << "reinsertion time: " << reinsertion_t/10e5 << std::endl;
        std::cout << "or_opt2 time: " << opt2_t/10e5 << std::endl;
        std::cout << "or_opt3 time: " << opt3_t /10e5<< std::endl;
    }
    */
    return 0;
}

