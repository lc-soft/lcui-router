#ifndef ROUTER_H
#define ROUTER_H

#include <string.h>
#include <stdlib.h>
#include <LCUI.h>
#include "lcui-router.h"

#pragma warning(disable: 4996)

#define router_mem_free(ptr)       \
	do {                       \
		if (ptr) {         \
			free(ptr); \
		}                  \
		ptr = NULL;        \
	} while (0)

struct router_location_t {
	char *name;
	char *path;
	char *hash;
	router_string_dict_t *params;
	router_string_dict_t *query;
	router_boolean_t normalized;
};

struct router_route_t {
	char *name;
	char *path;
	char *full_path;
	char *hash;
	router_string_dict_t *query;
	router_string_dict_t *params;
	router_linkedlist_t matched;
};

struct router_route_record_t {
	char *name;
	char *path;
	const router_route_record_t *parent;
	router_string_dict_t *components;
	router_linkedlist_node_t node;
};

struct router_history_t {
	int index;
	router_route_t *current;
	router_linkedlist_t routes;
	router_linkedlist_t watchers;
};

struct router_config_t {
	char *name;
	char *path;
	router_string_dict_t *components;
};

struct router_matcher_t {
	Dict *name_map;
	Dict *path_map;
	router_linkedlist_t path_list;
};

struct router_watcher_t {
	void *data;
	router_callback_t callback;
	router_linkedlist_node_t node;
};

struct router_resolved_t {
	router_route_t *route;
	router_location_t *location;
};

struct router_t {
	char *name;
	char *link_active_class;
	char *link_exact_active_class;
	router_matcher_t *matcher;
	router_history_t *history;
};

#endif
