#include "router.h"

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L266

router_route_t *router_route_create(const router_route_record_t *record,
				    const router_location_t *location)
{
	router_route_t *route;

	route = malloc(sizeof(router_route_t));
	if (location->name) {
		route->name = strdup(location->name);
	} else if (record && record->name) {
		route->name = strdup(record->name);
	}
	route->path = strdup(location->path ? location->path : "/");
	route->hash = strdup(location->hash ? location->hash : "");
	route->query = router_string_dict_duplicate(location->query);
	route->params = router_string_dict_duplicate(location->params);
	route->full_path = router_location_stringify(location);
	LinkedList_Init(&route->matched);
	while (record) {
		LinkedList_Insert(&route->matched, 0, (void *)record);
		record = record->parent;
	}
	return route;
}

void router_route_destroy(router_route_t *route)
{
	router_mem_free(route->name);
	router_mem_free(route->path);
	free(route);
}

const router_route_record_t *router_route_get_matched_record(
    const router_route_t *route, size_t index)
{
	return LinkedList_Get(&route->matched, index);
}
