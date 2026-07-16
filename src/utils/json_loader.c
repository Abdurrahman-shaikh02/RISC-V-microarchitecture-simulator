#include "header.h"
#include "cache.h"
#include "control.h"
#include "branch_prediction.h"
#include "memory.h"
#include "cJSON.h"

#define READ_INT(json, name, var)                     \
do {                                                  \
    cJSON *tmp = cJSON_GetObjectItemCaseSensitive(json, name); \
    if(cJSON_IsNumber(tmp))                           \
        (var) = tmp->valueint;                        \
} while(0)


static void read_cache_config(cJSON *json, cache_level *c)
{
    if(json == NULL) return;

    READ_INT(json, "cache_size", c->cache_size);
    READ_INT(json, "block_size", c->block_size);
    READ_INT(json, "associativity", c->associativity);
    READ_INT(json, "read_latency", c->read_latency);
    READ_INT(json, "write_latency", c->write_latency);
    READ_INT(json, "write_miss_policy", c->write_miss_policy);
    READ_INT(json, "replacement_policy", c->replacement_policy);
    READ_INT(json, "write_policy", c->write_policy);
}


void load_config_json(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("Could not open config file %s\n", filename);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *buffer = malloc(len + 1);
    if(buffer == NULL)
    {
        fclose(fp);
        return;
    }

    fread(buffer, 1, len, fp);
    buffer[len] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if(root == NULL)
    {
        printf("Invalid JSON configuration.\n");
        return;
    }

    /*---------------- Global ----------------*/

    READ_INT(root, "CACHE_LEVELS", CACHE_LEVELS);
    READ_INT(root, "L1_SPLIT", L1_SPLIT);

    READ_INT(root, "DRAM_READ_LATENCY", DRAM_READ_LATENCY);
    READ_INT(root, "DRAM_WRITE_LATENCY", DRAM_WRITE_LATENCY);
    READ_INT(root, "DRAM_SIZE", DRAM_SIZE);

    READ_INT(root, "FORWARDING_SWITCH", FORWARDING_SWITCH);
    READ_INT(root, "BRANCH_PREDICTION_STRATEGY", BRANCH_PREDICTION_STRATEGY);

    READ_INT(root, "text_segment_limit", text_segment_limit);
    READ_INT(root, "rodata_segment_limit", rodata_segment_limit);

    /*---------------- L1 Data ----------------*/

    read_cache_config(
        cJSON_GetObjectItemCaseSensitive(root, "l1_d"),
        &l1_d
    );

    /*---------------- L1 Instruction ----------------*/

    read_cache_config(
        cJSON_GetObjectItemCaseSensitive(root, "l1_i"),
        &l1_i
    );

    /*---------------- L2 ----------------*/

    read_cache_config(
        cJSON_GetObjectItemCaseSensitive(root, "l2"),
        &l2
    );

    /*---------------- L3 ----------------*/

    read_cache_config(
        cJSON_GetObjectItemCaseSensitive(root, "l3"),
        &l3
    );

    /*
        These are fixed by hierarchy.
        Ignore whatever user writes.
    */

    l1_d.level = 1;
    l1_i.level = 1;
    l2.level = 2;
    l3.level = 3;

    total_number_of_instructions =
        (text_segment_limit + 1) / 4;

    cJSON_Delete(root);
}
