#include "router.h"

router_matcher_t *router_matcher_create(void)
{
	router_matcher_t *matcher;

	matcher = malloc(sizeof(router_matcher_t));
	matcher->name_map = Dict_Create(&DictType_StringKey, NULL);
	matcher->path_map = Dict_Create(&DictType_StringKey, NULL);
	LinkedList_Init(&matcher->path_list);
	return matcher;
}

void router_matcher_destroy(router_matcher_t *matcher)
{
	Dict_Release(matcher->name_map);
	Dict_Release(matcher->path_map);
	LinkedList_ClearData(&matcher->path_list, router_route_record_destroy);
	matcher->name_map = NULL;
	matcher->path_map = NULL;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L1282

router_route_record_t *router_matcher_add_route_record(
    router_matcher_t *matcher, const router_config_t *config,
    const router_route_record_t *parent)
{
	size_t i, len;
	router_route_record_t *record;
	LinkedListNode *node;

	record = router_route_record_create();
	if (config->name) {
		if (Dict_FetchValue(matcher->name_map, config->name)) {
			Logger_Error("duplicate named routes definition: "
				     "{ name: \"%s\", path: \"%s\" }",
				     config->name, config->path);
			router_route_record_destroy(record);
			return NULL;
		}
		record->name = strdup(config->name);
		Dict_Add(matcher->name_map, record->name, record);
	}
	router_route_record_set_path(record, config->path);
	if (Dict_FetchValue(matcher->path_map, record->path)) {
		Logger_Warning("duplicate routes definition: { path: \"%s\" }",
			       record->path);
	} else {
		LinkedList_AppendNode(&matcher->path_list, &record->node);
		Dict_Add(matcher->path_map, record->path, record);
	}
	if (parent) {
		LinkedList_Unlink(&matcher->path_list, &record->node);
		LinkedList_Link(&matcher->path_list, parent->node.prev,
				&record->node);
	}
	len = matcher->path_list.length;
	node = matcher->path_list.head.next;
	// ensure wildcard routes are always at the end
	for (i = 0; node && i < len; ++i, node = node->next) {
		if (strcmp(((router_route_t *)node->data)->path, "*") == 0) {
			LinkedList_Unlink(&matcher->path_list, node);
			LinkedList_AppendNode(&matcher->path_list, node);
			i--;
			len--;
		}
	}
	record->parent = parent;
	return record;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L1599

router_boolean_t router_matcher_match_route(router_route_record_t *record,
					    const char *path, Dict *params)
{
	char *key;
	char *value;
	size_t len;
	size_t base_i;
	size_t *base_seps;
	size_t base_seps_i;
	size_t base_seps_len;
	size_t target_i;
	size_t *target_seps;
	size_t target_seps_i;
	size_t target_seps_len;

	base_seps_i = 1, target_seps_i = 1;
	base_seps_len = router_path_scan_separators(record->path, &base_seps);
	target_seps_len = router_path_scan_separators(path, &target_seps);
	// record->path: "/example/:type/:name/info"
	// path: "/exmaple/food/orange/info"
	while (target_seps_i < target_seps_len && base_seps_i < base_seps_len) {
		base_i = base_seps[base_seps_i - 1] + 1;
		target_i = target_seps[target_seps_i - 1] + 1;
		// if is param key
		if (record->path[base_i] == ':') {
			base_i++;
			len = base_seps[base_seps_i] - base_i;
			key = malloc(sizeof(char) * (len + 1));
			strncpy(key, record->path + base_i, len);
			key[len] = 0;
			len = target_seps[target_seps_i] - target_i;
			value = malloc(sizeof(char) * (len + 1));
			strncpy(value, path + target_i, len);
			value[len] = 0;
			Dict_Add(params, key, value);
			free(key);
			free(value);
		} else {
			len = base_seps[base_seps_i] - base_i;
			if (len != target_seps[target_seps_i] - target_i) {
				return FALSE;
			}
			if (strncmp(record->path + base_i, path + target_i,
				    len)) {
				return FALSE;
			}
		}
		base_seps_i++;
		target_seps_i++;
	}
	return TRUE;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L1457

router_route_t *router_matcher_match_by_name(
    router_matcher_t *matcher, router_location_t *location,
    const router_route_t *current_route)
{
	char *key;
	char *value;
	router_route_record_t *record;
	router_linkedlist_t param_names;
	router_linkedlist_node_t *node;

	record = Dict_FetchValue(matcher->name_map, location->name);
	if (!record) {
		Logger_Warning("route with name '%s' does not exist",
			       location->name);
		return router_route_create(NULL, location);
	}
	if (!location->params) {
		location->params = router_string_dict_create();
	}
	if (current_route) {
		LinkedList_Init(&param_names);
		router_path_parse_keys(record->path, &param_names);
		for (LinkedList_Each(node, &param_names)) {
			key = node->data;
			value = Dict_FetchValue(current_route->params, key);
			if (value && !Dict_FetchValue(location->params, key)) {
				Dict_Add(location->params, key, value);
			}
		}
	}
	location->path =
	    router_path_fill_params(record->path, location->params);
	return router_route_create(record, location);
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L1481

router_route_t *router_matcher_match_by_path(router_matcher_t *matcher,
					     router_location_t *location)
{
	router_route_record_t *record;
	router_linkedlist_node_t *node;

	for (LinkedList_Each(node, &matcher->path_list)) {
		record = Dict_FetchValue(matcher->path_map, node->data);
		if (router_matcher_match_route(record, location->path,
					       location->params)) {
			return router_route_create(record, location);
		}
	}
	return router_route_create(NULL, location);
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L1449

router_route_t *router_matcher_match(router_matcher_t *matcher,
				     const router_location_t *raw_location,
				     const router_route_t *current_route)
{
	router_route_t *route;
	router_location_t *location;

	location = router_location_normalize(raw_location, current_route);
	if (location->name) {
		route = router_matcher_match_by_name(matcher, location,
						     current_route);
	} else if (location->path) {
		route = router_matcher_match_by_path(matcher, location);
	} else {
		route = router_route_create(NULL, location);
	}
	router_location_destroy(location);
	return route;
}
