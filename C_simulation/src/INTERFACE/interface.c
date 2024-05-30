/**
 * @file interface.c
 * @author Marcos Dominguez
 *
 * @brief Module for interfacing DAC and ADC with Q15 conversion
 *
 * @version 0.1
 * @date 2024-05-27
 */

/*========= [DEPENDENCIES] =====================================================*/

#include "interface.h"
#include "real_world.h"

/*========= [PRIVATE MACROS AND CONSTANTS] =====================================*/

#define DAC_MAX_MV 3300
#define ADC_MAX_MV 3300

/*========= [PRIVATE DATA TYPES] ===============================================*/

/*========= [TASK DECLARATIONS] ================================================*/

/*========= [PRIVATE FUNCTION DECLARATIONS] ====================================*/

/*========= [INTERRUPT FUNCTION DECLARATIONS] ==================================*/

/*========= [PUBLIC FUNCTION IMPLEMENTATIONS] ==================================*/

/**
 * @brief Write to the DAC with a value in millivolts.
 *
 * @param output_dac_mv Value to write to the DAC in millivolts.
 */
void INTERFACE_DACWriteMv(uint16_t output_dac_mv) {
    // Convert millivolts to Q15
    int32_t value_q15 = (Q15_SCALE(output_dac_mv)) / DAC_MAX_MV; // 9929  19859
    REAL_WORLD_Input(value_q15);
}

void INTERFACE_DACWrite(uint32_t q15) {
    REAL_WORLD_Input(q15);
}

/**
 * @brief Read from the ADC and return the value in millivolts.
 *
 * @return uint16_t Value read from the ADC in millivolts.
 */
uint16_t INTERFACE_ADCRead(void) {
    // Read Q15 value from ADC
    int32_t value_q15 = REAL_WORLD_Output();
    // Convert Q15 to millivolts
    uint16_t input_adc_mv = (value_q15 * ADC_MAX_MV) >> 15;

    return input_adc_mv;
}
