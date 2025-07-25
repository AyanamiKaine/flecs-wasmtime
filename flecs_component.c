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
    ECS_IMPORT(world, FlecsDoc);
    printf("SUCCESS: Flecs world is fully initialized and ready!\n");
    

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_entity_t move = ecs_system(world, {
        // Systems are entities, and by initializing the .entity field we can
        // set some additional properties for the system like a name. While this
        // is not mandatory, it makes a system easier to find in tools like the
        // explorer (https://www.flecs.dev/explorer/).
        .entity = ecs_entity(world, {
            .name = "Move" 
        }),
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

    // This entity will not match as it does not have Position, Velocity
    ecs_entity_t e3 = ecs_entity(world, { .name = "e3" });
    ecs_set(world, e3, Position, {10, 20});

    // Run the system
    ecs_run(world, move, 0.0f, NULL);

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

