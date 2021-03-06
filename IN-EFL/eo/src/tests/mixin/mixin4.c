#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "mixin4.h"
#include "simple.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN4_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin4",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin4_class_get, &class_desc, NULL, NULL);

