#include "src/OSAL/inc/port/support/FreeRTOS_task_simulated.h"
#include "src/OSAL/inc/port/port_task_freertos.h"
#include "src/OSAL/inc/osal_task.h"
#include "src/RealWorld/real_world.h"
#include "src/Control/control.h"
#include "/var/lib/gems/2.7.0/gems/ceedling-0.31.1/vendor/unity/src/unity.h"
int32_t RecurrenceFunction(int32_t input);



void TaskRealWorld(void *not_used);

void test_SquareOpenLoop(void) {

    CONTROLLER_SquareOpenLoop(10);

    UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT32)((50)), (UNITY_INT)(UNITY_UINT32)((xTaskGetTickCount())), (

   ((void *)0)

   ), (UNITY_UINT)(43), UNITY_DISPLAY_STYLE_UINT32);

    for (uint8_t i = 0; i < 50; i += 5) {

        TaskRealWorld(

                     ((void *)0)

                         );

    }

    CONTROLLER_SquareOpenLoop(10);

    UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT32)((100)), (UNITY_INT)(UNITY_UINT32)((xTaskGetTickCount())), (

   ((void *)0)

   ), (UNITY_UINT)(48), UNITY_DISPLAY_STYLE_UINT32);

}











void vTaskDelay(TickType_t delay_ticks) {}
