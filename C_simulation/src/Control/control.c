/**
 * @file control.c
 * @author Marcos Dominguez
 *
 * @brief Controller
 *
 * @version 0.1
 * @date 2024-05-27
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "control.h"
#include "real_world.h"
#include "osal_task.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define FREC_TO_MS(frequency) (1000000 / (frequency)) / 1000

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

STATIC uint32_t output = 0;

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

void CONTROLLER_SquareOpenLoop(uint8_t frequency) {
    osal_tick_t last_enter_to_task = OSAL_TASK_GetTickCount();

    #ifndef TEST
    while (TRUE)
    #endif
    {
        output ^= Q15_SCALE(1);
        REAL_WORLD_Input(output);
        OSAL_TASK_DelayUntil(&last_enter_to_task, OSAL_MS_TO_TICKS(FREC_TO_MS(frequency))/2);
    }
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
