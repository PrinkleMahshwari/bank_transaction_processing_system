#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_TX 100

// Transaction structure to hold all fields
typedef struct {
    char id[10];
    char type[30];
    int at, bt, priority;
} Transaction;

// Global transaction array and count
static Transaction tx[MAX_TX];
static int tx_count = 0;

// Input widgets
static GtkWidget *entry_id, *entry_type, *entry_at, *entry_bt;
static GtkWidget *entry_priority, *entry_quantum;
static GtkWidget *status_label;

// Transaction list store and view shown below the input area
static GtkListStore *tx_store;
static GtkWidget   *tx_tree;

// Output area container inside scroll window
static GtkWidget *output_container;

// This function extracts number from process ID
// Example: P1 = 1, P2 = 2, P10 = 10
static int getProcessNumber(const char *id) {
    int num = 0;
    for (int i = 1; id[i] != '\0'; i++)
        num = num * 10 + (id[i] - '0');
    return num;
}

// Shows a short status message below the button row
static void set_status(const char *text) {
    gtk_label_set_text(GTK_LABEL(status_label), text);
}

// Resets all input fields after a transaction is added
static void clear_entries(void) {
    gtk_entry_set_text(GTK_ENTRY(entry_id),       "");
    gtk_entry_set_text(GTK_ENTRY(entry_type),     "");
    gtk_entry_set_text(GTK_ENTRY(entry_at),       "");
    gtk_entry_set_text(GTK_ENTRY(entry_bt),       "");
    gtk_entry_set_text(GTK_ENTRY(entry_priority), "");
}

// Rebuilds the transaction list store from the tx[] array
// Called after every add or remove operation
static void refresh_tx_table(void) {
    gtk_list_store_clear(tx_store);

    if (tx_count == 0) {
        GtkTreeIter iter;
        gtk_list_store_append(tx_store, &iter);
        gtk_list_store_set(tx_store, &iter,
            0, "", 1, "No transactions added yet", 2, "", 3, "", 4, "", 5, "", -1);
        return;
    }

    for (int i = 0; i < tx_count; i++) {
        char num[8], at_s[8], bt_s[8], pr_s[8];
        snprintf(num,  sizeof(num),  "%d",  i + 1);
        snprintf(at_s, sizeof(at_s), "%d",  tx[i].at);
        snprintf(bt_s, sizeof(bt_s), "%d",  tx[i].bt);
        snprintf(pr_s, sizeof(pr_s), "%d",  tx[i].priority);

        GtkTreeIter iter;
        gtk_list_store_append(tx_store, &iter);
        gtk_list_store_set(tx_store, &iter,
            0, num,
            1, tx[i].id,
            2, tx[i].type,
            3, at_s,
            4, bt_s,
            5, pr_s,
            -1);
    }
}

// Clears all previous output widgets before showing new result
static void clear_output(void) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(output_container));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);
}

// ---------- Button Callbacks ----------

// Called when Add Transaction button is clicked
// Reads entry boxes and appends to tx[] array then refreshes the table
static void on_add_transaction(GtkWidget *widget, gpointer data) {
    if (tx_count >= MAX_TX) {
        set_status("Transaction limit reached.");
        return;
    }

    // Reading values from entry boxes
    const char *id   = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const char *type = gtk_entry_get_text(GTK_ENTRY(entry_type));
    const char *at_s = gtk_entry_get_text(GTK_ENTRY(entry_at));
    const char *bt_s = gtk_entry_get_text(GTK_ENTRY(entry_bt));
    const char *pr_s = gtk_entry_get_text(GTK_ENTRY(entry_priority));

    // Basic validation, ID/AT/BT are compulsory fields
    if (strlen(id) == 0 || strlen(at_s) == 0 || strlen(bt_s) == 0) {
        set_status("Please fill Transaction ID, Arrival Time and Burst Time.");
        return;
    }

    // Storing transaction in array
    Transaction *t = &tx[tx_count];
    strncpy(t->id,   id,                        sizeof(t->id)   - 1);
    strncpy(t->type, strlen(type) ? type : "N/A", sizeof(t->type) - 1);
    t->at       = atoi(at_s);
    t->bt       = atoi(bt_s);
    t->priority = strlen(pr_s) ? atoi(pr_s) : 0;
    tx_count++;

    refresh_tx_table();
    clear_entries();

    char msg[160];
    snprintf(msg, sizeof(msg),
        "Added: %s | %s | AT=%d  BT=%d  Priority=%d  [%d transaction(s) total]",
        t->id, t->type, t->at, t->bt, t->priority, tx_count);
    set_status(msg);
}

// Called when Clear button is clicked
// Removes last added transaction from tx[] array
static void on_clear_last(GtkWidget *widget, gpointer data) {
    if (tx_count == 0) {
        set_status("No transactions to clear.");
        return;
    }
    tx_count--;
    refresh_tx_table();

    char msg[80];
    snprintf(msg, sizeof(msg), "Last transaction removed. [%d transaction(s) remaining]", tx_count);
    set_status(msg);
}

// Called when Clear All button is clicked
// Removes all transactions from tx[] array
static void on_clear_all(GtkWidget *widget, gpointer data) {
    tx_count = 0;
    refresh_tx_table();
    clear_output();
    set_status("All transactions cleared.");
}

// ---------- UI Builder Helpers ----------

// Builds the algorithm name heading shown above the result table
static GtkWidget *build_heading(const char *algo_name) {
    char buf[80];
    snprintf(buf, sizeof(buf), "%s Execution Summary", algo_name);
    GtkWidget *label = gtk_label_new(buf);
    gtk_widget_set_name(label, "result-heading");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}

// Builds the execution summary table using GtkTreeView
// headers = column names, rows = data, n_cols/n_rows = dimensions
static GtkWidget *build_table(const char *headers[], int n_cols,
                               char rows[][9][32], int n_rows) {
    GType *types = g_new(GType, n_cols);
    for (int i = 0; i < n_cols; i++) types[i] = G_TYPE_STRING;

    GtkListStore *store = gtk_list_store_newv(n_cols, types);
    for (int r = 0; r < n_rows; r++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        for (int c = 0; c < n_cols; c++)
            gtk_list_store_set(store, &iter, c, rows[r][c], -1);
    }

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_set_name(tree, "result-table");
    g_object_unref(store);

    for (int c = 0; c < n_cols; c++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, "xpad", 10, "ypad", 7, NULL);
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
            headers[c], renderer, "text", c, NULL);
        gtk_tree_view_column_set_expand(col, TRUE);
        gtk_tree_view_column_set_alignment(col, 0.5);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
    }
    g_free(types);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_name(frame, "table-frame");
    gtk_container_add(GTK_CONTAINER(frame), tree);
    return frame;
}

// Builds the Gantt chart as colored visual blocks
// Each block shows the process ID and its time range
// X blocks represent idle CPU time
static GtkWidget *build_gantt(const char *title, char gantt[][100],
                               int gtime[], int gindex, int end_time) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    GtkWidget *label = gtk_label_new(title);
    gtk_widget_set_name(label, "subsection-label");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 70);

    GtkWidget *strip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_name(strip, "gantt-strip");
    gtk_container_set_border_width(GTK_CONTAINER(strip), 8);

    for (int i = 0; i < gindex; i++) {
        int seg_end = (i + 1 < gindex) ? gtime[i + 1] : end_time;

        GtkWidget *block = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        gtk_widget_set_name(block,
            strcmp(gantt[i], "X") == 0 ? "gantt-idle" : "gantt-block");

        GtkWidget *id_label = gtk_label_new(gantt[i]);
        gtk_widget_set_name(id_label, "gantt-id");

        char time_text[32];
        snprintf(time_text, sizeof(time_text), "%d-%d", gtime[i], seg_end);
        GtkWidget *time_label = gtk_label_new(time_text);
        gtk_widget_set_name(time_label, "gantt-time");

        gtk_box_pack_start(GTK_BOX(block), id_label,   FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(block), time_label, FALSE, FALSE, 0);
        gtk_widget_set_size_request(block, 70, 54);
        gtk_container_set_border_width(GTK_CONTAINER(block), 6);

        gtk_box_pack_start(GTK_BOX(strip), block, FALSE, FALSE, 0);
    }

    gtk_container_add(GTK_CONTAINER(scroll), strip);
    gtk_box_pack_start(GTK_BOX(box), scroll, FALSE, FALSE, 0);
    return box;
}

// Builds the three pastel stat cards showing Average WT, TAT, RT
static GtkWidget *build_avg_cards(float avg_wt, float avg_tat, float avg_rt) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    gtk_box_set_homogeneous(GTK_BOX(row), TRUE);

    // Card data: caption, css id, value
    struct { const char *name; const char *css; float value; } cards[3] = {
        {"Average Waiting Time",    "stat-card-pink",  avg_wt},
        {"Average Turnaround Time", "stat-card-blue",  avg_tat},
        {"Average Response Time",   "stat-card-green", avg_rt},
    };

    for (int i = 0; i < 3; i++) {
        GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_set_name(card, cards[i].css);
        gtk_container_set_border_width(GTK_CONTAINER(card), 20);
        gtk_widget_set_valign(card, GTK_ALIGN_CENTER);

        char val_text[16];
        snprintf(val_text, sizeof(val_text), "%.2f", cards[i].value);

        GtkWidget *val_label = gtk_label_new(val_text);
        gtk_widget_set_name(val_label, "stat-value");

        GtkWidget *cap_label = gtk_label_new(cards[i].name);
        gtk_widget_set_name(cap_label, "stat-caption");
        gtk_label_set_justify(GTK_LABEL(cap_label), GTK_JUSTIFY_CENTER);

        gtk_box_pack_start(GTK_BOX(card), val_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(card), cap_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(row),  card,      TRUE,  TRUE,  0);
    }
    return row;
}

// Assembles all result widgets: heading -> table -> gantt(s) -> stat cards
// gantt2 is only passed for Round Robin which has ready queue + running queue
static void render_result(const char *algo_name,
                           const char *headers[], int n_cols,
                           char rows[][9][32], int n_rows,
                           GtkWidget *gantt1, GtkWidget *gantt2,
                           float avg_wt, float avg_tat, float avg_rt) {
    clear_output();

    gtk_box_pack_start(GTK_BOX(output_container), build_heading(algo_name), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(output_container), build_table(headers, n_cols, rows, n_rows), FALSE, FALSE, 0);

    if (gantt1) gtk_box_pack_start(GTK_BOX(output_container), gantt1, FALSE, FALSE, 0);
    if (gantt2) gtk_box_pack_start(GTK_BOX(output_container), gantt2, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(output_container), build_avg_cards(avg_wt, avg_tat, avg_rt), FALSE, FALSE, 4);

    gtk_widget_show_all(output_container);
}

// ---------- FCFS ----------
// Called when FCFS button is clicked
static void on_fcfs(GtkWidget *widget, gpointer data) {
    if (tx_count == 0) { set_status("No transactions added."); return; }
    int n = tx_count;
    Transaction local[MAX_TX];
    memcpy(local, tx, sizeof(Transaction) * n);

    // FCFS sorting
    // First check Arrival Time
    // If Arrival Time is same, smaller Process ID will execute first
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (local[j].at > local[j+1].at ||
               (local[j].at == local[j+1].at &&
                getProcessNumber(local[j].id) > getProcessNumber(local[j+1].id))) {
                Transaction tmp = local[j]; local[j] = local[j+1]; local[j+1] = tmp;
            }
        }
    }

    int ct[MAX_TX], wt[MAX_TX], tat[MAX_TX], rt[MAX_TX];
    int current_time = 0, total_wt = 0, total_tat = 0, total_rt = 0;
    char gantt[100][100]; int gtime[100], gindex = 0;

    // Calculation part
    for (int i = 0; i < n; i++) {
        // If CPU is idle before next process arrives
        if (current_time < local[i].at) {
            strcpy(gantt[gindex], "X");
            gtime[gindex] = current_time;
            gindex++;
            current_time = local[i].at;
        }

        // Store process in Gantt chart
        strcpy(gantt[gindex], local[i].id);
        gtime[gindex] = current_time;
        gindex++;

        rt[i]  = current_time - local[i].at;
        current_time += local[i].bt;
        ct[i]  = current_time;
        tat[i] = ct[i] - local[i].at;
        wt[i]  = tat[i] - local[i].bt;

        total_wt  += wt[i];
        total_tat += tat[i];
        total_rt  += rt[i];
    }

    // Display execution summary
    const char *headers[] = {"Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT"};
    char rows[MAX_TX][9][32];
    for (int i = 0; i < n; i++) {
        snprintf(rows[i][0], 32, "%s", local[i].id);
        snprintf(rows[i][1], 32, "%s", local[i].type);
        snprintf(rows[i][2], 32, "%d", local[i].at);
        snprintf(rows[i][3], 32, "%d", local[i].bt);
        snprintf(rows[i][4], 32, "%d", ct[i]);
        snprintf(rows[i][5], 32, "%d", wt[i]);
        snprintf(rows[i][6], 32, "%d", tat[i]);
        snprintf(rows[i][7], 32, "%d", rt[i]);
    }

    render_result("FCFS", headers, 8, rows, n,
                  build_gantt("Gantt Chart", gantt, gtime, gindex, current_time), NULL,
                  (float)total_wt/n, (float)total_tat/n, (float)total_rt/n);
}

// ---------- SJF ----------
// Called when SJF button is clicked
static void on_sjf(GtkWidget *widget, gpointer data) {
    if (tx_count == 0) { set_status("No transactions added."); return; }
    int n = tx_count;
    Transaction local[MAX_TX];
    memcpy(local, tx, sizeof(Transaction) * n);

    int completed[MAX_TX] = {0}, order[MAX_TX];
    int ct[MAX_TX], wt[MAX_TX], tat[MAX_TX], rt[MAX_TX];
    int current_time = 0, completed_count = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;
    char gantt[100][100]; int gtime[100], gindex = 0;

    while (completed_count < n) {
        int shortest = -1, min_bt = 999999;

        // First CPU checks which processes have arrived in ready queue
        // Then SJF selects process with smallest burst time
        // If burst time is same, smaller Process ID will execute first
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 && local[i].at <= current_time) {
                if (shortest == -1 ||
                    local[i].bt < min_bt ||
                   (local[i].bt == min_bt &&
                    getProcessNumber(local[i].id) < getProcessNumber(local[shortest].id))) {
                    min_bt = local[i].bt;
                    shortest = i;
                }
            }
        }

        // If no process has arrived, CPU will stay idle
        if (shortest == -1) {
            if (gindex == 0 || strcmp(gantt[gindex-1], "X") != 0) {
                strcpy(gantt[gindex], "X");
                gtime[gindex] = current_time;
                gindex++;
            }
            current_time++;
        } else {
            // Selected process will execute completely because SJF is non-preemptive
            strcpy(gantt[gindex], local[shortest].id);
            gtime[gindex] = current_time;
            gindex++;

            rt[shortest]  = current_time - local[shortest].at;
            current_time += local[shortest].bt;
            ct[shortest]  = current_time;
            tat[shortest] = ct[shortest] - local[shortest].at;
            wt[shortest]  = tat[shortest] - local[shortest].bt;

            total_wt  += wt[shortest];
            total_tat += tat[shortest];
            total_rt  += rt[shortest];

            completed[shortest] = 1;
            order[completed_count++] = shortest;
        }
    }

    // Display execution summary
    const char *headers[] = {"Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT"};
    char rows[MAX_TX][9][32];
    for (int i = 0; i < n; i++) {
        int p = order[i];
        snprintf(rows[i][0], 32, "%s", local[p].id);
        snprintf(rows[i][1], 32, "%s", local[p].type);
        snprintf(rows[i][2], 32, "%d", local[p].at);
        snprintf(rows[i][3], 32, "%d", local[p].bt);
        snprintf(rows[i][4], 32, "%d", ct[p]);
        snprintf(rows[i][5], 32, "%d", wt[p]);
        snprintf(rows[i][6], 32, "%d", tat[p]);
        snprintf(rows[i][7], 32, "%d", rt[p]);
    }

    render_result("SJF", headers, 8, rows, n,
                  build_gantt("Gantt Chart", gantt, gtime, gindex, current_time), NULL,
                  (float)total_wt/n, (float)total_tat/n, (float)total_rt/n);
}

// ---------- Priority ----------
// Called when Priority button is clicked
// Priority values are read from transaction data entered before running
static void on_priority(GtkWidget *widget, gpointer data) {
    if (tx_count == 0) { set_status("No transactions added."); return; }
    int n = tx_count;
    Transaction local[MAX_TX];
    memcpy(local, tx, sizeof(Transaction) * n);

    int completed[MAX_TX] = {0}, order[MAX_TX];
    int ct[MAX_TX], wt[MAX_TX], tat[MAX_TX], rt[MAX_TX];
    int current_time = 0, completed_count = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;
    char gantt[100][100]; int gtime[100], gindex = 0;

    while (completed_count < n) {
        int selected = -1, highest_priority = -1;

        // First CPU checks which processes have arrived in ready queue
        // Then Priority Scheduling selects highest priority process
        // Condition: higher number means higher priority
        // If priority is same, smaller Process ID will execute first
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 && local[i].at <= current_time) {
                if (selected == -1 ||
                    local[i].priority > highest_priority ||
                   (local[i].priority == highest_priority &&
                    getProcessNumber(local[i].id) < getProcessNumber(local[selected].id))) {
                    highest_priority = local[i].priority;
                    selected = i;
                }
            }
        }

        // If no process has arrived, CPU will stay idle
        if (selected == -1) {
            if (gindex == 0 || strcmp(gantt[gindex-1], "X") != 0) {
                strcpy(gantt[gindex], "X");
                gtime[gindex] = current_time;
                gindex++;
            }
            current_time++;
        } else {
            // Selected process will execute completely because Priority is non-preemptive
            strcpy(gantt[gindex], local[selected].id);
            gtime[gindex] = current_time;
            gindex++;

            rt[selected]  = current_time - local[selected].at;
            current_time += local[selected].bt;
            ct[selected]  = current_time;
            tat[selected] = ct[selected] - local[selected].at;
            wt[selected]  = tat[selected] - local[selected].bt;

            total_wt  += wt[selected];
            total_tat += tat[selected];
            total_rt  += rt[selected];

            completed[selected] = 1;
            order[completed_count++] = selected;
        }
    }

    // Display execution summary
    const char *headers[] = {"Priority", "Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT"};
    char rows[MAX_TX][9][32];
    for (int i = 0; i < n; i++) {
        int p = order[i];
        snprintf(rows[i][0], 32, "%d", local[p].priority);
        snprintf(rows[i][1], 32, "%s", local[p].id);
        snprintf(rows[i][2], 32, "%s", local[p].type);
        snprintf(rows[i][3], 32, "%d", local[p].at);
        snprintf(rows[i][4], 32, "%d", local[p].bt);
        snprintf(rows[i][5], 32, "%d", ct[p]);
        snprintf(rows[i][6], 32, "%d", wt[p]);
        snprintf(rows[i][7], 32, "%d", tat[p]);
        snprintf(rows[i][8], 32, "%d", rt[p]);
    }

    render_result("Priority Scheduling", headers, 9, rows, n,
                  build_gantt("Gantt Chart", gantt, gtime, gindex, current_time), NULL,
                  (float)total_wt/n, (float)total_tat/n, (float)total_rt/n);
}

// ---------- Round Robin ----------
// Called when Round Robin button is clicked
// Time Quantum is taken from Time Quantum entry box
// If Time Quantum box is empty, default value 1 is used
static void on_rr(GtkWidget *widget, gpointer data) {
    if (tx_count == 0) { set_status("No transactions added."); return; }
    int n = tx_count;
    Transaction local[MAX_TX];
    memcpy(local, tx, sizeof(Transaction) * n);

    // Taking time quantum from entry box
    // If box is empty, default time quantum will be 1
    const char *tq_s = gtk_entry_get_text(GTK_ENTRY(entry_quantum));
    int tq = strlen(tq_s) ? atoi(tq_s) : 1;
    if (tq <= 0) tq = 1;

    int remaining_bt[MAX_TX], completed[MAX_TX] = {0}, in_queue[MAX_TX] = {0};
    int ct[MAX_TX], wt[MAX_TX], tat[MAX_TX], rt[MAX_TX];

    for (int i = 0; i < n; i++) {
        remaining_bt[i] = local[i].bt;
        // -1 means process did not get CPU yet
        rt[i] = -1;
    }

    // Sorting by Arrival Time first
    // If Arrival Time is same, smaller Process ID will come first
    // This is important for Round Robin ready queue order
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (local[j].at > local[j+1].at ||
               (local[j].at == local[j+1].at &&
                getProcessNumber(local[j].id) > getProcessNumber(local[j+1].id))) {
                Transaction tt = local[j]; local[j] = local[j+1]; local[j+1] = tt;
                int tmp = remaining_bt[j]; remaining_bt[j] = remaining_bt[j+1]; remaining_bt[j+1] = tmp;
            }
        }
    }

    // Queue is used because Round Robin = FCFS + Time Quantum
    int queue[1000], front = 0, rear = 0;
    int order[1000], order_index = 0;
    int current_time = 0, completed_count = 0;
    int total_wt = 0, total_tat = 0, total_rt = 0;

    // These arrays are used for Ready Queue and Running Queue charts
    char running_chart[1000][100], ready_chart[1000][100];
    int chart_time[1000], chart_index = 0;

    while (completed_count < n) {

        // First CPU checks which process has arrived in ready queue
        // Because already sorted by AT and PID, same AT processes come in proper order
        for (int i = 0; i < n; i++) {
            if (completed[i] == 0 &&
                in_queue[i] == 0 &&
                local[i].at <= current_time &&
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
        ready_chart[chart_index][0] = '\0';
        for (int i = front; i < rear; i++) {
            strcat(ready_chart[chart_index], local[queue[i]].id);
            if (i != rear - 1) strcat(ready_chart[chart_index], ",");
        }

        // Select first process from ready queue
        int selected = queue[front++];
        in_queue[selected] = 0;

        // Store selected process in Running Queue chart
        strcpy(running_chart[chart_index], local[selected].id);
        chart_time[chart_index] = current_time;
        chart_index++;

        // Response Time is calculated only first time when process gets CPU
        if (rt[selected] == -1)
            rt[selected] = current_time - local[selected].at;

        // Process executes for time quantum or remaining burst time
        int execute_time;
        if (remaining_bt[selected] > tq)
            execute_time = tq;
        else
            execute_time = remaining_bt[selected];

        current_time += execute_time;
        remaining_bt[selected] -= execute_time;

        // After time slice, check newly arrived processes
        // They enter ready queue before selected process goes back again
        for (int i = 0; i < n; i++) {
            if (i != selected &&
                completed[i] == 0 &&
                in_queue[i] == 0 &&
                local[i].at <= current_time &&
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
            ct[selected]  = current_time;
            tat[selected] = ct[selected] - local[selected].at;
            wt[selected]  = tat[selected] - local[selected].bt;
            total_wt  += wt[selected];
            total_tat += tat[selected];
            total_rt  += rt[selected];
            // This stores completion order
            order[order_index++] = selected;
        }
    }

    // Display execution summary
    const char *headers[] = {"Process", "Type", "AT", "BT", "CT", "WT", "TAT", "RT"};
    char rows[MAX_TX][9][32];
    for (int i = 0; i < order_index; i++) {
        int p = order[i];
        snprintf(rows[i][0], 32, "%s", local[p].id);
        snprintf(rows[i][1], 32, "%s", local[p].type);
        snprintf(rows[i][2], 32, "%d", local[p].at);
        snprintf(rows[i][3], 32, "%d", local[p].bt);
        snprintf(rows[i][4], 32, "%d", ct[p]);
        snprintf(rows[i][5], 32, "%d", wt[p]);
        snprintf(rows[i][6], 32, "%d", tat[p]);
        snprintf(rows[i][7], 32, "%d", rt[p]);
    }

    char ready_title[50];
    snprintf(ready_title, sizeof(ready_title), "Ready Queue Chart  (Quantum = %d)", tq);

    // Ready Queue Chart first, then Running Queue / Gantt Chart
    render_result("Round Robin", headers, 8, rows, order_index,
                  build_gantt(ready_title, ready_chart, chart_time, chart_index, current_time),
                  build_gantt("Running Queue / Gantt Chart", running_chart, chart_time, chart_index, current_time),
                  (float)total_wt/n, (float)total_tat/n, (float)total_rt/n);
}

// ---------- GUI building ----------

// Helper to create a label + entry pair and attach them to the grid
static void labeled_entry(GtkWidget *grid, const char *label_text,
                           GtkWidget **entry_out, int col, int row) {
    GtkWidget *label = gtk_label_new(label_text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_name(label, "field-label");

    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_widget_set_name(entry, "field-entry");

    gtk_grid_attach(GTK_GRID(grid), label, col * 2,     row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, col * 2 + 1, row, 1, 1);

    *entry_out = entry;
}

// Builds the transaction input table shown below the button row
// Columns: #, Process ID, Type, AT, BT, Priority
static GtkWidget *build_tx_table(void) {
    tx_store = gtk_list_store_new(6,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    tx_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tx_store));
    gtk_widget_set_name(tx_tree, "tx-table");
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tx_tree), TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tx_tree), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    const char *col_names[] = {"#", "Process ID", "Type", "AT", "BT", "Priority"};
    for (int c = 0; c < 6; c++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, "xpad", 10, "ypad", 6, NULL);
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
            col_names[c], renderer, "text", c, NULL);
        gtk_tree_view_column_set_expand(col, TRUE);
        gtk_tree_view_column_set_alignment(col, 0.5);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tx_tree), col);
    }

    refresh_tx_table();

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 120);
    gtk_container_add(GTK_CONTAINER(scroll), tx_tree);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_name(frame, "tx-frame");
    gtk_container_add(GTK_CONTAINER(frame), scroll);
    return frame;
}

// Loads CSS for the whole window
// Pastel color theme with hover and transition effects on all interactive elements
static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: #f5f4fb; }"

        "#title-label { font-size: 24px; font-weight: bold; color: #5b4b8a;"
        "  margin-bottom: 4px; }"
        "#section-label { font-size: 13px; font-weight: bold; color: #5b4b8a;"
        "  margin-top: 2px; }"
        "#field-label { font-size: 12px; color: #555555; }"
        "#status-label { font-size: 11px; color: #7a6fa3; padding-left: 2px; }"

        "#field-entry { padding: 7px 10px; border-radius: 7px;"
        "  border: 1px solid #dcd6f0; background: #ffffff;"
        "  transition: border-color 200ms ease; font-size: 12px; }"
        "#field-entry:focus { border-color: #a594e0; }"

        "#card { background-color: #ffffff; border-radius: 14px;"
        "  border: 1px solid #ece9f7; }"

        "#add-btn { padding: 9px 20px; border-radius: 8px;"
        "  background: #e7e1fb; border: 1px solid #d0c8f5;"
        "  font-weight: bold; color: #4d3d80; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#add-btn:hover { background: #d4cafc; }"

        "#clear-btn { padding: 9px 20px; border-radius: 8px;"
        "  background: #fff0d9; border: 1px solid #f5dfa8;"
        "  font-weight: bold; color: #8a6010; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#clear-btn:hover { background: #ffe8be; }"

        "#clear-all-btn { padding: 9px 20px; border-radius: 8px;"
        "  background: #ffe1ea; border: 1px solid #f5b8c8;"
        "  font-weight: bold; color: #8a1030; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#clear-all-btn:hover { background: #ffc8d8; }"

        "#tx-frame { border-radius: 10px; border: 1px solid #ece9f7; }"
        "#tx-table { font-size: 12px; background: #ffffff; }"
        "#tx-table row:nth-child(odd) { background: #faf9fe; }"
        "#tx-table row:hover { background: #efe9fc; transition: background 150ms ease; }"

        "#algo-fcfs { padding: 13px; border-radius: 10px;"
        "  background: #dff1ff; border: 1px solid #c4e4fb;"
        "  font-weight: bold; color: #1f5c8a; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#algo-fcfs:hover { background: #c6e5ff; }"

        "#algo-sjf { padding: 13px; border-radius: 10px;"
        "  background: #dff7e6; border: 1px solid #c2edcf;"
        "  font-weight: bold; color: #2a7a45; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#algo-sjf:hover { background: #c6f0d4; }"

        "#algo-priority { padding: 13px; border-radius: 10px;"
        "  background: #fff0d9; border: 1px solid #fbe1b3;"
        "  font-weight: bold; color: #95630a; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#algo-priority:hover { background: #ffe4bb; }"

        "#algo-rr { padding: 13px; border-radius: 10px;"
        "  background: #f3e2fb; border: 1px solid #e6c8f7;"
        "  font-weight: bold; color: #7a3a99; font-size: 13px;"
        "  transition: background 180ms ease; }"
        "#algo-rr:hover { background: #e9d0fa; }"

        "#result-heading { font-size: 18px; font-weight: bold; color: #5b4b8a;"
        "  margin-top: 4px; margin-bottom: 2px; }"
        "#subsection-label { font-size: 12px; font-weight: bold; color: #7a6fa3;"
        "  margin-top: 4px; }"

        "#table-frame { border-radius: 10px; border: 1px solid #ece9f7; }"
        "#result-table { font-size: 12px; background: #ffffff; }"
        "#result-table row:nth-child(odd) { background: #faf9fe; }"
        "#result-table row:hover { background: #efe9fc; transition: background 150ms ease; }"

        "#gantt-strip { background: #f8f7fe; border-radius: 10px; }"
        "#gantt-block { background: #e7e1fb; border-radius: 9px;"
        "  border: 1px solid #cec5f5; transition: background 180ms ease; }"
        "#gantt-block:hover { background: #d4cafc; }"
        "#gantt-idle  { background: #ebebeb; border-radius: 9px;"
        "  border: 1px solid #d8d8d8; }"
        "#gantt-id    { font-weight: bold; font-size: 12px; color: #4d3d80; }"
        "#gantt-time  { font-size: 10px; color: #999999; }"

        "#stat-card-pink  { background: #ffe1ea; border-radius: 14px; }"
        "#stat-card-blue  { background: #dff1ff; border-radius: 14px; }"
        "#stat-card-green { background: #dff7e6; border-radius: 14px; }"
        "#stat-value   { font-size: 26px; font-weight: bold; color: #4d3d80; }"
        "#stat-caption { font-size: 11px; color: #6b6584; }";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

// Function to close application
void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    apply_css();

    // Create window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bank Transaction Processing System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1060, 860);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

    // Main scroll window so entire UI is scrollable if window is small
    GtkWidget *main_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(main_scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), main_scroll);

    // Outer vertical box holds everything
    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_container_set_border_width(GTK_CONTAINER(outer), 24);
    gtk_container_add(GTK_CONTAINER(main_scroll), outer);

    // Title label at the top
    GtkWidget *title = gtk_label_new("Bank Transaction Processing System");
    gtk_widget_set_name(title, "title-label");
    gtk_box_pack_start(GTK_BOX(outer), title, FALSE, FALSE, 0);

    // Card wrapping the input form
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_name(card, "card");
    gtk_container_set_border_width(GTK_CONTAINER(card), 20);
    gtk_box_pack_start(GTK_BOX(outer), card, FALSE, FALSE, 0);

    // Grid holds all the labels and entry boxes in 2-column layout
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 14);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_pack_start(GTK_BOX(card), grid, FALSE, FALSE, 0);

    // Place labels and entries in grid
    labeled_entry(grid, "Transaction ID:",          &entry_id,       0, 0);
    labeled_entry(grid, "Burst Time:",              &entry_bt,       1, 0);
    labeled_entry(grid, "Transaction Type:",        &entry_type,     0, 1);
    labeled_entry(grid, "Priority (optional):",     &entry_priority, 1, 1);
    labeled_entry(grid, "Arrival Time:",            &entry_at,       0, 2);
    labeled_entry(grid, "Time Quantum (optional):", &entry_quantum,  1, 2);

    // Button row: Add Transaction | Clear | Clear All on same row
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(card), btn_row, FALSE, FALSE, 0);

    GtkWidget *btn_add       = gtk_button_new_with_label("Add Transaction");
    GtkWidget *btn_clear     = gtk_button_new_with_label("Clear Last");
    GtkWidget *btn_clear_all = gtk_button_new_with_label("Clear All");

    gtk_widget_set_name(btn_add,       "add-btn");
    gtk_widget_set_name(btn_clear,     "clear-btn");
    gtk_widget_set_name(btn_clear_all, "clear-all-btn");

    gtk_widget_set_hexpand(btn_add, TRUE);

    g_signal_connect(btn_add,       "clicked", G_CALLBACK(on_add_transaction), NULL);
    g_signal_connect(btn_clear,     "clicked", G_CALLBACK(on_clear_last),      NULL);
    g_signal_connect(btn_clear_all, "clicked", G_CALLBACK(on_clear_all),       NULL);

    gtk_box_pack_start(GTK_BOX(btn_row), btn_add,       TRUE,  TRUE,  0);
    gtk_box_pack_start(GTK_BOX(btn_row), btn_clear,     FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_row), btn_clear_all, FALSE, FALSE, 0);

    // Status label shows confirmation or validation messages
    status_label = gtk_label_new("");
    gtk_widget_set_name(status_label, "status-label");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(status_label), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start(GTK_BOX(card), status_label, FALSE, FALSE, 0);

    // Transaction table shows all added transactions
    gtk_box_pack_start(GTK_BOX(card), build_tx_table(), TRUE, TRUE, 0);

    // Scheduling algorithms section label
    GtkWidget *sec_label = gtk_label_new("Scheduling Algorithms");
    gtk_widget_set_name(sec_label, "section-label");
    gtk_box_pack_start(GTK_BOX(outer), sec_label, FALSE, FALSE, 0);

    // Horizontal box for the four algorithm buttons
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_set_homogeneous(GTK_BOX(btn_box), TRUE);
    gtk_box_pack_start(GTK_BOX(outer), btn_box, FALSE, FALSE, 0);

    GtkWidget *btn_fcfs     = gtk_button_new_with_label("FCFS");
    GtkWidget *btn_sjf      = gtk_button_new_with_label("SJF");
    GtkWidget *btn_priority = gtk_button_new_with_label("Priority");
    GtkWidget *btn_rr       = gtk_button_new_with_label("Round Robin");

    gtk_widget_set_name(btn_fcfs,     "algo-fcfs");
    gtk_widget_set_name(btn_sjf,      "algo-sjf");
    gtk_widget_set_name(btn_priority, "algo-priority");
    gtk_widget_set_name(btn_rr,       "algo-rr");

    g_signal_connect(btn_fcfs,     "clicked", G_CALLBACK(on_fcfs),     NULL);
    g_signal_connect(btn_sjf,      "clicked", G_CALLBACK(on_sjf),      NULL);
    g_signal_connect(btn_priority, "clicked", G_CALLBACK(on_priority), NULL);
    g_signal_connect(btn_rr,       "clicked", G_CALLBACK(on_rr),       NULL);

    // Add buttons to the algorithm button row
    gtk_box_pack_start(GTK_BOX(btn_box), btn_fcfs,     TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn_sjf,      TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn_priority, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn_rr,       TRUE, TRUE, 0);

    // Output area section label
    GtkWidget *out_label = gtk_label_new("Output Area");
    gtk_widget_set_name(out_label, "section-label");
    gtk_box_pack_start(GTK_BOX(outer), out_label, FALSE, FALSE, 0);

    // Output container holds heading + table + gantt + stat cards dynamically
    output_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_container_set_border_width(GTK_CONTAINER(output_container), 4);
    gtk_box_pack_start(GTK_BOX(outer), output_container, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}