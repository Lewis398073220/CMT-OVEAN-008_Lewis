/**
 ****************************************************************************************
 *
 * @file acc.h
 *
 * @brief Audio Content Control - Header file
 *
 * Copyright (C) RivieraWaves 2019-2021
 *
 ****************************************************************************************
 */

#ifndef ACC_H_
#define ACC_H_

/**
 ****************************************************************************************
 * @defgroup ACC Audio Content Control (ACC)
 * @ingroup GAF
 * @brief Description of Audio Content Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_COMMON Common
 * @ingroup ACC
 * @brief Description of common API for Audio Content Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_ENUM Enumerations
 * @ingroup ACC_COMMON
 * @brief Enumerations for Audio Content Control block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"         // GAF Configuration
#include "gaf.h"             // GAF API

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_ENUM
/// @{

/// Module type values for Audio Content Control block
enum acc_module_type
{
    /// Common module
    ACC_MODULE_COMMON = 0,
    /// Telephone Bearer Service Server module
    ACC_MODULE_TBS = 1,
    /// Telephone Bearer Service Client module
    ACC_MODULE_TBC = 2,
    /// Media Control Service Server module
    ACC_MODULE_MCS = 3,
    /// Media Control Service Client module
    ACC_MODULE_MCC = 4,
    /// Object Transfer Service Server module
    ACC_MODULE_OTS = 5,
    /// Object Transfer Service Client module
    ACC_MODULE_OTC = 6,
    /// Data Transfer Service Client module
    ACC_MODULE_DTC = 7,
    /// Data Transfer Service Client module
    ACC_MODULE_DTS = 8,

    /// Maximum value
    ACC_MODULE_MAX,
};

/// @} ACC_ENUM

#endif // ACC_H_
