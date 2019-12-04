#include "router.h"

router_location_t *router_location_create(const char *name, const char *path)
{
	router_location_t *location;

	location = malloc(sizeof(router_location_t));
	location->name = name ? strdup(name) : NULL;
	location->path = path ? strdup(path) : NULL;
	location->hash = NULL;
	location->query = NULL;
	location->params = NULL;
	location->normalized = FALSE;
	return location;
}

void router_location_destroy(router_location_t *location)
{
	router_mem_free(location->name);
	router_mem_free(location->hash);
	router_mem_free(location->path);
	if (location->params) {
		router_string_dict_destroy(location->params);
	}
	if (location->query) {
		router_string_dict_destroy(location->query);
	}
	location->query = NULL;
	location->params = NULL;
	free(location);
}

router_location_t *router_location_duplicate(const router_location_t *target)
{
	router_location_t *location;

	location = router_location_create(target->name, target->path);
	location->hash = target->hash ? strdup(target->hash) : NULL;
	location->query = router_string_dict_duplicate(target->query);
	location->params = router_string_dict_duplicate(target->params);
	location->normalized = target->normalized;
	return location;
}

void router_location_set_name(router_location_t *location, const char *name)
{
	router_mem_free(location->name);
	if (name) {
		location->name = strdup(name);
	}
}

static router_location_t *router_location_from_path(
    const router_location_t *raw, const router_route_t *current,
    router_boolean_t append)
{
	size_t i;
	char *path = NULL;
	char *hash = NULL;
	char *query_str = NULL;
	const char *base_path = current ? current->path : "/";
	router_location_t *location;
	router_string_dict_t *query;

	if (raw->path) {
		path = strdup(raw->path);
		for (i = 0; path[i]; ++i) {
			if (path[i] == '#') {
				hash = strdup(raw->path + i);
				path[i] = 0;
				break;
			}
		}
		for (i = 0; path[i]; ++i) {
			if (path[i] == '?') {
				query_str = strdup(raw->path + i + 1);
				path[i] = 0;
				break;
			}
		}
	}
	location = router_location_create(NULL, NULL);
	if (path) {
		location->path = router_path_resolve(path, base_path, append);
		free(path);
	} else {
		location->path = strdup(base_path);
	}
	query = router_parse_query(query_str);
	router_string_dict_extend(query, raw->query);
	location->query = query;
	location->hash = hash;
	location->normalized = TRUE;
	router_mem_free(query_str);
	return location;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L936

router_location_t *router_location_normalize(const router_location_t *raw,
					     const router_route_t *current,
					     router_boolean_t append)
{
	Dict *params;
	router_location_t *location;
	router_route_record_t *record;

	if (raw->normalized || raw->name) {
		return router_location_duplicate(raw);
	}
	if (!raw->path && raw->params && current) {
		location = router_location_duplicate(raw);
		location->normalized = TRUE;
		params = router_string_dict_create();
		router_string_dict_extend(params, current->params);
		router_string_dict_extend(params, raw->params);
		if (current->name) {
			router_location_set_name(location, current->name);
			Dict_Release(location->params);
			location->params = params;
		} else if (current->matched.length > 0) {
			record = LinkedList_Get(&current->matched,
						current->matched.length - 1);
			if (location->path) {
				free(location->path);
			}
			location->path =
			    router_path_fill_params(record->path, params);
		} else {
			Logger_Warning("relative params navigation requires a "
				       "current route.");
		}
		return location;
	}
	return router_location_from_path(raw, current, append);
}

const char *router_location_get_param(const router_location_t *location,
				      const char *key)
{
	if (!location->params) {
		return NULL;
	}
	return router_string_dict_get(location->params, key);
}

const char *router_location_get_query(const router_location_t *location,
				      const char *key)
{
	if (!location->query) {
		return NULL;
	}
	return router_string_dict_get(location->query, key);
}

char *router_location_stringify(const router_location_t *location)
{
	// TODO:
	return NULL;
}
