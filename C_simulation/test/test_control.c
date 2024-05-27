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
#include "control.h"
#include "real_world.h"
#include "osal_task.h"
#include "port_task_freertos.h"
#include "FreeRTOS_task_simulated.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

int32_t RecurrenceFunction(int32_t input);

void TaskRealWorld(void *not_used);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [TEST FUNCTION IMPLEMENTATION] ===================================*/

void test_SquareOpenLoop(void) {
    CONTROLLER_SquareOpenLoop(10);
    TEST_ASSERT_EQUAL_UINT32(50, OSAL_TASK_GetTickCount());
    for (uint8_t i = 0; i < 50; i += 5) {
        TaskRealWorld(NULL);
    }
    CONTROLLER_SquareOpenLoop(10);
    TEST_ASSERT_EQUAL_UINT32(100, OSAL_TASK_GetTickCount());
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/

void vTaskDelay(TickType_t delay_ticks) {}
