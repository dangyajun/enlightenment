/** @file etk_signal_callback.c */
#include "etk_signal_callback.h"
#include <stdlib.h>
#include "etk_signal.h"

/**
 * @addtogroup Etk_Signal_Callback
 * @{
 */

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Creates a new signal callback for the signal @a signal, with callback function @a callback, and with user data @a data
 * @param signal the signal to add this callback to
 * @param callback the callback function called when the signal is emitted
 * @param data the user data to pass to the callback function
 * @param swapped if @a swapped != 0, the callback function with the data as the only arguments
 * @return Returns the new callback on succes or NULL on failure
 * @warning The new signal callback has to be freed with etk_signal_callback_delete()
 */
Etk_Signal_Callback *etk_signal_callback_new(Etk_Signal *signal, Etk_Signal_Callback_Function callback, void *data, Etk_Bool swapped)
{
   Etk_Signal_Callback *new_callback;

   if (!signal || !callback)
      return NULL;

   new_callback = malloc(sizeof(Etk_Signal_Callback));
   new_callback->signal = signal;
   new_callback->callback = callback;
   new_callback->data = data;
   new_callback->swapped = swapped;

   return new_callback;
}

/**
 * @brief Deletes the signal callback
 * @param signal_callback the signal callback to delete
 */
void etk_signal_callback_delete(Etk_Signal_Callback *signal_callback)
{
   free(signal_callback);
}

/**
 * @brief Calls the callback @a callback on the object @a object
 * @param callback the signal callback to call
 * @param object the object to call the callback on
 * @param return_value the location for the return value (if none, it can be NULL)
 * @param ... the arguments to pass to the callback
 */
void etk_signal_callback_call(Etk_Signal_Callback *callback, Etk_Object *object, void *return_value,  ...)
{
   va_list args;

   if (!callback || !object)
      return;

   va_start(args, return_value);
   etk_signal_callback_call_valist(callback, object, return_value, args);
   va_end(args);
}

/**
 * @brief Calls the callback @a callback on the object @a object
 * @param callback the signal callback to call
 * @param object the object to call the callback on
 * @param return_value the location for the return value (if none, it can be NULL)
 * @param args the arguments to pass to the callback
 */
void etk_signal_callback_call_valist(Etk_Signal_Callback *callback, Etk_Object *object, void *return_value, va_list args)
{
   Etk_Marshaller marshaller;

   if (!callback || !callback->callback || !callback->signal ||
         !object || !(marshaller = etk_signal_marshaller_get(callback->signal)))
      return;

   if (callback->swapped)
   {
      Etk_Signal_Swapped_Callback_Function swapped_callback = ETK_SWAPPED_CALLBACK(callback->callback);
      swapped_callback(callback->data);
   }
   else
   {
      va_list args2;
      
      va_copy(args2, args);
      marshaller(callback->callback, object, callback->data, return_value, args2);
      va_end(args2);
   }
}

/** @} */
