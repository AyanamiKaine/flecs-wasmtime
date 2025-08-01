#include <runtime_nested_component.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    ecs_world_t *ecs = ecs_init_w_args(argc, argv);

    // Create components for types that aren't known at compile time
    ecs_entity_t Point = ecs_struct(ecs, {
        .members = {
            { .name = "x", .type = ecs_id(ecs_f32_t) }, // builtin float type
            { .name = "y", .type = ecs_id(ecs_f32_t) }
        }
    });

    ecs_entity_t Line = ecs_struct(ecs, {
        .members = {
            { .name = "start", .type = Point },
            { .name = "stop", .type = Point }
        }
    });

    // Create entity, set value of position using reflection API
    ecs_entity_t ent = ecs_entity(ecs, { .name = "ent" });

    // Get type info because we need to pass in the size
    const ecs_type_info_t *ti = ecs_get_type_info(ecs, Line);
    void *ptr = ecs_ensure_id(ecs, ent, Line, (size_t)ti->size);

    ecs_meta_cursor_t cur = ecs_meta_cursor(ecs, Line, ptr);
    ecs_meta_push(&cur);          // {
    ecs_meta_push(&cur);          //   {
    ecs_meta_set_float(&cur, 10); //     10
    ecs_meta_next(&cur);          //     ,
    ecs_meta_set_float(&cur, 20); //     20
    ecs_meta_pop(&cur);           //   }
    ecs_meta_next(&cur);          //   ,
    ecs_meta_push(&cur);          //   {
    ecs_meta_set_float(&cur, 30); //     30
    ecs_meta_next(&cur);          //     ,
    ecs_meta_set_float(&cur, 40); //     40
    ecs_meta_pop(&cur);           //   }
    ecs_meta_pop(&cur);           // }

    // Convert component to string
    char *str = ecs_ptr_to_expr(ecs, Line, ptr);
    printf("%s\n", str); // {start: {x: 10.00, y: 20.00}, stop: {x: 30.00, y: 40.00}}
    ecs_os_free(str);

    ecs_fini(ecs);
}
