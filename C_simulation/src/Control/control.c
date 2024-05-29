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

/*========= [PRIVATE DATA TYPES] ===============================================*/

typedef struct {
    int32_t Kp;
    int32_t Ti;
    int32_t Td;
    int32_t h;
    int32_t N;
    int32_t b;
} pid_config_t;

typedef struct {
    int32_t pastD;
    int32_t pastY;
    int32_t futureI;
} pid_state_t;


/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

STATIC void PidInit(pid_config_t *config, pid_state_t *state);

STATIC int32_t PidControl(pid_config_t *config, pid_state_t *state, int32_t y, int32_t r);

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
        INTERFACE_DACWrite(output[out_index]);
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
    
    pid_config_t pid_config;
    pid_state_t pid_state;

    pid_config.h = (int32_t)(0.005 * 32768);
    pid_config.Kp = (int32_t)(4.3932085220687425 * 32768);
    pid_config.Ti = (int32_t)((0.12619764736142597 * 32768) * pid_config.h / 32768);
    pid_config.Td = (int32_t)((0.04711088353311812 * 32768) * pid_config.h / 32768);
    pid_config.b = (int32_t)(1.0 * 32768);
    pid_config.N = (int32_t)(20 * 32768);

    PidInit(&pid_config, &pid_state);

    osal_tick_t last_enter_to_task = OSAL_TASK_GetTickCount();

    #ifndef TEST
    while (TRUE)
    #endif
    {
        input_mv = INTERFACE_ADCRead();
        uint32_t input_q15 = (Q15_SCALE(input_mv)) / 3300;
        uint32_t u = PidControl(&pid_config, &pid_state, input_q15, r[r_index]);
        u = (u * 3300) >> 15;
        INTERFACE_DACWrite(u);
        if (count > (period * 1000 / TS_MS)) {
            count = 0;
            r_index ^= 1;
        }
        count++;
        OSAL_TASK_DelayUntil(&last_enter_to_task, OSAL_MS_TO_TICKS(TS_MS));
    }
}

STATIC void PidInit(pid_config_t *config, pid_state_t *state) {
    state->futureI = 0;
    state->pastY = 0;
    state->pastD = 0;
}

int32_t PidControl(pid_config_t *config, pid_state_t *state, int32_t y, int32_t r) {
    int32_t P = 0;
    int32_t D = 0;
    int32_t I = 0;
    int32_t U = 0;

    P = (config->Kp * ((config->b * r) >> 15) - y) >> 15;
    D = ((config->Td * state->pastD - config->N * config->Kp * config->Td * (y - state->pastY) / 32768) / (config->Td + config->N * config->h));
    I = state->futureI;

    U = P + I + D;

    state->pastD = D;
    state->pastY = y;
    state->futureI = I + (config->Kp * config->h / config->Ti * (r - y) / 32768);

    return U;
}

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/

/*========= [INTERRUPT FUNCTION IMPLEMENTATION] ================================*/
