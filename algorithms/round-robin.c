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

    // Taking time quantum
    // If user just presses Enter, default time quantum will be 1
    char tq_input[20];
    int tq;

    printf("Enter time quantum (press Enter for default 1): ");
    fgets(tq_input, sizeof(tq_input), stdin);

    if (tq_input[0] == '\n') {
        tq = 1;
    } else {
        tq = atoi(tq_input);

        if (tq <= 0) {
            tq = 1;
        }
    }

    printf("Time Quantum Used: %d\n", tq);

    int at[n], bt[n], remaining_bt[n], ct[n], wt[n], tat[n], rt[n];
    int completed[n], in_queue[n];
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

        remaining_bt[i] = bt[i];
        completed[i] = 0;
        in_queue[i] = 0;

        // -1 means process did not get CPU yet
        rt[i] = -1;
    }

    // Sorting by Arrival Time first
    // If Arrival Time is same, smaller Process ID will come first
    // This is important for Round Robin ready queue order
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

                temp = remaining_bt[j];
                remaining_bt[j] = remaining_bt[j + 1];
                remaining_bt[j + 1] = temp;

                temp = completed[j];
                completed[j] = completed[j + 1];
                completed[j + 1] = temp;

                temp = in_queue[j];
                in_queue[j] = in_queue[j + 1];
                in_queue[j + 1] = temp;

                temp = rt[j];
                rt[j] = rt[j + 1];
                rt[j + 1] = temp;

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

    // Queue is used because Round Robin = FCFS + Time Quantum
    int queue[1000];
    int front = 0, rear = 0;

    int order[1000], order_index = 0;
    int current_time = 0, completed_count = 0;

    int total_wt = 0, total_tat = 0, total_rt = 0;

    // These arrays are used for Ready Queue and Running Queue charts
    char running_chart[1000][20];
    char ready_chart[1000][100];
    int chart_time[1000];
    int chart_index = 0;

    while (completed_count < n) {

        // First CPU checks which process has arrived in ready queue
        // Because already sorted by AT and PID, same AT processes come in proper order
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 &&
                in_queue[i] == 0 &&
                at[i] <= current_time &&
                remaining_bt[i] > 0) {

                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        // If no process is available, CPU will stay idle
        if (front == rear) {
            strcpy(ready_chart[chart_index], "X");
            strcpy(running_chart[chart_index], "X");
            chart_time[chart_index] = current_time;
            chart_index++;

            current_time++;
            continue;
        }

        // Store Ready Queue before process goes to Running Queue
        strcpy(ready_chart[chart_index], "");

        for (int i = front; i < rear; i++) {
            strcat(ready_chart[chart_index], id[queue[i]]);

            if (i != rear - 1) {
                strcat(ready_chart[chart_index], ",");
            }
        }

        // Select first process from ready queue
        int selected = queue[front++];
        in_queue[selected] = 0;

        // Store selected process in Running Queue
        strcpy(running_chart[chart_index], id[selected]);
        chart_time[chart_index] = current_time;
        chart_index++;

        // Response Time is calculated only first time when process gets CPU
        if (rt[selected] == -1) {
            rt[selected] = current_time - at[selected];
        }

        // Process executes for time quantum or remaining burst time
        int execute_time;

        if (remaining_bt[selected] > tq) {
            execute_time = tq;
        } else {
            execute_time = remaining_bt[selected];
        }

        current_time += execute_time;
        remaining_bt[selected] -= execute_time;

        // After time slice, check newly arrived processes
        // They enter ready queue before selected process goes back again
        for (int i = 0; i < n; i++) {
            if (i != selected &&
                completed[i] == 0 &&
                in_queue[i] == 0 &&
                at[i] <= current_time &&
                remaining_bt[i] > 0) {

                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        // If selected process is not completed, send it back to ready queue
        if (remaining_bt[selected] > 0) {
            queue[rear++] = selected;
            in_queue[selected] = 1;
        } else {
            // If process is completed, calculate final values
            completed[selected] = 1;
            completed_count++;

            ct[selected] = current_time;
            tat[selected] = ct[selected] - at[selected];
            wt[selected] = tat[selected] - bt[selected];

            total_wt += wt[selected];
            total_tat += tat[selected];
            total_rt += rt[selected];

            // This stores completion order
            order[order_index++] = selected;
        }
    }

    // Display execution summary
    printf("\n\n=== Round Robin Execution Summary ===\n");
    printf("\n====================================================================================\n");
    printf("%-10s %-25s %-5s %-5s %-5s %-5s %-5s %-5s\n",
           "Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT");
    printf("====================================================================================\n");

    for (int i = 0; i < order_index; i++) {
        int p = order[i];

        printf("%-10s %-25s %-5d %-5d %-5d %-5d %-5d %-5d\n",
               id[p], type[p], at[p], bt[p], ct[p], wt[p], tat[p], rt[p]);
    }

    // Display averages
    printf("\nAverage Waiting Time: %.2f", (float)total_wt / n);
    printf("\nAverage Turn Around Time: %.2f", (float)total_tat / n);
    printf("\nAverage Response Time: %.2f\n", (float)total_rt / n);

    // Ready Queue Chart first
    printf("\nReady Queue Chart:\n");

    for (int i = 0; i < chart_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < chart_index; i++) {
        printf("| %-10s ", ready_chart[i]);
    }
    printf("|\n");

    for (int i = 0; i < chart_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < chart_index; i++) {
        printf("%-13d", chart_time[i]);
    }
    printf("%d\n", current_time);

    // Running Queue / Gantt Chart
    printf("\nRunning Queue / Gantt Chart:\n");

    for (int i = 0; i < chart_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < chart_index; i++) {
        printf("| %-10s ", running_chart[i]);
    }
    printf("|\n");

    for (int i = 0; i < chart_index; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (int i = 0; i < chart_index; i++) {
        printf("%-13d", chart_time[i]);
    }
    printf("%d\n", current_time);

    return 0;
}
