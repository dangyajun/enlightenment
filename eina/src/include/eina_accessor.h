/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_ACCESSOR_H__
#define EINA_ACCESSOR_H__

#include "eina_types.h"
#include "eina_magic.h"

/**
 * @defgroup Eina_Accessor_Group Accessor Functions
 *
 * @{
 */

/**
 * @typedef Eina_Accessor
 * Type for accessors.
 */
typedef struct _Eina_Accessor Eina_Accessor;

typedef Eina_Bool (*Eina_Accessor_Get_At_Callback)(Eina_Accessor *it, unsigned int index, void **data);
typedef void *(*Eina_Accessor_Get_Container_Callback)(Eina_Accessor *it);
typedef void (*Eina_Accessor_Free_Callback)(Eina_Accessor *it);

struct _Eina_Accessor
{
#define EINA_MAGIC_ACCESSOR 0x98761232
   EINA_MAGIC;

   Eina_Accessor_Get_At_Callback        get_at;
   Eina_Accessor_Get_Container_Callback	get_container;
   Eina_Accessor_Free_Callback          free;
};

#define FUNC_ACCESSOR_GET_AT(Function) ((Eina_Accessor_Get_At_Callback)Function)
#define FUNC_ACCESSOR_GET_CONTAINER(Function) ((Eina_Accessor_Get_Container_Callback)Function)
#define FUNC_ACCESSOR_FREE(Function) ((Eina_Accessor_Free_Callback)Function)


EAPI void eina_accessor_free           (Eina_Accessor *accessor);

EAPI Eina_Bool eina_accessor_data_get  (Eina_Accessor *accessor, unsigned int position, void **data);
EAPI void *eina_accessor_container_get (Eina_Accessor *accessor);

EAPI void eina_accessor_over           (Eina_Accessor *accessor,
					Eina_Each cb,
					unsigned int start,
					unsigned int end,
					const void *fdata);

/**
 * @}
 */

#endif
