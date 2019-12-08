#include "router.h"

router_history_t *router_history_create(void)
{
	router_history_t *history;

	history = malloc(sizeof(router_history_t));
	history->index = 0;
	history->current = NULL;
	LinkedList_Init(&history->watchers);
	LinkedList_Init(&history->routes);
	return history;
}

static void router_history_on_destroy_route(void *data)
{
	router_route_destroy(data);
}

void router_history_destroy(router_history_t *history)
{
	history->index = 0;
	history->current = NULL;
	LinkedList_ClearData(&history->watchers, free);
	LinkedList_Clear(&history->routes, router_history_on_destroy_route);
	free(history);
}

router_watcher_t *router_history_watch(router_history_t *history,
				       router_callback_t callback, void *data)
{
	router_watcher_t *watcher;

	watcher = malloc(sizeof(router_watcher_t));
	watcher->node.data = watcher;
	watcher->node.next = NULL;
	watcher->node.prev = NULL;
	watcher->data = data;
	watcher->callback = callback;
	LinkedList_AppendNode(&history->watchers, &watcher->node);
	return watcher;
}

void router_history_unwatch(router_history_t *history,
			    router_watcher_t *watcher)
{
	LinkedList_Unlink(&history->watchers, &watcher->node);
	free(watcher);
}

static void router_history_change(router_history_t *history, router_route_t *to)
{
	router_watcher_t *watcher;
	router_linkedlist_node_t *node;

	for (LinkedList_Each(node, &history->watchers)) {
		watcher = node->data;
		watcher->callback(watcher->data, to, history->current);
	}
	history->current = to;
}

void router_history_push(router_history_t *history, router_route_t *route)
{
	router_history_change(history, route);
	LinkedList_Append(&history->routes, route);
	history->index = (int)history->routes.length - 1;
}

void router_history_replace(router_history_t *history, router_route_t *route)
{
	router_linkedlist_node_t *node;

	node = LinkedList_GetNode(&history->routes, history->index);
	router_history_change(history, route);
	router_route_destroy(node->data);
	LinkedList_Insert(&history->routes, history->index, route);
	LinkedList_DeleteNode(&history->routes, node);
}

void router_history_go(router_history_t *history, int delta)
{
	history->index += delta;
	if ((size_t)history->index >= history->routes.length) {
		history->index = (int)history->routes.length - 1;
	}
	if (history->index < 0) {
		history->index = 0;
	}
	router_history_change(history,
			      LinkedList_Get(&history->routes, history->index));
}
