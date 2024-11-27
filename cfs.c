#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#define NICE_OFFSET 20 // default nice value is 0
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct Process
{
    int pid;
    char name[100];
    int priority;
    int arrive_time;
    int burst_time;
    int vruntime;
} Process;

const int sched_nice_to_weight[40] = //-20 ~ 19 weight = 1024 * 1.25 ^ (-nice)
{
    88817, 71054, 56843, 45474, 36379,
    29103, 23283, 18626, 14901, 11920,
    9536, 7629, 6103, 4882, 3906,
    3125, 2500, 2000, 1600, 1280,
    1024, 819, 655, 524, 419,
    335, 268, 214, 171, 137,
    109, 87, 70, 56, 45,
    36, 28, 23, 18, 14
};

void schedule(Process proc[], int n)
{
    int current_cpu_time = 0;
    const int PRIO_0_WEIGHT = 1024;
    int min_vruntime = 0;
    int sysctl_sched_latency = 6000000;
    int total_weight = 0;
    int pick;
    int min_notarrive_time;
    while (1)
    {
        total_weight = 0;
        min_notarrive_time = INT_MAX;
        for (int i = 0; i < n; i++)
        {
            if (current_cpu_time >= proc[i].arrive_time)
                total_weight += sched_nice_to_weight[proc[i].priority];
            else
                min_notarrive_time = min(min_notarrive_time, proc[i].arrive_time);
        }
        if (total_weight == 0)
        {
            current_cpu_time = min_notarrive_time;
            continue;
        }
        pick = -1;
        for (int i = 0; i < n; i++)
        {
            if (proc[i].burst_time == 0 || current_cpu_time < proc[i].arrive_time)
                continue;
            if (pick == -1 || proc[pick].vruntime > proc[i].vruntime)
                pick = i;
        }
        if (pick == -1)
            break;
        int pick_weight = sched_nice_to_weight[proc[pick].priority];
        int calc_time = (int)sysctl_sched_latency * (1.0 * pick_weight / total_weight);
        int time_slice = min(proc[pick].burst_time, calc_time);
        printf("process %d name[%s] burst %d ns\n", proc[pick].pid, proc[pick].name, time_slice);
        proc[pick].burst_time -= time_slice;
        proc[pick].vruntime += (int)time_slice * (1.0 * PRIO_0_WEIGHT / pick_weight);
        min_vruntime = INT_MAX;
        for (int i = 0; i < n; i++)
        {
            if (proc[i].burst_time > 0 && proc[i].arrive_time <= current_cpu_time)
                min_vruntime = min(min_vruntime, proc[i].vruntime);
        }
        if (min_vruntime == INT_MAX)
            min_vruntime = 0;
        for (int i = 0; i < n; i++)
        {
            if (proc[i].arrive_time > current_cpu_time && proc[i].arrive_time <= current_cpu_time + time_slice)
                proc[i].vruntime = min_vruntime;
        }
        current_cpu_time += time_slice;
    }
}

int main()
{
    Process proc[] =
    {
        {
            0,
            "init",
            NICE_OFFSET + 0,
            0,
            10000000,
            0,
        },
        {
            1,
            "bash",
            NICE_OFFSET + 1,
            0,
            20000000,
            0,
        },
        {
            2,
            "vim",
            NICE_OFFSET + 5,
            100,
            10000000,
            0,
        }
    };
    schedule(proc, 3);
    return 0;
}
