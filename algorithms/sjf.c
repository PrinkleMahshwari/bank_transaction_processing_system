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

    int at[n], bt[n], ct[n], wt[n], tat[n], rt[n];
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

        while(getchar() != '\n');

        completed[i] = 0;
    }

    int current_time = 0, completed_count = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;

    char gantt[100][5];
    int gantt_time[100];
    int gantt_index = 0;

    while (completed_count < n) {
        int shortest = -1;
        int min_bt = 9999;

        // First CPU checks which processes have arrived in ready queue
        // Then SJF selects process with smallest burst time
        // If burst time is same, smaller Process ID will execute first
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 && at[i] <= current_time) {

                if (shortest == -1 ||
                    bt[i] < min_bt ||
                    (bt[i] == min_bt &&
                     getProcessNumber(id[i]) < getProcessNumber(id[shortest]))) {

                    min_bt = bt[i];
                    shortest = i;
                }
            }
        }

        // If no process has arrived, CPU will stay idle
        if (shortest == -1) {
            if (gantt_index == 0 || strcmp(gantt[gantt_index - 1], "X") != 0) {
                strcpy(gantt[gantt_index], "X");
                gantt_time[gantt_index] = current_time;
                gantt_index++;
            }

            current_time++;
        } else {
            // Selected process will execute completely because SJF is non-preemptive
            strcpy(gantt[gantt_index], id[shortest]);
            gantt_time[gantt_index] = current_time;
            gantt_index++;

            rt[shortest] = current_time - at[shortest];

            current_time += bt[shortest];
            ct[shortest] = current_time;

            tat[shortest] = ct[shortest] - at[shortest];
            wt[shortest] = tat[shortest] - bt[shortest];

            total_wt += wt[shortest];
            total_tat += tat[shortest];
            total_rt += rt[shortest];

            completed[shortest] = 1;
            order[completed_count] = shortest;
            completed_count++;
        }
    }

    // Display execution summary
    printf("\n\n=== SJF Execution Summary ===\n");
    printf("\n====================================================================================\n");
    printf("%-10s %-25s %-5s %-5s %-5s %-5s %-5s %-5s\n",
           "Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT");
    printf("====================================================================================\n");

    for (int i = 0; i < n; i++) {
        int p = order[i];

        printf("%-10s %-25s %-5d %-5d %-5d %-5d %-5d %-5d\n",
               id[p], type[p], at[p], bt[p], ct[p], wt[p], tat[p], rt[p]);
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
