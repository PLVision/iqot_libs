/*=============================================================================
*
* The file is part of IQoT project which release under Apache 2.0 license.
* See the License file in the root of project.
* _____________________________________________________________________________
* Copyright (c) 2018 PLVision sp. z o.o. All Rights Reserved.
*
* NOTICE: All information contained herein is, and remains the property of
* PLVision sp. z o.o. and its suppliers, if any. The intellectual and technical
* concepts contained herein are proprietary to PLVision sp. z o.o. and its
* suppliers, and are protected by trade secret or copyright law. Dissemination
* of this information or reproduction of this material is strictly forbidden
* unless prior written permission is obtained from PLVision.
*
=============================================================================*/

#ifndef IQOT_LIB_API_H
/** @cond GUARD_MACRO */
#define IQOT_LIB_API_H
/** @endcond */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/******************************************************
 * DEFINITIONS
 ******************************************************/

#ifdef __GNUC__
#define __NOT_OPTIMIZE__ __attribute__((noinline, optimize("O0")))
#else  // __GNUC__
#define __NOT_OPTIMIZE__
#warning Please, implement some optimization preventing for this platform!
#endif  // __GNUC__

#define UNUSED(x) ((void)x)

#define SENSOR_NAME_MAX (16)
#define SENSOR_COUNT_MAX (16)

#define ACTION_NAME_MAX (16)
#define ACTION_COUNT_MAX (16)
#define ACTION_ARGUMENT_COUNT_MAX (4)
#define ACTION_ARGUMENT_NAME_MAX (8)
#define ACTION_ARGUMENT_STRING_MAX (1024)

#define TRANSPORT_COUNT_MAX (1)

#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 1

#ifndef CONFIG_IQOT_LIB_LOG_ENABLE
#define CONFIG_IQOT_LIB_LOG_ENABLE 0
#endif  // CONFIG_IQOT_LIB_LOG_ENABLE

#ifndef CONFIG_IQOT_LOG_COLORS_ENABLE
#define CONFIG_IQOT_LOG_COLORS_ENABLE 0
#endif  // CONFIG_IQOT_LOG_COLORS_ENABLE

#if defined(CONFIG_IQOT_LOG_COLORS_ENABLE) && (CONFIG_IQOT_LOG_COLORS_ENABLE + 0) == 1
#define __LOG_COLOR_BLACK "30"
#define __LOG_COLOR_RED "31"
#define __LOG_COLOR_GREEN "32"
#define __LOG_COLOR_BROWN "33"
#define __LOG_COLOR_BLUE "34"
#define __LOG_COLOR_PURPLE "35"
#define __LOG_COLOR_CYAN "36"
#define __LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define __LOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define __LOG_RESET_COLOR "\033[0m"
#define __LOG_COLOR_E __LOG_COLOR(__LOG_COLOR_RED)
#define __LOG_COLOR_W __LOG_COLOR(__LOG_COLOR_BROWN)
#define __LOG_COLOR_I
#else  // defined(CONFIG_IQOT_LOG_COLORS_ENABLE) && (CONFIG_IQOT_LOG_COLORS_ENABLE + 0) == 1
#define __LOG_COLOR_E
#define __LOG_COLOR_W
#define __LOG_COLOR_I
#define __LOG_RESET_COLOR
#endif  // defined(CONFIG_IQOT_LOG_COLORS_ENABLE) && (CONFIG_IQOT_LOG_COLORS_ENABLE + 0) == 1

#define __LOG_FORMAT(letter, _format_)                                       \
  __LOG_COLOR_##letter "%d: " __BASE_FILE_NAME__                             \
                       ":" S(__LINE__) ": " _format_ __LOG_RESET_COLOR "\n", \
      (xTaskGetTickCount() * portTICK_PERIOD_MS)

#if defined(CONFIG_IQOT_LIB_LOG_ENABLE) && (CONFIG_IQOT_LIB_LOG_ENABLE + 0) == 1

#define __LOG_FUNC printf

#define _LOG(_level_, _format_, ...)                        \
  {                                                         \
    if (LOG_LEVEL_ERROR == _level_) {                       \
      __LOG_FUNC(__LOG_FORMAT(E, _format_), ##__VA_ARGS__); \
    } else if (LOG_LEVEL_WARNING == _level_) {              \
      __LOG_FUNC(__LOG_FORMAT(W, _format_), ##__VA_ARGS__); \
    } else if (LOG_LEVEL_INFO == _level_) {                 \
      __LOG_FUNC(__LOG_FORMAT(I, _format_), ##__VA_ARGS__); \
    }                                                       \
  }
#else  // defined(CONFIG_IQOT_LIB_LOG_ENABLE) && (CONFIG_IQOT_LIB_LOG_ENABLE + 0) == 1
#define _LOG(...)
#endif  // defined(CONFIG_IQOT_LIB_LOG_ENABLE) && (CONFIG_IQOT_LIB_LOG_ENABLE + 0) == 1

#define TRY_TAKE_SEMAPHORE(_sem_, _delay_, _ret_, _retv_)            \
  {                                                                  \
    BaseType_t res = xSemaphoreTake(_sem_, _delay_);                 \
    if (pdFALSE == res) {                                            \
      TaskHandle_t holder = xSemaphoreGetMutexHolder(_sem_);         \
      _LOG(LOG_LEVEL_WARNING, "semaphore taking failed, holder: %s", \
           pcTaskGetTaskName(holder));                               \
      UNUSED(holder);                                                \
      if (false != _ret_) {                                          \
        return _retv_;                                               \
      }                                                              \
    }                                                                \
  }

/**
 * @brief Errors for IQOT Library.
 */
typedef enum iqot_error {
  IQOT_ERROR_OK,
  IQOT_ERROR_NULL,
  IQOT_ERROR_INVALID_PARAMETER,
  IQOT_ERROR_NOT_ENOUGH_MEMORY,
  IQOT_ERROR_ALREADY_INITED,
  IQOT_ERROR_INTERNAL_ERROR,
  IQOT_ERROR_WRONG_STATE,
  /* rest ERRORS list */

  _IQOT_ERROR_COUNT
} iqot_error_t;

/**
 * @brief Type of value for sensor.
 */
typedef enum iqot_data_type {
  IQOT_DATA_TYPE_INT,
  IQOT_DATA_TYPE_FLOAT,
  IQOT_DATA_TYPE_BOOL,
  IQOT_DATA_TYPE_STRING, /* Not supported. */
  IQOT_DATA_TYPE_INVALID,

  _IQOT_DATA_TYPE_COUNT = IQOT_DATA_TYPE_INVALID
} iqot_data_type_t;

typedef enum _action_result_code_t {
  ACTION_RESULT_CODE_SUCCESS = 0,
  ACTION_RESULT_CODE_ERROR = 1,
  ACTION_RESULT_CODE_BAD_MESSAGE = 2,
  ACTION_RESULT_CODE_METHOD_NOT_FOUND = 3,
  ACTION_RESULT_CODE_NO_RESPONSE = 4,

  _ACTION_RESULT_CODE_COUNT
} action_result_code_t;

/**
 * @brief Transport connection state.
 */
typedef enum iqot_connection_state {
  IQOT_CONNECTION_STATE_DISCONNECTED,
  IQOT_CONNECTION_STATE_CONNECTED,

  _IQOT_CONNECTION_STATE_COUNT
} iqot_connection_state_t;

/**
 * @brief Type for sensor's data and action arguments.
 */
typedef union iqot_data {
  int32_t int32_value;
  float float_value;
  bool bool_value;
  const char* str_value;
} iqot_data_t;

/**
 * @brief Struct with configuration parameters.
 */
typedef struct iqot_config {
  uint32_t telemetry_interval_ms;
  size_t queue_size_tx;
  size_t queue_size_rx;
  uint32_t thread_priority_tx;
  uint32_t thread_priority_rx;
} iqot_config_t;

typedef struct sensor_data {
  uint32_t sensor_id;
  iqot_data_t value;
} sensor_data_t;

typedef struct action_result {
  int action_id;
  action_result_code_t rc;
} action_result_t;

typedef struct init_msg_info {
  size_t transport_id;
} init_msg_info_t;

typedef struct buffer {
  void *ptr;
  size_t len;
} buffer_t;

/******************************************************************************
 * @brief Callback which library call when TX data available.
 *
 * It is synchronous call, transport should return "true" if message
 * successfully delivered on other side, otherwise - "false".
 *
 * @param data Pointer to data buffer.
 * @param len Length of data in bytes.
 *
 * @return Success status.
 *****************************************************************************/
typedef bool (*iqot_transport_send_cb_t)(const void* data, size_t len);

/**
 * @brief Struct with transport's callback functions.
 */
typedef struct _iqot_transport_cb_t {
  iqot_transport_send_cb_t send_data;
} iqot_transport_cb_t;

/******************************************************************************
 * @brief Action callback type.
 *
 * @param args Array of action arguments.
 *****************************************************************************/
typedef void (*iqot_action_cb_t)(const iqot_data_t args[]);


/******************************************************
 * FUNCTION DECLARATION
 ******************************************************/


/******************************************************************************
 * @brief Configure and start IQOT library.
 *
 * @warning This function is not thread-safe!
 * <br />
 * <br />This function should be called before any other "iqot_*" function.
 *
 * @param config_struct Structure with all needed parameters, for example:
 * telemetry interval, queue size, thread priority and etc.
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_init(const iqot_config_t* config_struct);

/******************************************************************************
 * @brief Deinit IQOT library.
 *
 * @warning This function is not thread-safe!
 * <br />
 * <br />This function should be called to finish work with IQoT library.
 *
 * @param none
 *
 * @return none
 *****************************************************************************/
void iqot_deinit(void);

/******************************************************************************
 * @brief Register transport module to library.
 *
 * @param transport_cb_struct Structure that contains all needed transport
 * functions.
 * @param transport_id Pointer to Transport ID variable.
 * It is used by transport module on calling library functions to indicate from
 * which transport module function is called.
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_register_transport(
    const iqot_transport_cb_t* transport_cb_struct,
    uint32_t* transport_id);

/******************************************************************************
 * @brief Register sensor (data source) module to library.
 *
 * @param sensor_name Zero-terminated string with name of sensor.
 * @param data_type Type which is used for sensor data type identify.
 * @param sensor_id Pointer to Sensor ID variable.
 * It is used by sensor module on data update to indicate from which sensor
 * \ref iqot_sensor_data_update() function is called.
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_register_sensor(const char* sensor_name,
                                  iqot_data_type_t data_type,
                                  uint32_t* sensor_id);

/******************************************************************************
 * @brief Register action to library.
 *
 * @param action_name Zero-terminated string with name of action.
 * @param action_cb Callback function for action.
 * @param action_args_count Count of arguments for callback.
 * @param ... Action arguments' names and types:
 * ..., name1, type1, name2, type2, ...
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_register_action(const char* action_name,
                                  iqot_action_cb_t action_cb,
                                  size_t action_args_count, ...);

/******************************************************************************
 * @brief Register action to library with variable argument list.
 *
 * @param action_name Zero-terminated string with name of action.
 * @param action_cb Callback function for action.
 * @param action_args_count Count of arguments for callback.
 * @param valist Action arguments' names and types:
 * ..., name1, type1, name2, type2, ...
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_register_action_valist(const char* action_name,
                                         iqot_action_cb_t action_cb,
                                         size_t action_args_count,
                                         va_list valist);

/******************************************************************************
 * @brief Update data from sensor module.
 *
 * @param sensor_id Sensor's ID. It is generated on \ref iqot_register_sensor().
 * @param data Sensor's data.
 *****************************************************************************/
void iqot_sensor_data_update(uint32_t sensor_id, iqot_data_t data);

/******************************************************************************
 * @brief Handle received data.
 *
 * Data is copied.
 *
 * @param transport_id Transport module ID. It is generated on
 *        \ref iqot_register_transport().
 * @param data Received data.
 * @param len Data length in bytes.
 *****************************************************************************/
void iqot_data_received(uint32_t transport_id, const void* data, size_t len);

/******************************************************************************
 * @brief Notify library about connection state changing.
 *
 * @param transport_id Transport module ID. It is generated on
 *        \ref iqot_register_transport().
 * @param new_state New state (e.g.: connected, disconnected).
 *****************************************************************************/
void iqot_connection_state_changed(uint32_t transport_id,
                                   iqot_connection_state_t new_state);

/******************************************************************************
 * @brief Run registered action by name.
 *
 * @param action_name Action name as Zero-terminated string.
 * @param action_args_count Count of passed arguments for action.
 * @param action_args Array of arguments' values.
 * @param action_args_types Array of arguments' types.
 *
 * @return Error info.
 *****************************************************************************/
iqot_error_t iqot_action_run(const char* action_name, size_t action_args_count,
                             const iqot_data_t action_args[],
                             const iqot_data_type_t action_args_types[]);

/******************************************************************************
 * @brief Get last sent sensor's data by name
 *
 * @warning This function does @b NOT support sensors with string data.
 *
 * @param sensor_name Sensor's name as Zero-terminated string.
 * @param out_data Pointer to output data variable.
 *
 * @return Type of sensor's data.
 * @retval IQOT_DATA_TYPE_INVALID indicates error.
 *****************************************************************************/
iqot_data_type_t iqot_sensor_data_get(const char* sensor_name,
                                      iqot_data_t* out_data);

#endif  /* IQOT_LIB_API_H */
