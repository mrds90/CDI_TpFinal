#include "test/support/expected_output.h"
#include "src/OSAL/inc/port/support/FreeRTOS_task_simulated.h"
#include "src/OSAL/inc/port/port_task_freertos.h"
#include "src/OSAL/inc/osal_task.h"
#include "src/RealWorld/real_world.h"
#include "/var/lib/gems/2.7.0/gems/ceedling-0.31.1/vendor/unity/src/unity.h"




int32_t RecurrenceFunction(int32_t input);

void test_RecurrenceFunction(void) {



    const int num_samples = sizeof(expected_output) / sizeof(expected_output[0]);





    for (int i = 0; i < num_samples; ++i) {

        int32_t actual_output = RecurrenceFunction(input[i]);

        UnityAssertEqualNumber((UNITY_INT)(UNITY_INT32)((expected_output[i])), (UNITY_INT)(UNITY_INT32)((actual_output)), (

       ((void *)0)

       ), (UNITY_UINT)(46), UNITY_DISPLAY_STYLE_INT32);

    }

}
