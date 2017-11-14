//
//  main.cpp
//  parallel_sum
//
//  Created by 星宇  韦 on 14/11/2017.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>

using namespace std;
const int NUM_THREAD = 3;

// the following code is used from
// https://gist.github.com/jbenet/1087739

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}


double sum(int *a, int start, int end) {
    double ans = 0;
    int i, j;
    for(i = start; i < end; i += NUM_THREAD) {
        for(j = 1; j <= a[i]; j++)
            ans += j;
    }

    return ans;
}

void parallel_sum(int *a, int start, int end) {
    double *g;
    int i;
    g = (double *)malloc(sizeof(double ) * (NUM_THREAD + 3));
    
    dispatch_queue_t concurrent_q = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    
    // we use group to group all summation operations.
    dispatch_group_t group = dispatch_group_create();
    double ans = 0;
    double wall_clock_start = get_wall_time();
    double cpu_clock_start = get_cpu_time();
    
    for (i=0; i < NUM_THREAD; i++) {
        dispatch_group_async(group, concurrent_q, ^{
            g[i] = sum(a, i, end);
        });
    }
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    for (i=0; i<NUM_THREAD; i++)
        ans += g[i];
    double wall_elapsed_time = get_wall_time() - wall_clock_start;
    double cpu_elapsed_time = get_cpu_time() - cpu_clock_start;

    printf("the sum is %lf\n", ans);
    printf("Elapsed time for parallel: cpu: %lf, wall: %lf\n", cpu_elapsed_time, wall_elapsed_time);
    
    // delete all dispatch group
    dispatch_release(group);
    dispatch_release(concurrent_q);
    free(g);
}

int main(int argc, const char * argv[]) {
    int *a, i;
    
    printf("argc: %d\n", argc);
    
    if (argc != 3)
        exit(-1);
    
    
    int start = atoi(argv[1]);
    int end = atoi(argv[2]);
    
    
    a = (int *)malloc(sizeof(int) * (end - start + 10));
    for (i = start; i <= end; i++)
        a[i - start] = i;
    parallel_sum(a, 0, end - start + 1);

    double correct_ans = 0.0;
    int cnt = end - start + 1;
    int j;
    double wall_clock_start = get_wall_time();
    double cpu_clock_start = get_cpu_time();
    for (i = 0; i < cnt; i++)
        for(j = 1; j <= a[i]; j++)
            correct_ans += j;
    
    double wall_elapsed_time = get_wall_time() - wall_clock_start;
    double cpu_elapsed_time = get_cpu_time() - cpu_clock_start;
    
    
    printf("The correct sum is %lf\n", correct_ans);
    printf("Elapsed time for parallel: cpu: %lf, wall: %lf\n", cpu_elapsed_time, wall_elapsed_time);
    free(a);
    return 0;
}
