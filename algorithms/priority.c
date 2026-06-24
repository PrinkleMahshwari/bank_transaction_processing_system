#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// This function extracts number from process ID
// Example: P1 = 1, P2 = 2, P10 = 10
int getProcessNumber(char id[]) {
    int num = 0;

    for (int i = 1; id[i] != '\0'; i++) {
        num = num * 10 + (id[i] - '0');
    }

    return num;
}

int main() {
    printf("\t\t===Bank Transaction Processing System===\n\n");

    int n;
    printf("Enter number of transactions: ");
    scanf("%d", &n);
    while(getchar() != '\n');

    int at[n], bt[n], ct[n], wt[n], tat[n], rt[n], priority[n];
    int completed[n], order[n];

    char id[n][5], type[n][30];

    // Taking dynamic input from user
    for (int i = 0; i < n; i++) {
        printf("\n--- Transaction %d ---\n", i + 1);

        printf("Enter transaction ID: ");
        fgets(id[i], sizeof(id[i]), stdin);
        id[i][strcspn(id[i], "\n")] = '\0';

        printf("Enter transaction type: ");
        fgets(type[i], sizeof(type[i]), stdin);
        type[i][strcspn(type[i], "\n")] = '\0';

        printf("Enter arrival time: ");
        scanf("%d", &at[i]);

        printf("Enter burst time: ");
        scanf("%d", &bt[i]);

        printf("Enter priority: ");
        scanf("%d", &priority[i]);

        while(getchar() != '\n');

        completed[i] = 0;
    }

    int current_time = 0, completed_count = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;

    char gantt[100][5];
    int gantt_time[100];
    int gantt_index = 0;

    while (completed_count < n) {
        int selected = -1;
        int highest_priority = -1;

        // First CPU checks which processes have arrived in ready queue
        // Then Priority Scheduling selects highest priority process
        // Condition: higher number means higher priority
        // If priority is same, smaller Process ID will execute first
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 && at[i] <= current_time) {

                if (selected == -1 ||
                    priority[i] > highest_priority ||
                    (priority[i] == highest_priority &&
                     getProcessNumber(id[i]) < getProcessNumber(id[selected]))) {

                    highest_priority = priority[i];
                    selected = i;
                }
            }
        }

        // If no process has arrived, CPU will stay idle
        if (selected == -1) {
            if (gantt_index == 0 || strcmp(gantt[gantt_index - 1], "X") != 0) {
                strcpy(gantt[gantt_index], "X");
                gantt_time[gantt_index] = current_time;
                gantt_index++;
            }

            current_time++;
        } else {
            // Selected process will execute completely because Priority is non-preemptive
            strcpy(gantt[gantt_index], id[selected]);
            gantt_time[gantt_index] = current_time;
            gantt_index++;

            rt[selected] = current_time - at[selected];

            current_time += bt[selected];
            ct[selected] = current_time;

            tat[selected] = ct[selected] - at[selected];
            wt[selected] = tat[selected] - bt[selected];

            total_wt += wt[selected];
            total_tat += tat[selected];
            total_rt += rt[selected];

            completed[selected] = 1;
            order[completed_count] = selected;
            completed_count++;
        }
    }

    // Display execution summary
    printf("\n\n=== Priority Scheduling Execution Summary ===\n");
    printf("\n==================================================================================================\n");
    printf("%-8s %-10s %-25s %-5s %-5s %-5s %-5s %-5s %-5s\n",
           "Priority", "Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT");
    printf("==================================================================================================\n");

    for (int i = 0; i < n; i++) {
        int p = order[i];

        printf("%-8d %-10s %-25s %-5d %-5d %-5d %-5d %-5d %-5d\n",
               priority[p], id[p], type[p], at[p], bt[p], ct[p], wt[p], tat[p], rt[p]);
    }

    printf("\nAverage Waiting Time: %.2f", (float)total_wt / n);
    printf("\nAverage Turn Around Time: %.2f", (float)total_tat / n);
    printf("\nAverage Response Time: %.2f\n", (float)total_rt / n);

    // Gantt Chart
    printf("\nGantt Chart:\n");

    for (int i = 0; i < gantt_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < gantt_index; i++) {
        printf("| %-10s ", gantt[i]);
    }
    printf("|\n");

    for (int i = 0; i < gantt_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < gantt_index; i++) {
        printf("%-13d", gantt_time[i]);
    }
    printf("%d\n", current_time);

    return 0;
}
