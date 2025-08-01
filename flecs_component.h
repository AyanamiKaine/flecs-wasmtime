// Generated by `wit-bindgen` 0.43.0. DO NOT EDIT!
#ifndef __BINDINGS_FLECS_COMPONENT_H
#define __BINDINGS_FLECS_COMPONENT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// A handle representing a Flecs world instance.
typedef uint64_t exports_local_flecs_flecs_api_world_handle_t;

// A handle representing a Flecs entity.
typedef uint64_t exports_local_flecs_flecs_api_entity_handle_t;

// A handle representing a Flecs component.
typedef uint64_t exports_local_flecs_flecs_api_component_handle_t;

// Exported Functions from `local:flecs/flecs-api@0.1.0`
exports_local_flecs_flecs_api_world_handle_t exports_local_flecs_flecs_api_create_world(void);
void exports_local_flecs_flecs_api_delete_world(exports_local_flecs_flecs_api_world_handle_t world_handle);
exports_local_flecs_flecs_api_entity_handle_t exports_local_flecs_flecs_api_new_entity(exports_local_flecs_flecs_api_world_handle_t world_handle);
void exports_local_flecs_flecs_api_add_component(exports_local_flecs_flecs_api_world_handle_t world_handle, exports_local_flecs_flecs_api_entity_handle_t entity, exports_local_flecs_flecs_api_component_handle_t component);

#ifdef __cplusplus
}
#endif
#endif
