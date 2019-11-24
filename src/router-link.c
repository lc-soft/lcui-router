#include "router.h"
#include <LCUI/gui/widget.h>

typedef struct router_link_t router_link_t;

struct router_link_t {
	char *active_class;
	router_t *router;
	router_location_t *to;
	router_watcher_t *watcher;
	router_boolean_t replace;
	router_boolean_t exact;
};

static struct {
	LCUI_WidgetPrototype proto;
} self;

static void router_link_on_route_update(void *w, const router_route_t *to,
					const router_route_t *from)
{
	// TODO:
}

static void router_link_on_ready(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	router_t *router;
	router_link_t *link;
	const router_route_t *route;
	const char *name;
	LCUI_Widget parent;

	link = Widget_GetData(w, self.proto);
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
	link->watcher = router_watch(router, router_link_on_route_update, w);
	link->router = router;
	Widget_UnbindEvent(w, "ready", router_link_on_ready);
}

static void router_link_on_init(LCUI_Widget w)
{
	router_link_t *link;

	link = Widget_AddData(w, self.proto, sizeof(router_link_t));
	link->active_class = strdup("router-link-active");
	link->to = NULL;
	link->replace = FALSE;
	link->exact = FALSE;
	Widget_BindEvent(w, "ready", router_link_on_ready, NULL, NULL);
}

static void router_link_on_destroy(LCUI_Widget w)
{
	router_link_t *link;

	link = Widget_GetData(w, self.proto);
	if (link->router) {
		router_unwatch(link->router, link->watcher);
		link->watcher = NULL;
	}
	router_mem_free(link->active_class);
}

static void router_link_on_click(LCUI_Widget w)
{
	// TODO:
	router_t *router;
	router_link_t *link;

	link = Widget_GetData(w, self.proto);
	if (link->replace) {
		router_replace(router, link->to);
	} else {
		router_push(router, link->to);
	}
}

void router_link_install(void)
{
	self.proto = LCUIWidget_NewPrototype("router-link", NULL);
	self.proto->init = router_link_on_init;
	self.proto->destroy = router_link_on_destroy;
}
