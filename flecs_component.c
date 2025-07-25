#include "flecs.h"
#include <flecs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a type for clarity that matches Flecs's internal size type.
// Flecs uses ecs_size_t, which is typically a signed int.
typedef int flecs_os_api_size_t;

// --- Wrapper Functions ---

// Wrapper for malloc
static
void* wasi_malloc_wrapper(flecs_os_api_size_t size) {
    // Cast the 'int' size to 'size_t' for the real malloc
    return malloc((size_t)size);
}

// Wrapper for realloc
static
void* wasi_realloc_wrapper(void *ptr, flecs_os_api_size_t size) {
    // Cast the 'int' size to 'size_t' for the real realloc
    return realloc(ptr, (size_t)size);
}

// Wrapper for calloc. Flecs's calloc_ expects one argument (total size),
// while standard calloc takes two (count, member_size). We can simulate
// it with malloc + memset to guarantee zero-initialized memory.
static
void* wasi_calloc_wrapper(flecs_os_api_size_t size) {
    if (size == 0) {
        return NULL;
    }
    // Cast the 'int' size to 'size_t' for malloc
    void *ptr = malloc((size_t)size);
    if (ptr) {
        // calloc guarantees the memory is set to zero
        memset(ptr, 0, (size_t)size);
    }
    return ptr;
}

// Wrapper for free (the signature already matches)
static
void wasi_free_wrapper(void *ptr) {
    free(ptr);
}


// --- Your Initializer ---

void initialize_flecs_os_api_for_wasi() {
    // This is the correct pattern. Get the defaults first.
    ecs_os_set_api_defaults();
    
    // Then get a copy of the API struct to modify
    ecs_os_api_t os_api = ecs_os_get_api();
    
    // Now, assign your type-safe wrappers to the API function pointers
    os_api.malloc_ = wasi_malloc_wrapper;
    os_api.realloc_ = wasi_realloc_wrapper;
    os_api.calloc_ = wasi_calloc_wrapper;
    os_api.free_ = wasi_free_wrapper; // Good practice to wrap this too
    os_api.get_time_ = NULL; // Keep this as is if you don't need time

    // Finally, set the modified API back to Flecs
    ecs_os_set_api(&os_api);
}
// Implementation for: export create-world: func() -> world-handle;
uint64_t exports_flecs_world_create_world() {
    printf("DEBUG: Creating a minimal world with ecs_mini().\n");
    
    // ecs_mini() creates a world without automatically importing any addons.
    // This avoids the problematic flecs.meta addon.
    ecs_world_t* world = ecs_mini();

    if (!world) {
        printf("ERROR: Failed to create Flecs world with ecs_mini().\n");
        return 0;
    }

    printf("DEBUG: World created. Manually importing safe addons...\n");

    // Manually import the addons we need. These are known to be safe.
    ECS_IMPORT(world, FlecsSystem);
    ECS_IMPORT(world, FlecsPipeline);
    ECS_IMPORT(world, FlecsTimer);

    printf("SUCCESS: Flecs world is fully initialized and ready!\n");
    
    // Cast the pointer to a 64-bit integer to pass it across the WASM boundary.
    return (uint64_t)world;
}

// Implementation for: export delete-world: func(world: world-handle);
void exports_flecs_world_delete_world(uint64_t world_handle) {
    // ecs_fini cleans up the world.
    // We cast the handle back to a pointer.
    ecs_world_t* world = (ecs_world_t*)world_handle;
    ecs_fini(world);
}

// Implementation for: export new-entity: func(world: world-handle) -> entity-handle;
uint64_t exports_flecs_world_new_entity(uint64_t world_handle) {
    // ecs_new_id creates a new entity.
    ecs_world_t* world = (ecs_world_t*)world_handle;
    ecs_entity_t entity = ecs_new(world);
    return (uint64_t)entity;
}

// Implementation for: export add-component: func(world: world-handle, entity: entity-handle, component: component-handle);
void exports_flecs_world_add_component(uint64_t world_handle, uint64_t entity, uint64_t component) {
    // ecs_add_id adds a component (represented by its ID) to an entity.
    ecs_world_t* world = (ecs_world_t*)world_handle;
    ecs_add_id(world, (ecs_entity_t)entity, (ecs_id_t)component);
}

