/**
 * @file test_template.c
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
#include "real_world.h"
#include "osal_task.h"
#include "port_task_freertos.h"
#include "FreeRTOS_task_simulated.h"
#include "expected_output.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

int32_t RecurrenceFunction(int32_t input);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [TEST FUNCTION IMPLEMENTATION] ===================================*/

void test_RecurrenceFunction(void) {
        // Definir el tamaño de los arreglos de entrada y salida esperados
    const int num_samples = sizeof(expected_output) / sizeof(expected_output[0]);

    // Realizar el test
    for (int i = 0; i < num_samples; ++i) {
        int32_t actual_output = RecurrenceFunction(input[i]);
        TEST_ASSERT_EQUAL_INT32(expected_output[i], actual_output);
    }
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
