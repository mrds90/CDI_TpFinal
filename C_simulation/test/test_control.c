/**
 * @file test_control.c
 * @author Marcos Dominguez
 * @brief Test module
 * @version 01
 * @date 2023-11-08
 *
 * @copyright Copyright (c) 2023
 *
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "unity.h"

#include "interface.h"
#include "control.h"
#include "real_world.h"

#include "osal_task.h"
#include "port_task_freertos.h"
#include "FreeRTOS_task_simulated.h"
#include "expected_output.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define ADC_MAX_MV 3300

/*========= [PRIVATE DATA TYPES] ===============================================*/

typedef struct {
    int32_t input;
    int32_t output;
} real_world_t;

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

int32_t RecurrenceFunction(int32_t input);

void TaskRealWorld(void *not_used);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [TEST FUNCTION IMPLEMENTATION] ===================================*/

void test_SquareOpenLoop(void) {
    static osal_tick_t tick_count = 0;
    uint8_t freq = 10;
    TESTED_VARIABLE real_world_t real_world;
    TESTED_VARIABLE uint16_t input_mv;
    for (uint8_t j = 0; j < 5; j++) {
        CONTROLLER_SquareOpenLoop((void *)(&freq));
        if (tick_count != 0) {
            TEST_ASSERT_EQUAL_UINT16((square_expected_output[(tick_count - 1) / 5] * ADC_MAX_MV) >> 15, input_mv);
        }
        for (uint8_t i = tick_count; i < tick_count + 50; i += 5) {
            TEST_ASSERT_EQUAL_INT32(square_input[i / 5], real_world.input);
            TaskRealWorld(NULL);
            TEST_ASSERT_EQUAL_INT32(square_expected_output[i / 5], REAL_WORLD_Output());
        }
        tick_count += 50;
        TEST_ASSERT_EQUAL_UINT32(tick_count, OSAL_TASK_GetTickCount());
    }
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/

void vTaskDelay(TickType_t delay_ticks) {}
