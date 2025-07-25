#include "flecs.h"
#include <flecs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
One major problem we have is to correctly expose all of the flecs api to wasm in essence we have to create a wrapper function for every
function we want to export. Maybe this can be generated?
*/

typedef struct {
    double x, y;
} Position, Velocity;

void Move(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    const Velocity *v = ecs_field(it, Velocity, 1);

    for (int i = 0; i < it->count; i ++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
        printf("%s: {%f, %f}\n", ecs_get_name(it->world, it->entities[i]), 
            p[i].x, p[i].y);
    }
}

/*EXAMPLE TO CHECK IF IT ACTUALLY WORKS

This also give me a create idea for a test suite written in C# that uses the wasmtime to run tests for every function exported! Also to see if all examples run the flecs defines!
We could also implement some of the flecs tests.
*/

// Implementation for: export create-world: func() -> world-handle;
uint64_t exports_local_flecs_flecs_world_ecs_init(uint64_t world_handle) {
    // This function seems ill-defined in the WIT. ecs_init creates a world, it doesn't take a handle.
    // Let's assume the intent is just to create a standard world.
    printf("DEBUG: Creating a standard world with ecs_init().\n");
    ecs_world_t* world = ecs_init();
    return (uint64_t)world;
}

/*
To invoke this function in wasmtime-cli say:
wasmtime run --invoke 'local:flecs/flecs-world#ecs-mini' flecs.wasm
*/

// Implementation for: export ecs-mini: func() -> u64;
uint64_t exports_local_flecs_flecs_world_ecs_mini() {
    printf("DEBUG: Creating a minimal world with ecs_mini().\n");
    
    // ecs_mini() creates a world without automatically importing any addons.
    ecs_world_t* world = ecs_mini();

    if (!world) {
        printf("ERROR: Failed to create Flecs world with ecs_mini().\n");
        return 0;
    }

    printf("DEBUG: World created. Manually importing safe addons...\n");

    // Manually import the addons we need.
    ECS_IMPORT(world, FlecsSystem);
    ECS_IMPORT(world, FlecsPipeline);
    ECS_IMPORT(world, FlecsTimer);
    
    printf("SUCCESS: Flecs world is initialized and ready!\n");
    
    // Your example setup code:
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_system(world, {
        .entity = ecs_entity(world, { .name = "Move" }),
        .query.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Velocity), .inout = EcsIn }
        },
        .callback = Move
    });

    ecs_entity_t e1 = ecs_entity(world, { .name = "e1" });
    ecs_set(world, e1, Position, {10, 20});
    ecs_set(world, e1, Velocity, {1, 2});

    ecs_entity_t e2 = ecs_entity(world, { .name = "e2" });
    ecs_set(world, e2, Position, {10, 20});
    ecs_set(world, e2, Velocity, {3, 4});

    // Cast the pointer to a 64-bit integer to pass it across the WASM boundary.
    return (uint64_t)world;
}

// Implementation for: export ecs-fini: func(world-handle: u64) -> s64;
int64_t exports_local_flecs_flecs_world_ecs_fini(uint64_t world_handle) {
    ecs_world_t* world = (ecs_world_t*)world_handle;
    return ecs_fini(world);
}

// Implementation for: export ecs-is-fini: func(world-handle: u64) -> bool;
bool exports_local_flecs_flecs_world_ecs_is_fini(uint64_t world_handle) {
    ecs_world_t* world = (ecs_world_t*)world_handle;
    return ecs_is_fini(world);
}

// Implementation for: export ecs-new: func(world-handle: u64) -> u64;
uint64_t exports_local_flecs_flecs_entity_ecs_new(uint64_t world_handle) {
    ecs_world_t* world = (ecs_world_t*)world_handle;
    ecs_entity_t entity = ecs_new(world);
    return (uint64_t)entity;
}

// Implementation for: export ecs-new-low-id: func(world-handle: u64) -> u64;
uint64_t exports_local_flecs_flecs_entity_ecs_new_low_id(uint64_t world_handle) {
    ecs_world_t* world = (ecs_world_t*)world_handle;
    ecs_entity_t entity = ecs_new_low_id(world);
    return (uint64_t)entity;
}