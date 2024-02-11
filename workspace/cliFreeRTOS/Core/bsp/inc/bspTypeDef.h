/**
******************************************************************************
* @file    bspTypeDef.h
* @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
* @brief   Header that contains BSP definitions
******************************************************************************
*/

#ifndef  __BSP_TYPE_DEF_H
#define __BSP_TYPE_DEF_H

#include "errno.h"

typedef enum
{
    BSP_NO_ERROR,
    BSP_ERROR_EIO = EIO,
    BSP_ERROR_EINVAL = EINVAL,
} BspError_e;

#endif
