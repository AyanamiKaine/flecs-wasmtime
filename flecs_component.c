#include "flecs.h"
#include <flecs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
One major problem we have is to correctly expose all of the flecs api to wasm in essence we have to create a wrapper function for every
function we want to export.
*/

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

