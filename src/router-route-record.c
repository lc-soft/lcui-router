﻿#include "router.h"

router_route_record_t *router_route_record_create(void)
{
	router_route_record_t *record;

	record = malloc(sizeof(router_route_record_t));
	record->name = NULL;
	record->path = NULL;
	record->components = router_string_dict_create();
	record->node.data = record;
	record->node.prev = NULL;
	record->node.next = NULL;
	record->parent = NULL;
	return record;
}

void router_route_record_destroy(router_route_record_t *record)
{
	if (record->name) {
		free(record->name);
	}
	if (record->path) {
		free(record->path);
	}
	record->name = NULL;
	record->path = NULL;
	Dict_Release(record->components);
	free(record);
}

void router_route_record_set_path(router_route_record_t *record,
				  const char *path)
{
	router_mem_free(record->path);
	record->path = strdup(path);
}

const char *router_route_record_get_component(
    const router_route_record_t *record, const char *key)
{
	if (!key) {
		key = "default";
	}
	return router_string_dict_get(record->components, key);
}
