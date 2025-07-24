#include "flecs.h"
#include <flecs.h>
// Implementation for: export create-world: func() -> world-handle;
uint64_t exports_flecs_world_create_world() {
    // ecs_init creates and returns a new world.
    // We cast the pointer to a 64-bit integer to pass it across the WASM boundary.
    ecs_world_t* world = ecs_init();
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

