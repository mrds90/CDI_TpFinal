/**
 * @file control.h
 * @author Marcos Dominguez
 *
 * @brief Controller
 *
 * @version 0.1
 * @date 2024-05-27
 */


#ifndef _INC_TEMPLATE_H
#define _INC_TEMPLATE_H

#ifdef  __cplusplus
extern "C" {
#endif

/*========= [DEPENDENCIES] =====================================================*/

#include "data_types.h"
#include "utils.h"

/*========= [PUBLIC MACRO AND CONSTANTS] =======================================*/

#define PERIODO_SQUARE 8
/*========= [PUBLIC DATA TYPE] =================================================*/

/*========= [PUBLIC FUNCTION DECLARATIONS] =====================================*/

void CONTROLLER_SquareOpenLoop(void *freq);

#ifdef  __cplusplus
}

#endif

#endif  /* _INC_TEMPLATE_H */
