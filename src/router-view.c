#include "router.h"
#include <LCUI/gui/widget.h>

typedef struct router_view_t router_view_t;

struct router_view_t {
	size_t index;
	router_t *router;
	router_watcher_t *watcher;
	router_boolean_t keep_alive;
	Dict *cache;
	LCUI_Widget current_widget;
};

static struct {
	LCUI_WidgetPrototype proto;
} self;

static LCUI_Widget router_view_get_matched(LCUI_Widget w, const router_route_t *route)
{
	const char *name;
	const char *component_name;
	const router_route_record_t *record;
	router_view_t *view;
	LCUI_Widget component;

	view = Widget_GetData(w, self.proto);
	name = Widget_GetAttribute(w, "name");
	record = router_route_get_matched_record(route, view->index);
	if (!record) {
		Logger_Error("no matching route found");
		return NULL;
	}
	if (!name) {
		name = "default";
	}
	component_name = router_route_record_get_component(record, name);
	if (view->keep_alive) {
		component = Dict_FetchValue(view->cache, component_name);
		if (!component) {
			component = LCUIWidget_New(component_name);
			Dict_Add(view->cache, (void*)component_name, component);
		}
	} else {
		component = LCUIWidget_New(component_name);
	}
	return component;
}

static void router_view_on_route_update(void *w, const router_route_t *to,
					const router_route_t *from)
{
	router_view_t *view;

	view = Widget_GetData(w, self.proto);
	if (view->current_widget && view->keep_alive) {
		Widget_Unlink(view->current_widget);
	}
	view->current_widget = router_view_get_matched(w, to);
	Widget_Empty(w);
	Widget_Append(w, view->current_widget);
}

static void router_view_on_ready(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	size_t index;
	router_t *router;
	router_view_t *view;
	const router_route_t *route;
	const char *name;
	LCUI_Widget parent;

	view = Widget_GetData(w, self.proto);
	for (index = 0, parent = w->parent; parent; parent = parent->parent) {
		if (Widget_CheckType(parent, "router-view")) {
			++index;
		}
	}
	for (parent = w; parent; parent = parent->parent) {
		name = Widget_GetAttribute(parent, "router");
		if (name) {
			break;
		}
	}
	if (!name) {
		name = "default";
	}
	router = router_get_by_name(name);
	route = router_get_current_route(router);
	view->watcher = router_watch(router, router_view_on_route_update, w);
	view->router = router;
	view->index = index;
	view->current_widget = router_view_get_matched(w, route);
	Widget_Append(w, view->current_widget);
	Widget_UnbindEvent(w, "ready", router_view_on_ready);
}

static void router_view_on_init(LCUI_Widget w)
{
	router_view_t *view;

	view = Widget_AddData(w, self.proto, sizeof(router_view_t));
	view->router = NULL;
	view->watcher = NULL;
	view->cache = Dict_Create(&DictType_StringCopyKey, NULL);;
	view->keep_alive = FALSE;
	Widget_BindEvent(w, "ready", router_view_on_ready, NULL, NULL);
}

static void router_view_on_destroy(LCUI_Widget w)
{
	router_view_t *view;

	view = Widget_GetData(w, self.proto);
	if (view->router) {
		router_unwatch(view->router, view->watcher);
		view->watcher = NULL;
	}
}

void router_view_install(void)
{
	self.proto = LCUIWidget_NewPrototype("router-view", NULL);
	self.proto->init = router_view_on_init;
	self.proto->destroy = router_view_on_destroy;
}
