#ifndef LCUI_ROUTER_H
#define LCUI_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef Dict router_string_dict_t;
typedef LinkedList router_linkedlist_t;
typedef LinkedListNode router_linkedlist_node_t;
typedef unsigned char router_boolean_t;
typedef struct router_t router_t;
typedef struct router_location_t router_location_t;
typedef struct router_config_t router_config_t;
typedef struct router_route_t router_route_t;
typedef struct router_route_record_t router_route_record_t;
typedef struct router_history_t router_history_t;
typedef struct router_watcher_t router_watcher_t;
typedef struct router_matcher_t router_matcher_t;
typedef struct router_resolved_t router_resolved_t;
typedef void (*router_callback_t)(void *, const router_route_t *,
				  const router_route_t *);

// router string dict

router_string_dict_t *router_string_dict_create(void);

void router_string_dict_destroy(router_string_dict_t *dict);

void router_string_dict_delete(router_string_dict_t *dict, const char *key);

int router_string_dict_set(router_string_dict_t *dict, const char *key,
			   const char *value);

const char *router_string_dict_get(router_string_dict_t *dict, const char *key);

size_t router_string_dict_extend(router_string_dict_t *target,
				 router_string_dict_t *other);

router_string_dict_t *router_string_dict_duplicate(
    router_string_dict_t *target);

router_boolean_t router_string_dict_includes(router_string_dict_t *a,
					     router_string_dict_t *b);

router_boolean_t router_string_dict_equal(router_string_dict_t *a,
					  router_string_dict_t *b);

// router utils

char *router_path_fill_params(const char *path, router_string_dict_t *params);

const char *router_path_parse_key(const char *path, char key[256],
				  size_t *key_len);

size_t router_path_parse_keys(const char *path, router_linkedlist_t *keys);

char *router_path_resolve(const char *relative, const char *base,
			  router_boolean_t append);

router_string_dict_t *router_parse_query(const char *query_str);

int router_string_compare(const char *a, const char *b);

int router_path_compare(const char *a, const char *b);

router_boolean_t router_path_starts_with(const char *path, const char *subpath);

// router config

router_config_t *router_config_create(void);

void router_config_destroy(router_config_t *config);

// router location

router_location_t *router_location_create(const char *name, const char *path);

void router_location_destroy(router_location_t *location);

void router_location_set_name(router_location_t *location, const char *name);

router_location_t *router_location_duplicate(const router_location_t *location);

router_location_t *router_location_normalize(const router_location_t *raw,
					     const router_route_t *current,
					     router_boolean_t append);

const char *router_location_get_param(const router_location_t *location,
				      const char *key);

const char *router_location_get_query(const router_location_t *location,
				      const char *key);

char *router_location_stringify(const router_location_t *location);

// router route record

router_route_record_t *router_route_record_create(void);

void router_route_record_destroy(router_route_record_t *record);

void router_route_record_set_path(router_route_record_t *record,
				  const char *path);

const char *router_route_record_get_component(
    const router_route_record_t *record, const char *key);

// router route

router_route_t *router_route_create(const router_route_record_t *record,
				    const router_location_t *location);

void router_route_destroy(router_route_t *route);

const router_route_record_t *router_route_get_matched_record(
    const router_route_t *route, size_t index);

const char *router_route_get_param(const router_route_t *route,
				   const char *key);

const char *router_route_get_query(const router_route_t *route,
				   const char *key);

// router matcher

router_matcher_t *router_matcher_create(void);

void router_matcher_destroy(router_matcher_t *matcher);

router_route_t *router_matcher_match(router_matcher_t *matcher,
				     const router_location_t *raw_location,
				     const router_route_t *current_route);

router_route_record_t *router_matcher_add_route_record(
    router_matcher_t *matcher, const router_config_t *config,
    const router_route_record_t *parent);

// router history

router_history_t *router_history_create(void);

void router_history_destroy(router_history_t *history);

router_watcher_t *router_history_watch(router_history_t *history,
				       router_callback_t callback, void *data);

void router_history_unwatch(router_history_t *history,
			    router_watcher_t *watcher);

void router_history_push(router_history_t *history, router_route_t *route);

void router_history_replace(router_history_t *history, router_route_t *route);

void router_history_go(router_history_t *history, int delta);

// router

router_t *router_create(const char *name);

void router_destroy(router_t *router);

router_route_record_t *router_add_route_record(
    router_t *router, const router_config_t *config,
    const router_route_record_t *parent);

router_route_t *router_match(router_t *router,
			     const router_location_t *raw_location,
			     const router_route_t *current_route);

router_route_record_t *router_get_matched_route_record(router_t *router,
						       size_t index);

router_watcher_t *router_watch(router_t *router, router_callback_t callback,
			       void *data);

void router_unwatch(router_t *router, router_watcher_t *watcher);

router_resolved_t *router_resolve(router_t *router, router_location_t *location,
				  router_boolean_t append);

router_location_t *router_resolved_get_location(router_resolved_t *resolved);

router_route_t *router_resolved_get_route(router_resolved_t *resolved);

void router_resolved_destroy(router_resolved_t *resolved);

const router_route_t *router_get_current_route(router_t *router);

void router_push(router_t *router, router_location_t *location);

void router_replace(router_t *router, router_location_t *location);

void router_go(router_t *router, int delta);

void router_back(router_t *router);

void router_forward(router_t *router);

router_t *router_get_by_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif
