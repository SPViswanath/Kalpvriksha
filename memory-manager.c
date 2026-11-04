
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FREE_B, USED_B };

typedef struct {
    size_t offset, size;
    int id, status;
} Block;

enum { FIRST_FIT = 1, BEST_FIT = 2, WORST_FIT = 3 };

static unsigned char *memory_pool = NULL;
static size_t pool_size = 0;
static Block *blocks = NULL;
static size_t block_count = 0, block_capacity = 8;
static int next_allocation_id = 1, allocation_strategy = FIRST_FIT;

static int read_num(const char *prompt, long *input_value)
{
    printf("%s", prompt);

    while (scanf("%ld", input_value) != 1) {
        if (feof(stdin))
            return -1;

        scanf("%*[^\n]");
        printf("Invalid input. %s", prompt);
    }
    return 0;
}

static int init_pool(long requested_size)
{
    if (requested_size < 1 || requested_size > 1000000) {
        printf("Pool size must be 1 to 1000000.\n");
        return -1;
    }
    memory_pool = malloc(requested_size);
    blocks = malloc(block_capacity * sizeof(Block));

    if (!memory_pool || !blocks) {
        free(memory_pool);
        free(blocks);
        printf("Memory allocation failed.\n");
        return -1;
    }
    pool_size = requested_size;
    block_count = 1;
    block_capacity = 8;
    next_allocation_id = 1;

    blocks[0] = (Block){0, (size_t)requested_size, 0, FREE_B};

    return 0;
}

static int insert_block(size_t block_index, Block new_block)
{
    if (block_count == block_capacity) {
        Block *resized_blocks = realloc(
            blocks, block_capacity * 2 * sizeof(Block)
        );

        if (!resized_blocks)
            return -1;  /* old array still valid */

        blocks = resized_blocks;
        block_capacity *= 2;
    }

    memmove(
        &blocks[block_index + 1],
        &blocks[block_index],
        (block_count - block_index) * sizeof(Block)
    );

    blocks[block_index] = new_block;
    block_count++;

    return 0;
}

static void remove_block(size_t block_index)
{
    memmove(&blocks[block_index], &blocks[block_index + 1],(block_count - block_index - 1) * sizeof(Block));

    block_count--;
}

static int find_block(long allocation_id)
{
    for (size_t block_index = 0; block_index < block_count; block_index++) {
        if (blocks[block_index].status == USED_B &&
            blocks[block_index].id == allocation_id)
            return (int)block_index;
    }

    return -1;
}

/* ---- allocation strategies: return block index or -1 ---- */

static int first_fit(size_t requested_size)
{
    for (size_t block_index = 0; block_index < block_count; block_index++) {
        if (blocks[block_index].status == FREE_B &&
            blocks[block_index].size >= requested_size)
            return (int)block_index;
    }

    return -1;
}

static int best_fit(size_t requested_size)
{
    int best_index = -1;

    for (size_t block_index = 0; block_index < block_count; block_index++) {
        if (blocks[block_index].status == FREE_B &&
            blocks[block_index].size >= requested_size &&
            (best_index < 0 ||
             blocks[block_index].size < blocks[best_index].size))
            best_index = (int)block_index;
    }

    return best_index;
}

static int worst_fit(size_t requested_size)
{
    int worst_index = -1;

    for (size_t block_index = 0; block_index < block_count; block_index++) {
        if (blocks[block_index].status == FREE_B &&
            blocks[block_index].size >= requested_size &&
            (worst_index < 0 ||
             blocks[block_index].size > blocks[worst_index].size))
            worst_index = (int)block_index;
    }

    return worst_index;
}

static int find_free(size_t requested_size)
{
    return allocation_strategy == 2 ? best_fit(requested_size) :
           allocation_strategy == 3 ? worst_fit(requested_size) :
           first_fit(requested_size);
}

/* Shrink block i to `size`; leftover becomes a free block after it.
   Inserts first, so on failure nothing has changed. */

static int split(size_t block_index, size_t requested_size)
{
    if (blocks[block_index].size > requested_size) {
        Block remaining_block = {
            blocks[block_index].offset + requested_size,
            blocks[block_index].size - requested_size,
            0,
            FREE_B
        };

        if (insert_block(block_index + 1, remaining_block) != 0)
            return -1;
    }

    blocks[block_index].size = requested_size;

    return 0;
}

static void merge_free_blocks(void)
{
    for (size_t block_index = 0; block_index + 1 < block_count;) {
        if (blocks[block_index].status == FREE_B &&
            blocks[block_index + 1].status == FREE_B) {
            blocks[block_index].size += blocks[block_index + 1].size;
            remove_block(block_index + 1);
        } else {
            block_index++;
        }
    }
}

static size_t total_free(void)
{
    size_t free_bytes = 0;

    for (size_t block_index = 0; block_index < block_count; block_index++) {
        if (blocks[block_index].status == FREE_B)
            free_bytes += blocks[block_index].size;
    }

    return free_bytes;
}

static void report_failure(size_t requested_size)
{
    size_t free_bytes = total_free();

    printf(
        "Failed: no contiguous free block is large enough for %zu bytes.\n",
        requested_size
    );

    if (free_bytes >= requested_size) {
        printf(
            "External fragmentation: %zu bytes are free in total, but only as separate blocks:",
            free_bytes
        );

        for (size_t block_index = 0; block_index < block_count; block_index++) {
            if (blocks[block_index].status == FREE_B)
                printf(" %zu", blocks[block_index].size);
        }

        printf("\n");
    }
}

static void allocate_memory(void)
{
    long requested_size;

    if (read_num("Requested size: ", &requested_size))
        return;

    if (requested_size <= 0 || (size_t)requested_size > pool_size) {
        printf("Invalid size.\n");
        return;
    }

    int block_index = find_free(requested_size);

    if (block_index < 0) {
        report_failure(requested_size);
        return;
    }

    if (split(block_index, requested_size) != 0) {
        printf("Allocation failed: out of metadata memory.\n");
        return;
    }

    blocks[block_index].status = USED_B;
    blocks[block_index].id = next_allocation_id++;

    printf(
        "Allocation successful. ID: %d, Start: %zu, Size: %ld\n",
        blocks[block_index].id,
        blocks[block_index].offset,
        requested_size
    );
}

static void release_memory(void)
{
    long allocation_id;

    if (read_num("Allocation ID: ", &allocation_id))
        return;

    int block_index = find_block(allocation_id);

    if (block_index < 0) {
        printf("Unknown ID or block already free.\n");
        return;
    }

    blocks[block_index].status = FREE_B;
    blocks[block_index].id = 0;

    merge_free_blocks();

    printf("Allocation released.\n");
}

static void resize_allocation(void)
{
    long allocation_id, requested_size;

    if (read_num("Allocation ID: ", &allocation_id) ||
        read_num("New size: ", &requested_size))
        return;

    int block_index = find_block(allocation_id);

    if (block_index < 0) {
        printf("Unknown ID or block already free.\n");
        return;
    }

    if (requested_size <= 0 || (size_t)requested_size > pool_size) {
        printf("Invalid size.\n");
        return;
    }

    size_t new_size = requested_size;
    size_t old_size = blocks[block_index].size;

    if (new_size <= old_size) {  /* shrink */
        if (split(block_index, new_size) != 0) {
            printf("Resize failed.\n");
            return;
        }

        merge_free_blocks();
    }
    else if ((size_t)block_index + 1 < block_count &&
             blocks[block_index + 1].status == FREE_B &&
             old_size + blocks[block_index + 1].size >= new_size) {
        /* grow in place */

        blocks[block_index + 1].offset += new_size - old_size;
        blocks[block_index + 1].size -= new_size - old_size;
        blocks[block_index].size = new_size;

        if (blocks[block_index + 1].size == 0)
            remove_block(block_index + 1);
    }
    else {  /* move */
        int target_index = find_free(new_size);

        if (target_index < 0 || split(target_index, new_size) != 0) {
            printf("Resize failed; original allocation unchanged.\n");
            report_failure(new_size);
            return;
        }

        if (target_index < block_index)
            block_index++;  /* array shifted by split */

        blocks[target_index].status = USED_B;
        blocks[target_index].id = (int)allocation_id;

        memcpy(
            memory_pool + blocks[target_index].offset,
            memory_pool + blocks[block_index].offset,
            old_size
        );

        blocks[block_index].status = FREE_B;
        blocks[block_index].id = 0;

        merge_free_blocks();
    }

    printf("Resize successful.\n");
}

static void display_memory_map(void)
{
    printf("Memory Map (%zu bytes):\n", pool_size);

    for (size_t block_index = 0; block_index < block_count; block_index++) {
        Block *block = &blocks[block_index];

        if (block->status == USED_B)
            printf(
                "[Allocated #%d: %zu bytes, Start: %zu]\n",
                block->id, block->size, block->offset
            );
        else
            printf(
                "[Free: %zu bytes, Start: %zu]\n",
                block->size, block->offset
            );
    }
}

static void display_statistics(void)
{
    size_t free_blocks = 0;
    size_t used_blocks = 0;
    size_t largest_free_block = 0;

    for (size_t block_index = 0; block_index < block_count; block_index++) {
        Block *block = &blocks[block_index];

        if (block->status == USED_B)
            used_blocks++;
        else {
            free_blocks++;

            if (block->size > largest_free_block)
                largest_free_block = block->size;
        }
    }

    size_t free_bytes = total_free();

    printf(
        "Total memory: %zu bytes\nAllocated: %zu bytes\nFree: %zu bytes\n",
        pool_size, pool_size - free_bytes, free_bytes
    );

    printf(
        "Allocated blocks: %zu\nFree blocks: %zu\nLargest free block: %zu bytes\n",
        used_blocks, free_blocks, largest_free_block
    );
}

static int choose_strategy(void)
{
    long strategy_choice;

    do {
        if (read_num(
                "Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ",
                &strategy_choice
            ))
            return -1;

    } while (strategy_choice < 1 || strategy_choice > 3);

    allocation_strategy = (int)strategy_choice;

    printf(
        "Strategy set to %s Fit.\n",
        strategy_choice == 1 ? "First" :
        strategy_choice == 2 ? "Best" : "Worst"
    );

    return 0;
}

int main(void)
{
    long requested_size, menu_choice = 0;

    do {
        if (read_num("Enter memory pool size: ", &requested_size))
            return 1;
    } while (init_pool(requested_size) != 0);

    if (choose_strategy())
        return 1;

    while (menu_choice != 7) {
        printf(
            "\nMenu:\n"
            "1. Allocate memory\n"
            "2. Release memory\n"
            "3. Resize allocation\n"
            "4. Display memory map\n"
            "5. Display memory statistics\n"
            "6. Change allocation strategy\n"
            "7. Exit\n"
        );

        if (read_num("Choice: ", &menu_choice))
            break;

        switch (menu_choice) {
            case 1:
                allocate_memory();
                break;

            case 2:
                release_memory();
                break;

            case 3:
                resize_allocation();
                break;

            case 4:
                display_memory_map();
                break;

            case 5:
                display_statistics();
                break;

            case 6:
                if (choose_strategy())
                    menu_choice = 7;
                break;

            case 7:
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    free(memory_pool);
    free(blocks);

    return 0;
}
