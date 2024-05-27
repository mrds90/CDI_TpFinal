/**
 * @file real_world.c
 * @author Marcos Dominguez
 *
 * @brief Simulate a physical plant
 *
 * @version 0.1
 * @date 2024-05-27
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "reat_world.h"
#include "osal_task.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [LOCAL VARIABLES] ==================================================*/

/*========= [STATE FUNCTION POINTERS] ==========================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATION] ===================================*/

void REAL_WORLD_Init(void);

void REAL_WORLD_Input(uint32_t value) {}

uint32_t REAL_WORLD_Output(void);

/*========= [PRIVATE FUNCTION IMPLEMENTATION] ==================================*/


#define NUM_SIZE 2
#define DEN_SIZE 3

// Coeficientes del numerador en Q15
#define NUM0 Q15_SCALE(0.14768561)
#define NUM1 Q15_SCALE(0.07379223)

// Coeficientes del denominador en Q15
#define DEN0 Q15_SCALE(1.0)
#define DEN1 Q15_SCALE(-0.8996254943415778)
#define DEN2 Q15_SCALE(0.12110333239232968)

// Buffers para mantener el estado
static int32_t input_buffer[NUM_SIZE] = {[0 ... NUM_SIZE - 1] = 0};
static int32_t output_buffer[DEN_SIZE - 1] = {[0 ... DEN_SIZE - 2] = 0};

// Función de recurrencia en tiempo real
int32_t funcion_recurrencia(int32_t input) {
    // Desplazar valores en el buffer de entrada
    for (int i = NUM_SIZE - 1; i > 0; --i) {
        input_buffer[i] = input_buffer[i - 1];
    }
    input_buffer[0] = input;

    // Calcular la parte del numerador
    int32_t output = 0;
    output += (NUM0 * input_buffer[0]) >> 15;
    output += (NUM1 * input_buffer[1]) >> 15;

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
