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
#include <stdio.h>
#include <string.h>

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define V_TO_MV(x)  ((x) * 1000)
#define N_SAMPLES (1 << 8)
#define TS_MS         5

#define NUM_SIZE 3
#define DEN_SIZE 3

// Coeficientes del numerador en Q15

#define NUM0 Q15_SCALE(1)
#define NUM1 Q15_SCALE(-1.35569551)
#define NUM2 Q15_SCALE(0.42632345)

// Coeficientes del denominador en Q15
#define DEN0 Q15_SCALE(1)
#define DEN1 Q15_SCALE(-1.21587686)
#define DEN2 Q15_SCALE(0.2865048 )

/*========= [PRIVATE DATA TYPES] ===============================================*/



/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

STATIC int32_t PidRecurrenceFunction(int32_t input);

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

STATIC uint16_t input_mv = 0;

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

void CONTROLLER_SquareOpenLoop(void *per) {
    uint8_t period = *((uint8_t *) per);
    static const uint16_t output[2] = {V_TO_MV(2), V_TO_MV(1)};
    static uint8_t out_index = 0;
    osal_tick_t last_enter_to_task = OSAL_TASK_GetTickCount();
    #ifndef TEST
    while (TRUE)
    #endif
    {
        INTERFACE_DACWriteMv(output[out_index]);
        input_mv = INTERFACE_ADCRead();
        out_index = (out_index + 1) & 1;
        OSAL_TASK_DelayUntil(&last_enter_to_task, OSAL_MS_TO_TICKS(period * 1000) / 2);
    }
}

void CONTROLLER_PID(void *per) {
    uint8_t period = *((uint8_t *) per);

    static const uint16_t r[2] = {Q15_SCALE(V_TO_MV(2)) / 3300, Q15_SCALE(V_TO_MV(1)) / 3300};

    static uint8_t r_index = 0;
    static uint32_t count = 0;
    osal_tick_t last_enter_to_task = OSAL_TASK_GetTickCount();

    #ifndef TEST
    while (TRUE)
    #endif
    {
        input_mv = INTERFACE_ADCRead();
        uint32_t input_q15 = (Q15_SCALE(input_mv)) / 3300;
        uint32_t u = PidRecurrenceFunction((2 * r[r_index] - input_q15));
        u = (u * 3300) >> 15;
        INTERFACE_DACWriteMv(u);
        count++;
        if (count >= ((period * 1000 / 2) / TS_MS)) {
            count = 0;
            r_index ^= 1;
        }
        OSAL_TASK_DelayUntil(&last_enter_to_task, OSAL_MS_TO_TICKS(TS_MS));
    }
}


/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

STATIC int32_t PidRecurrenceFunction(int32_t input) {
// Buffers para mantener el estado
    static int32_t input_buffer[NUM_SIZE] = {[0 ... NUM_SIZE - 1] = 0};
    static int32_t output_buffer[DEN_SIZE - 1] = {[0 ... DEN_SIZE - 2] = 0};
    // Desplazar valores en el buffer de entrada
    for (int i = NUM_SIZE - 1; i > 0; --i) {
        input_buffer[i] = input_buffer[i - 1];
    }
    input_buffer[0] = input;

    // Calcular la parte del numerador
    int32_t output = 0;
    output += (NUM0 * input_buffer[0]) >> 15;
    output += (NUM1 * input_buffer[1]) >> 15;
    output += (NUM2 * input_buffer[2]) >> 15;

    // Calcular la parte del denominador
    output -= (DEN1 * output_buffer[0]) >> 15;
    output -= (DEN2 * output_buffer[1]) >> 15;

    // Desplazar valores en el buffer de salida
    for (int i = DEN_SIZE - 2; i > 0; --i) {
        output_buffer[i] = output_buffer[i - 1];
    }
    output_buffer[0] = output;

    return output;
}
/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
