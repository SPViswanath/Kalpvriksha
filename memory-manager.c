#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FREE_BLOCK, USED_BLOCK };
enum { FIRST_FIT = 1, BEST_FIT, WORST_FIT };

typedef struct {
    size_t start_offset;
    size_t block_size;
    int allocation_id;
    int status;
} Block;

typedef struct {
    unsigned char *pool;
    size_t pool_size;
    Block *blocks;
    size_t block_count;
    size_t block_capacity;
    int next_allocation_id;
    int strategy;
} MemoryManager;

static int read_number(const char *prompt, long *value)
{
    printf("%s", prompt);

    while (scanf("%ld", value) != 1) {
        if (feof(stdin)) return -1;
        scanf("%*[^\n]");
        printf("Invalid input. %s", prompt);
    }

    return 0;
}

static int init_pool(MemoryManager *manager, long requested_pool_size)
{
    if (requested_pool_size < 1 || requested_pool_size > 1000000) {
        printf("Pool size must be 1 to 1000000.\n");
        return -1;
    }

    manager->pool = malloc(requested_pool_size);
    manager->blocks = malloc(8 * sizeof(Block));

    if (manager->pool == NULL || manager->blocks == NULL) {
        free(manager->pool);
        free(manager->blocks);
        printf("Memory allocation failed.\n");
        return -1;
    }

    manager->pool_size = requested_pool_size;
    manager->block_count = 1;
    manager->block_capacity = 8;
    manager->next_allocation_id = 1;
    manager->blocks[0] = (Block){0, (size_t)requested_pool_size, 0, FREE_BLOCK};

    return 0;
}

static int insert_block(MemoryManager *manager, size_t position, Block new_block)
{
    if (manager->block_count == manager->block_capacity) {
        Block *grown_blocks = realloc(manager->blocks, manager->block_capacity * 2 * sizeof(Block));

        if (grown_blocks == NULL) return -1;

        manager->blocks = grown_blocks;
        manager->block_capacity *= 2;
    }

    memmove(&manager->blocks[position + 1], &manager->blocks[position], (manager->block_count - position) * sizeof(Block));
    manager->blocks[position] = new_block;
    manager->block_count++;

    return 0;
}

static void remove_block(MemoryManager *manager, size_t position)
{
    memmove(&manager->blocks[position], &manager->blocks[position + 1], (manager->block_count - position - 1) * sizeof(Block));
    manager->block_count--;
}

static int find_block_by_id(MemoryManager *manager, long allocation_id)
{
    for (size_t index = 0; index < manager->block_count; index++) {
        if (manager->blocks[index].status == USED_BLOCK && manager->blocks[index].allocation_id == allocation_id)
            return (int)index;
    }

    return -1;
}

static int first_fit(MemoryManager *manager, size_t request_size)
{
    for (size_t index = 0; index < manager->block_count; index++) {
        if (manager->blocks[index].status == FREE_BLOCK && manager->blocks[index].block_size >= request_size)
            return (int)index;
    }

    return -1;
}

static int best_fit(MemoryManager *manager, size_t request_size)
{
    int best_index = -1;

    for (size_t index = 0; index < manager->block_count; index++) {
        if (manager->blocks[index].status == FREE_BLOCK && manager->blocks[index].block_size >= request_size &&
            (best_index < 0 || manager->blocks[index].block_size < manager->blocks[best_index].block_size))
            best_index = (int)index;
    }

    return best_index;
}

static int worst_fit(MemoryManager *manager, size_t request_size)
{
    int worst_index = -1;

    for (size_t index = 0; index < manager->block_count; index++) {
        if (manager->blocks[index].status == FREE_BLOCK && manager->blocks[index].block_size >= request_size &&
            (worst_index < 0 || manager->blocks[index].block_size > manager->blocks[worst_index].block_size))
            worst_index = (int)index;
    }

    return worst_index;
}

static int find_free_block(MemoryManager *manager, size_t request_size)
{
    if (manager->strategy == BEST_FIT) return best_fit(manager, request_size);
    if (manager->strategy == WORST_FIT) return worst_fit(manager, request_size);

    return first_fit(manager, request_size);
}

static int split_block(MemoryManager *manager, size_t block_index, size_t wanted_size)
{
    if (manager->blocks[block_index].block_size > wanted_size) {
        Block leftover = {
            manager->blocks[block_index].start_offset + wanted_size,
            manager->blocks[block_index].block_size - wanted_size,
            0,
            FREE_BLOCK
        };

        if (insert_block(manager, block_index + 1, leftover) != 0) return -1;
    }

    manager->blocks[block_index].block_size = wanted_size;

    return 0;
}

static void merge_free_blocks(MemoryManager *manager)
{
    size_t index = 0;

    while (index + 1 < manager->block_count) {
        if (manager->blocks[index].status == FREE_BLOCK && manager->blocks[index + 1].status == FREE_BLOCK) {
            manager->blocks[index].block_size += manager->blocks[index + 1].block_size;
            remove_block(manager, index + 1);
        } else {
            index++;
        }
    }
}

static size_t total_free_bytes(MemoryManager *manager)
{
    size_t free_bytes = 0;

    for (size_t index = 0; index < manager->block_count; index++) {
        if (manager->blocks[index].status == FREE_BLOCK)
            free_bytes += manager->blocks[index].block_size;
    }

    return free_bytes;
}

static void report_failure(MemoryManager *manager, size_t request_size)
{
    size_t free_bytes = total_free_bytes(manager);

    printf("Failed: no contiguous free block is large enough for %zu bytes.\n", request_size);

    if (free_bytes >= request_size) {
        printf("External fragmentation: %zu bytes are free in total, but only as separate blocks:", free_bytes);

        for (size_t index = 0; index < manager->block_count; index++) {
            if (manager->blocks[index].status == FREE_BLOCK)
                printf(" %zu", manager->blocks[index].block_size);
        }

        printf("\n");
    }
}

static void allocate_memory(MemoryManager *manager)
{
    long requested_size;

    if (read_number("Requested size: ", &requested_size)) return;

    if (requested_size <= 0 || (size_t)requested_size > manager->pool_size) {
        printf("Invalid size.\n");
        return;
    }

    int block_index = find_free_block(manager, requested_size);

    if (block_index < 0) {
        report_failure(manager, requested_size);
        return;
    }

    if (split_block(manager, block_index, requested_size) != 0) {
        printf("Allocation failed: out of metadata memory.\n");
        return;
    }

    manager->blocks[block_index].status = USED_BLOCK;
    manager->blocks[block_index].allocation_id = manager->next_allocation_id++;

    printf("Allocation successful. ID: %d, Start: %zu, Size: %ld\n",
           manager->blocks[block_index].allocation_id, manager->blocks[block_index].start_offset, requested_size);
}

static void release_memory(MemoryManager *manager)
{
    long allocation_id;

    if (read_number("Allocation ID: ", &allocation_id)) return;

    int block_index = find_block_by_id(manager, allocation_id);

    if (block_index < 0) {
        printf("Unknown ID or block already free.\n");
        return;
    }

    manager->blocks[block_index].status = FREE_BLOCK;
    manager->blocks[block_index].allocation_id = 0;
    merge_free_blocks(manager);

    printf("Allocation released.\n");
}

static void resize_allocation(MemoryManager *manager)
{
    long allocation_id, requested_size;

    if (read_number("Allocation ID: ", &allocation_id) || read_number("New size: ", &requested_size)) return;

    int block_index = find_block_by_id(manager, allocation_id);

    if (block_index < 0) {
        printf("Unknown ID or block already free.\n");
        return;
    }

    if (requested_size <= 0 || (size_t)requested_size > manager->pool_size) {
        printf("Invalid size.\n");
        return;
    }

    size_t new_size = requested_size;
    size_t old_size = manager->blocks[block_index].block_size;

    if (new_size <= old_size) {
        if (split_block(manager, block_index, new_size) != 0) {
            printf("Resize failed.\n");
            return;
        }

        merge_free_blocks(manager);
    } else if ((size_t)block_index + 1 < manager->block_count && manager->blocks[block_index + 1].status == FREE_BLOCK &&
               old_size + manager->blocks[block_index + 1].block_size >= new_size) {
        size_t extra_bytes = new_size - old_size;

        manager->blocks[block_index + 1].start_offset += extra_bytes;
        manager->blocks[block_index + 1].block_size -= extra_bytes;
        manager->blocks[block_index].block_size = new_size;

        if (manager->blocks[block_index + 1].block_size == 0) remove_block(manager, block_index + 1);
    } else {
        int target_index = find_free_block(manager, new_size);

        if (target_index < 0 || split_block(manager, target_index, new_size) != 0) {
            printf("Resize failed; original allocation unchanged.\n");
            report_failure(manager, new_size);
            return;
        }

        if (target_index < block_index) block_index++;

        manager->blocks[target_index].status = USED_BLOCK;
        manager->blocks[target_index].allocation_id = (int)allocation_id;

        memcpy(manager->pool + manager->blocks[target_index].start_offset, manager->pool + manager->blocks[block_index].start_offset, old_size);

        manager->blocks[block_index].status = FREE_BLOCK;
        manager->blocks[block_index].allocation_id = 0;
        merge_free_blocks(manager);
    }

    printf("Resize successful.\n");
}

static void display_memory_map(MemoryManager *manager)
{
    printf("Memory Map (%zu bytes):\n", manager->pool_size);

    for (size_t index = 0; index < manager->block_count; index++) {
        Block *block = &manager->blocks[index];

        if (block->status == USED_BLOCK)
            printf("[Allocated #%d: %zu bytes, Start: %zu]\n", block->allocation_id, block->block_size, block->start_offset);
        else
            printf("[Free: %zu bytes, Start: %zu]\n", block->block_size, block->start_offset);
    }
}

static void display_statistics(MemoryManager *manager)
{
    size_t allocated_blocks = 0, free_blocks = 0, largest_free_block = 0;

    for (size_t index = 0; index < manager->block_count; index++) {
        Block *block = &manager->blocks[index];

        if (block->status == USED_BLOCK) {
            allocated_blocks++;
        } else {
            free_blocks++;
            if (block->block_size > largest_free_block) largest_free_block = block->block_size;
        }
    }

    size_t free_bytes = total_free_bytes(manager);

    printf("Total memory: %zu bytes\n", manager->pool_size);
    printf("Allocated: %zu bytes\n", manager->pool_size - free_bytes);
    printf("Free: %zu bytes\n", free_bytes);
    printf("Allocated blocks: %zu\n", allocated_blocks);
    printf("Free blocks: %zu\n", free_blocks);
    printf("Largest free block: %zu bytes\n", largest_free_block);
}

static int choose_strategy(MemoryManager *manager)
{
    long strategy_choice;

    do {
        if (read_number("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ", &strategy_choice))
            return -1;
    } while (strategy_choice < FIRST_FIT || strategy_choice > WORST_FIT);

    manager->strategy = (int)strategy_choice;

    printf("Strategy set to %s Fit.\n",
           strategy_choice == FIRST_FIT ? "First" : strategy_choice == BEST_FIT ? "Best" : "Worst");

    return 0;
}

int main(void)
{
    MemoryManager manager;
    long pool_size_input, menu_choice = 0;

    do {
        if (read_number("Enter memory pool size: ", &pool_size_input))
             return 1;
    } 
    while (init_pool(&manager, pool_size_input) != 0);

    if (choose_strategy(&manager)) return 1;

    while (menu_choice != 7) {
        printf("\nMenu:\n1. Allocate memory\n2. Release memory\n3. Resize allocation\n"
               "4. Display memory map\n5. Display memory statistics\n"
               "6. Change allocation strategy\n7. Exit\n");

        if (read_number("Choice: ", &menu_choice)) break;

        switch (menu_choice) {
            case 1: allocate_memory(&manager); break;
            case 2: release_memory(&manager); break;
            case 3: resize_allocation(&manager); break;
            case 4: display_memory_map(&manager); break;
            case 5: display_statistics(&manager); break;
            case 6: if (choose_strategy(&manager)) menu_choice = 7; break;
            case 7: break;
            default: printf("Invalid choice.\n");
        }
    }

    free(manager.pool);
    free(manager.blocks);

    return 0;
}