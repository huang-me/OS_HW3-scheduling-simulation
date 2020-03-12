#ifndef RESOURCE_H
#define RESOURCE_H

#include "typedefine.h"
#include "config.h"

status_type get_resource(resource_type id);
status_type release_resource(resource_type id);

#endif /* RESOURCE_H */