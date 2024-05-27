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
#include "interface.h"
#include "osal_task.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define V_TO_MV(x)  ((x)*1000)

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

STATIC uint16_t input_mv = 0;

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

void CONTROLLER_SquareOpenLoop(void *per) {
    uint8_t period = *((uint8_t*) per);
    static const uint16_t output[2] = {V_TO_MV(2), V_TO_MV(1)};
    static uint8_t out_index = 0;
    osal_tick_t last_enter_to_task = OSAL_TASK_GetTickCount();
    #ifndef TEST
    while (TRUE)
    #endif
    {
        INTERFACE_DACWrite(output[out_index]);
        input_mv = INTERFACE_ADCRead();
        out_index = (out_index + 1) & 1;
        OSAL_TASK_DelayUntil(&last_enter_to_task, OSAL_MS_TO_TICKS(period*1000) / 2);
    }
}



/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
