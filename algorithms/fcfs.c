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
    }

    // FCFS sorting
    // First check Arrival Time
    // If Arrival Time is same, smaller Process ID will execute first
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (at[j] > at[j + 1] ||
               (at[j] == at[j + 1] &&
                getProcessNumber(id[j]) > getProcessNumber(id[j + 1]))) {

                int temp;

                temp = at[j];
                at[j] = at[j + 1];
                at[j + 1] = temp;

                temp = bt[j];
                bt[j] = bt[j + 1];
                bt[j + 1] = temp;

                char temp_str[30];

                strcpy(temp_str, id[j]);
                strcpy(id[j], id[j + 1]);
                strcpy(id[j + 1], temp_str);

                strcpy(temp_str, type[j]);
                strcpy(type[j], type[j + 1]);
                strcpy(type[j + 1], temp_str);
            }
        }
    }

    int current_time = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;

    char gantt[100][5];
    int gantt_time[100];
    int gantt_index = 0;

    // Calculation part
    for (int i = 0; i < n; i++) {

        // If CPU is idle before next process arrives
        if (current_time < at[i]) {
            strcpy(gantt[gantt_index], "X");
            gantt_time[gantt_index] = current_time;
            gantt_index++;

            current_time = at[i];
        }

        // Store process in Gantt chart
        strcpy(gantt[gantt_index], id[i]);
        gantt_time[gantt_index] = current_time;
        gantt_index++;

        rt[i] = current_time - at[i];

        current_time += bt[i];
        ct[i] = current_time;

        tat[i] = ct[i] - at[i];
        wt[i] = tat[i] - bt[i];

        total_wt += wt[i];
        total_tat += tat[i];
        total_rt += rt[i];
    }

    // Display execution summary
    printf("\n\n=== FCFS Execution Summary ===\n");
    printf("\n====================================================================================\n");
    printf("%-10s %-25s %-5s %-5s %-5s %-5s %-5s %-5s\n",
           "Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT");
    printf("====================================================================================\n");

    for (int i = 0; i < n; i++) {
        printf("%-10s %-25s %-5d %-5d %-5d %-5d %-5d %-5d\n",
               id[i], type[i], at[i], bt[i], ct[i], wt[i], tat[i], rt[i]);
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
