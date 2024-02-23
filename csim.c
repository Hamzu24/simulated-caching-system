#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <errno.h>

struct instruction {
    long addr;
    int size;
    char op;
};

struct ll_node {
    struct instruction data;
    struct ll_node *next;
};

typedef struct ll_node *ll_node_t;

struct linked_list {
    ll_node_t head;
    ll_node_t tail;
};

typedef struct linked_list *linked_list_t;

void sufficient_memory_check(void *val, const char err_msg[]) {
    if (val == NULL) {
        printf("%s", err_msg);
    }
}

void add_ll_node(linked_list_t list, ll_node_t node) {
    if (node != NULL) {
        list->tail->next = node;
        list->tail = node;
    } else {
        printf("Error: passed in NULL node to add to a linked list");
    }
}

int process_trace_file (const char *trace, linked_list_t instructions, unsigned long v_flag) { //0 for success, 1 for error
    FILE *tfp = fopen(trace, "rt");
    if (!tfp) {
        fprintf(stderr, "Error opening '%s': %s\n", trace, strerror(errno));
        return 1;
    }

    const int LINELEN = 13;
    char linebuf[LINELEN];
    int parse_error = 0;
    unsigned long line_num = 0;

    char separators[2] = {' ', ','};
    char *token;

    if (v_flag) {
        printf("\n------------------------PROCESSING FILE------------------------\n\n");
    }

    while (fgets(linebuf, LINELEN, tfp)) {
        line_num++;
        if (v_flag) {
            printf("\nNext Line to be processed is #%lu: %s\n", line_num, linebuf);
        }

        token = strtok(linebuf, separators);

        ll_node_t newNode = calloc(1, sizeof(struct ll_node));
        sufficient_memory_check(newNode, "Insufficient memory!");

        while (token != NULL) {
            if (v_flag) {
                printf("Next token to be processed: %s\n", token);
            }
            token = strtok(NULL, separators);
        }


    }
    fclose(tfp);
    return parse_error;
}

void usage(void) {
    printf("Usage: ./csim -ref [-v] -s <s> -E <E> -b <b> -t <trace >\n ./csim -ref -h\n -h Print this help message and exit\n -v Verbose mode: report effects of each memory operation\n -s <s> Number of set index bits (there are 2**s sets)\n -b <b> Number of block bits (there are 2**b blocks)\n -E <E> Number of lines per set ( associativity )\n -t <trace > File name of the memory trace to process\n");
}

int main(int argc, char **argv) {

    linked_list_t instructions = calloc(1, sizeof(struct linked_list));
    sufficient_memory_check(instructions, "Insufficient memory!");

    int ch;
    unsigned long v_flag= 0;
    unsigned long req_flags[] = {0, 0, 0}; // -s, -E, -b
    char *file_name;

    while ((ch = getopt(argc, argv, "s:E:b:t:v")) != -1) {
        switch (ch) {
            case 's':
                printf("received argument: %s\n", optarg);
                req_flags[0] = strtoul(optarg, NULL, 10);
            break;

            case 'E':
                printf("received argument: %s\n", optarg);
                req_flags[1] = strtoul(optarg, NULL, 10);
            break;

            case 'b':
                printf("received argument: %s\n", optarg);
                req_flags[2] = strtoul(optarg, NULL, 10);
            break;

            case 't':
                printf("received argument: %s\n", optarg);

                size_t index = 0;
                while (optarg[index] != '\0') {
                    index++;
                }

                size_t length = index;
                file_name = malloc(sizeof(char) * (index+1));
                index = 0;

                while (index < length) {
                    file_name[index] = optarg[index];
                    index++;
                }
                file_name[length] = '\0';

            break;

            case 'v':
                printf("received argument: %s\n", optarg);
                v_flag = 1;
            break;

            default:
                usage();
                exit(0);
        }
    }

    unsigned long curFlag;
    for (int i = 0; i < 3; i++) {
        curFlag = req_flags[i];
        if (curFlag <= 0) {
            printf("Error: all flags must be >= 0");
            exit(0);
        }
    }


    if (req_flags[0] + req_flags[2] > 16) {
        printf("Error: Values of s and b are cumulatively too large! (s + b > 16)");
    }

    if (v_flag) {
        printf("Verbose argumet set to 1...\n");
    }

    int error_status = process_trace_file(file_name, instructions, v_flag);
    printf("%d\n", error_status);
}

