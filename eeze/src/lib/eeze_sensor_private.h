#ifndef EEZE_SENSOR_PRIVATE_H
#define EEZE_SENSOR_PRIVATE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Eeze_Sensor.h>

#ifndef EEZE_SENSOR_COLOR_DEFAULT
#define EEZE_SENSOR_COLOR_DEFAULT EINA_COLOR_BLUE
#endif
extern int _eeze_sensor_log_dom;
#ifdef CRI
#undef CRI
#endif
#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define CRI(...)  EINA_LOG_DOM_CRIT(_eeze_sensor_log_dom, __VA_ARGS__)
#define DBG(...)  EINA_LOG_DOM_DBG(_eeze_sensor_log_dom, __VA_ARGS__)
#define INF(...)  EINA_LOG_DOM_INFO(_eeze_sensor_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_sensor_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_eeze_sensor_log_dom, __VA_ARGS__)

/**
 * @typedef Eeze_Sensor
 * @since 1.8
 *
 * Internal data structure to hold the information about loaded and available runtime modules of
 * Eeze_Sensor.
 */
typedef struct _Eeze_Sensor
{
   Eina_Array *modules_array;  /**< Array of available runtime modules */
   Eina_Hash  *modules; /**< Hash with loaded modules */
}  Eeze_Sensor;

/**
 * @typedef Eeze_Sensor_Module;
 * @since 1.8
 *
 * Internal data structure for the modules. It mainly holds function pointers each modules provides
 * to be called from the Eeze_Sensor core to access the data provided by the module.
 */
typedef struct _Eeze_Sensor_Module
{
   Eina_Bool (*init)(void); /**< Pointer to module init function */
   Eina_Bool (*shutdown)(void); /**< Pointer to module shutdown function */
   Eina_Bool (*async_read)(Eeze_Sensor_Type sensor_type, void *user_data); /**< Pointer to module async_read function */
   Eina_Bool (*read)(Eeze_Sensor_Type sensor_type, Eeze_Sensor_Obj *obj); /**< Pointer to module read function */
   Eina_List *sensor_list; /**< List of sensor objects attached to the module */
} Eeze_Sensor_Module;

/**
 * @brief Register a module to eeze_sensor core.
 * @param name Module name used for reference internally.
 * @param mod Sensor module to be registered.
 * @return EINA_TRUE is the module was successfully registered. EINA_FALSE is not.
 *
 * Private functions for modules to register itself to eeze sensor core to
 * advertise their functionality. These registered modules will then be accessed
 * based on a priority that is currently hardcoded in the code. Once more module
 * are available we need to re-consider this approach.
 *
 * @since 1.8
 */
Eina_Bool eeze_sensor_module_register(const char *name, Eeze_Sensor_Module *mod);

/**
 * @brief Unregister a module from eeze_sensor core.
 * @param name Module name used for reference internally.
 * @return EINA_TRUE is the module was successfully unregistered. EINA_FALSE is not.
 *
 * Private functions for modules to unregister itself from eeze sensor core.
 *
 * @since 1.8
 */
Eina_Bool eeze_sensor_module_unregister(const char *name);
#endif // EEZE_SENSOR_PRIVATE_H
