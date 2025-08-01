#include <core.h>
#include <stdlib.h>

void Error_setup(void) {
    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = test_abort;
    ecs_os_set_api(&os_api);
    ecs_log_set_level(-5);
}

void Error_abort(void) {
    test_expect_abort();
    ecs_abort(ECS_INTERNAL_ERROR, NULL);
}

void Error_abort_w_param(void) {
    test_expect_abort();
    ecs_abort(ECS_INTERNAL_ERROR, "some parameter");
}

static bool my_abort_called = false;
static
void my_abort(void) {
    my_abort_called = true;
}

void Error_override_abort(void) {
    /* The real reason this tests exists is to achieve 100% coverage. Without
     * this test, the last line of the 'abort' function would never be covered
     * because abort always exits before it gets there. */

    /* hack, because the setup already set the OS API */
    ecs_os_api.abort_ = my_abort;
    ecs_os_abort();
    test_assert(my_abort_called == true);
}

void Error_assert_true(void) {
    ecs_assert(true, ECS_INTERNAL_ERROR, NULL);

    /* Assert should not trigger */
    test_assert(true);
}

void Error_assert_false(void) {
    test_expect_abort();
    ecs_assert(false, ECS_INTERNAL_ERROR, NULL);

    /* Assert should not trigger */
    test_assert(false);
}

void Error_assert_false_w_param(void) {
    test_expect_abort();
    ecs_assert(false, ECS_INTERNAL_ERROR, "some parameter");

    /* Assert should not trigger */
    test_assert(false);
}

void Error_error_codes(void) {
    test_assert(ecs_strerror(ECS_INVALID_PARAMETER) != NULL);
    test_assert(ecs_strerror(ECS_INTERNAL_ERROR) != NULL);
    test_assert(ecs_strerror(ECS_ALREADY_DEFINED) != NULL);
    test_assert(ecs_strerror(ECS_INVALID_COMPONENT_SIZE) != NULL);
    test_assert(ecs_strerror(ECS_INVALID_COMPONENT_ALIGNMENT) != NULL);
    test_assert(ecs_strerror(ECS_OUT_OF_MEMORY) != NULL);
    test_assert(ecs_strerror(ECS_MODULE_UNDEFINED) != NULL);
    test_assert(ecs_strerror(ECS_COLUMN_INDEX_OUT_OF_RANGE) != NULL);
    test_assert(ecs_strerror(ECS_COLUMN_IS_NOT_SHARED) != NULL);
    test_assert(ecs_strerror(ECS_COLUMN_IS_SHARED) != NULL);
    test_assert(ecs_strerror(ECS_COLUMN_TYPE_MISMATCH) != NULL);
    test_assert(ecs_strerror(ECS_INVALID_WHILE_READONLY) != NULL);
    test_assert(ecs_strerror(ECS_INVALID_FROM_WORKER) != NULL);
    test_assert(ecs_strerror(ECS_OUT_OF_RANGE) != NULL);
    test_assert(ecs_strerror(ECS_OPERATION_FAILED) != NULL);
    test_assert(ecs_strerror(ECS_MISSING_OS_API) != NULL);
    test_assert(ecs_strerror(ECS_UNSUPPORTED) != NULL);
    test_assert(ecs_strerror(ECS_ACCESS_VIOLATION) != NULL);
}

void Error_log_dbg(void) {
    ecs_dbg("test debug message");
    test_assert(true);
}

void Error_log_log(void) {
    ecs_trace("test trace message");
    test_assert(true);
}

void Error_log_warning(void) {
    ecs_warn("test warning message");
    test_assert(true);
}

void Error_log_error(void) {
    ecs_err("test error message");
    test_assert(true);
}

void Error_last_error(void) {
    ecs_os_api.log_last_error_ = 10;
    int err = ecs_log_last_error();
    test_int(10, err);
    test_int(0, ecs_log_last_error());
}

void Error_set_log_level_return(void) {
    ecs_log_set_level(1);
    int prev = ecs_log_set_level(-1);
    test_int(prev, 1);
}
