#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

struct instruction {
    unsigned long addr;
    unsigned long size;
    char op;
};

typedef struct instruction *instruction_t;

struct ll_node {
    instruction_t data;
    struct ll_node *next;
};

typedef struct ll_node *node_t;

struct linked_list {
    node_t head;
    node_t tail;
};

typedef struct linked_list *linked_list_t;

struct line {
    unsigned long tag;
    long cycles_since_use;
    bool isDirty;
    bool isValid;
};

typedef struct line *line_t;

void sufficient_memory_check(void *val, const char err_msg[]) {
    if (val == NULL) {
        printf("%s", err_msg);
    }
}

line_t get_cache_index(line_t cache[], int num_lines, unsigned long i, unsigned long j) {
    return cache[(i * ((unsigned long) num_lines)) + j];
}

void add_ll_node(linked_list_t list, node_t node) {
    if (node != NULL) {
        if (list->tail == NULL) {
            list->tail = node;
            list->head = node;
        } else {
            list->tail->next = node;
            list->tail = node;
        }
    } else {
        printf("Error: passed in NULL node to add to a linked list\n");
    }
}

void free_ll(linked_list_t list) {
    node_t curNode = list->head;
    node_t prev = NULL;
    while (curNode != NULL) {
        free(curNode->data);
        prev = curNode;
        curNode = curNode->next;
        free(prev);
    }
    free(list);
}

void display_instruction(instruction_t instruct) {
    printf("Op: %c, Addr: %lu, Size: %lu\n", instruct->op, instruct->addr, instruct->size);
}

void display_ll(linked_list_t list) {
    if (list != NULL) {
        size_t node_num = 0;
        node_t curNode = list->head;

        while (curNode != NULL) {
            printf("Now displaying node #%zu:\n", node_num);
            display_instruction(curNode->data);

            node_num++;
            curNode = curNode->next;
        }
    } else {
        printf("Error: passed in NULL linked list to display\n");
    }
    printf("Displayed linked_list fully\n");
}

int process_trace_file (const char *trace, linked_list_t instructions, unsigned long v_flag, unsigned long req_flags[3]) { //0 for success, 1 for error
    FILE *tfp = fopen(trace, "rt");
    if (!tfp) {
        fprintf(stderr, "Error opening '%s': %s\n", trace, strerror(errno));
        return 1;
    }

    const int LINELEN = 22;
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
            printf("\nNext Line to be processed is #%lu: '%s'\n", line_num, linebuf);
        }

        if (strcmp(linebuf, "\n\0")) {

            size_t info_index = 0; //0 = op, 1 = addr, 2 = size
            token = strtok(linebuf, separators);

            node_t newNode = calloc(1, sizeof(struct ll_node));
            newNode->data = calloc(1, sizeof(struct instruction));
            sufficient_memory_check(newNode, "Insufficient memory!");

            while (token != NULL) {
                if (v_flag) {
                    printf("    Next token to be processed: %s (with info_index of %zu)\n", token, info_index);
                }

                if (info_index == 0) {
                    if ((!strcmp(token, "L")) || (!strcmp(token, "S"))) {
                        newNode->data->op = token[0];
                    } else {
                        printf("Incorrect instruction type on line #%lu. Received: %s\n", line_num, token);
                        exit(0);
                    }
                } else if (info_index == 1) {
                    size_t length = 0;
                    while (token[length] != '\0') {
                        length++;
                    }

                    /*if (length > 16 || ) {

                    }*/
                    newNode->data->addr = strtoul(token, NULL, 16);
                } else {
                    newNode->data->size = strtoul(token, NULL, 10);
                }

                info_index++;
                token = strtok(NULL, separators);
            }

            info_index = 0;
            add_ll_node(instructions, newNode);
        } else {
            if (v_flag) {
                printf("Skipping this line...\n");
            }
        }

    }

    if (v_flag) {
        printf("\n------------------------DONE PROCESSING FILE------------------------\n\n");
        printf("Created linked_list: \n");
        display_ll(instructions);
        printf("Reached");
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
                req_flags[0] = strtoul(optarg, NULL, 10);

            break;

            case 'E':
                req_flags[1] = strtoul(optarg, NULL, 10);

            break;

            case 'b':
                req_flags[2] = strtoul(optarg, NULL, 10);

            break;

            case 't':
                printf("");
                size_t index = 0;
                while (optarg[index] != '\0') {
                    index++;
                }

                size_t length = index+1;
                file_name = malloc(sizeof(char) * length);

                index = 0;
                while (index < length) {
                    file_name[index] = optarg[index];
                    index++;
                }
                file_name[length-1] = '\0';

            break;

            case 'v':
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
        if (curFlag < 0 || (i == 1 && curFlag == 0)) {
            printf("Error: E must be > 0 and s, b >= 0\n");
            exit(0);
        }
    }

    if (file_name == NULL) {
        printf("Error: did not specify a trace file to execute\n");
        exit(1);
    }

    if (req_flags[0] + req_flags[2] > 63) {
        printf("Error: Values of s and b are cumulatively too large!\n");
        exit(1);
    }


    if (v_flag) {
        printf("Verbose argumet set to 1...\n");
    }

    int num_sets = (int) pow(2, (int) req_flags[0]);
    int num_lines = (int) req_flags[1];
    int error_status = process_trace_file(file_name, instructions, v_flag, req_flags);
    if (error_status != 0) {
        printf("Fatal error in parsing the trace file...\n");
        exit(1);
    }

    line_t cache[num_sets * num_lines];
    for (int i = 0; i < num_sets * num_lines; i++) {
        cache[i] = calloc(1, sizeof(struct line));
        sufficient_memory_check(cache[i], "Insufficient Memory to create cache on Heap!\n");
        cache[i]->isValid = false;
        cache[i]->isDirty = false;
    }

    node_t curNode = instructions->head;

    unsigned long sb_sum = req_flags[0] + req_flags[2];
    unsigned long tag_shl = (unsigned long) 64 - sb_sum;
    unsigned long set_mask = ~(0xFFFFFFFFFFFFFFFFL << (long) req_flags[0]);
    unsigned long tag_mask;
    if (tag_shl == 64) {
       tag_mask = 0xFFFFFFFFFFFFFFFFL;
    } else {
       tag_mask = ~(0xFFFFFFFFFFFFFFFFL << (long) (64 - sb_sum));
    }

    if (v_flag) {
        printf("set_mask: %lu, tag_mask: %lu\n", set_mask, tag_mask);
    }

    csim_stats_t *stats = calloc(1, sizeof(long)*5);
    while (curNode != NULL) {
        instruction_t curInstruction = curNode->data;

        unsigned long tag = (curInstruction->addr >> (long) sb_sum) & tag_mask;
        unsigned long set = (curInstruction->addr >> req_flags[2]) & set_mask;
        if (v_flag) {
            printf("Next Instruction: Op: (%c), Addr: (%lu), Size: (%lu)\n", curInstruction->op, curInstruction->addr, curInstruction->size);
            printf("Tag: %lu, Set: %lu\n\n", tag, set);
        }

        bool isHit = false;
        long valid_count = 0;
        long LRU = 1;
        long LRU_cycles = -1;
        if (v_flag) {
            printf("Beginning check of set for instructed tag now\n");
        }
        for (int l = 0; l < num_lines; l++) {
            if (v_flag) {
                printf("Checking Line %d: ", l);
            }
            line_t curLine = get_cache_index(cache, num_lines, set, (unsigned long) l);

            if (curLine->tag == tag && curLine->isValid) {
                if (v_flag) {
                    printf("This line had the instructed tag!!\n");
                }
                isHit = true;
                LRU = l; //LRU is overloaded to also hold the index of the line where the HIT occured in the set
            } else if (curLine->isValid) {
                valid_count++;

                if (!isHit && curLine->cycles_since_use > LRU_cycles) {
                    if (v_flag) {
                        printf("This is the new least recently used line!\n");
                    }
                    LRU = l;
                    LRU_cycles = curLine->cycles_since_use;
                }

                curLine->cycles_since_use++;
            } else if (!isHit) {
                if (v_flag) {
                    printf("This line was unused!\n");
                }
                LRU = l; //LRU overloaded yet again to hold the index of the line that is currently not used
                LRU_cycles = 9999999;
            } else {
                if (v_flag) {
                    printf("\n");
                }
            }
        }


        if (isHit) {
            stats->hits++;
            if (v_flag) {
                printf("Hit! With line #%lu\n\n\n", LRU);
            }
            line_t hit_line = get_cache_index(cache, num_lines, set, (unsigned long) LRU);
            hit_line->cycles_since_use = 0;

        } else {
            stats->misses++;
            if (valid_count == num_lines) {
                stats->evictions++;

                line_t evicted_line = get_cache_index(cache, num_lines, set, (unsigned long) LRU);
                if (v_flag) {
                    printf("Miss and eviction! Line #%ld was evicted and had tag %lu, but now has tag %lu\n\n\n", LRU, evicted_line->tag, tag);
                }
                if (evicted_line->isDirty) {
                    stats->dirty_evictions++;
                    stats->dirty_bytes--;
                    evicted_line->isDirty = false;
                }
                evicted_line->tag = tag;
                evicted_line->cycles_since_use = 0;
            } else {
                if (v_flag) {
                    printf("Miss, no eviction! Inserting the address into line #%ld with tag %lu\n\n\n", LRU, tag);
                }
                line_t new_line = get_cache_index(cache, num_lines, set, (unsigned long) LRU);
                new_line->isValid = true;
                new_line->tag = tag;
            }
        }

        if (curInstruction->op == 'S') {
            line_t cur_line = get_cache_index(cache, num_lines, set, (unsigned long) LRU);
            if (!cur_line->isDirty) {
                stats->dirty_bytes++;
            }
            cur_line->isDirty = true;
        }

        curNode = curNode->next;
    }

    unsigned long multiplier = (unsigned long) pow(2, (int) req_flags[2]);
    stats->dirty_bytes = (stats->dirty_bytes * multiplier);
    stats->dirty_evictions = (stats->dirty_evictions * multiplier);
    printSummary(stats);

    free(stats);
    free_ll(instructions);
    free(file_name);

    for (int i = 0; i < num_sets * num_lines; i++) {
        free(cache[i]);
    }
}

