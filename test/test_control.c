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
#define REAL_NUM_SIZE 2
#define REAL_DEN_SIZE 3

#define TEST_TIME (PERIODO_SQUARE * 1000 * 4)
/*========= [PRIVATE DATA TYPES] ===============================================*/

typedef struct {
    int32_t input;
    int32_t output;
    int32_t input_buffer[REAL_NUM_SIZE];
    int32_t output_buffer[REAL_DEN_SIZE];
} real_world_t;

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

void CONTROLLER_SquareOpenLoop(void *per);

void CONTROLLER_PID(void *per);

int32_t RecurrenceFunction(int32_t input);

void TaskRealWorld(void *not_used);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [TEST FUNCTION IMPLEMENTATION] ===================================*/

void test_SquareOpenLoop(void) {
    REAL_WORLD_Reset();
    so_tick_count = 0;
    static osal_tick_t tick_count = 0;
    uint8_t period = PERIODO_SQUARE;
    TESTED_VARIABLE real_world_t real_world;
    TESTED_VARIABLE uint16_t input_mv;
    for (uint8_t j = 0; j < 5; j++) {
        if (tick_count != 0) {
            TEST_ASSERT_EQUAL_UINT16((square_expected_output[(tick_count - 1) / 5] * ADC_MAX_MV) >> 15, input_mv);
        }
        for (osal_tick_t i = tick_count; i < tick_count + period * 1000 / 2; i += 5) {
            CONTROLLER_SquareOpenLoop((void *)(&period));
            TEST_ASSERT_EQUAL_INT32(square_input[i / 5], real_world.input);
            TaskRealWorld(NULL);
            TEST_ASSERT_EQUAL_INT32(square_expected_output[i / 5], REAL_WORLD_Output());
        }
        tick_count += period * 1000 / 2;
        TEST_ASSERT_EQUAL_UINT32(tick_count, OSAL_TASK_GetTickCount());
    }
}

void test_PID(void) {
    REAL_WORLD_Reset();
    so_tick_count = 0;
    static osal_tick_t tick_count = 0;
    uint8_t period = PERIODO_SQUARE;
    TESTED_VARIABLE real_world_t real_world;
    TESTED_VARIABLE uint16_t input_mv;
    for (osal_tick_t i = tick_count; i < tick_count + TEST_TIME; i += 5) {
        if (tick_count != 0) {
            TEST_ASSERT_EQUAL_UINT16((controlled_expected_output[(tick_count - 1) / 5] * ADC_MAX_MV) >> 15, input_mv);
        }
        CONTROLLER_PID((void *)(&period));
        TEST_ASSERT_INT32_WITHIN(20, pid_expected_output[i / 5], real_world.input);
        TaskRealWorld(NULL);
        TEST_ASSERT_INT32_WITHIN(20, controlled_expected_output[i / 5], REAL_WORLD_Output());
    }
    tick_count += TEST_TIME;
    TEST_ASSERT_EQUAL_UINT32(tick_count, OSAL_TASK_GetTickCount());
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/

void vTaskDelay(TickType_t delay_ticks) {}
