#include "router.h"
#include "lcui-router-link.h"

// Refer: https://router.vuejs.org/api/#router-link-props

typedef struct RouterLinkRec_ {
	char *active_class;
	char *exact_active_class;
	router_boolean_t exact;
	router_boolean_t replace;
	router_t *router;
	router_location_t *to;
	router_route_record_t *record;
	router_watcher_t *watcher;
} RouterLinkRec, *RouterLink;

static LCUI_WidgetPrototype router_link_proto;

static void RouterLink_OnRouteUpdate(void *w, const router_route_t *to,
				     const router_route_t *from)
{
	RouterLink link;
	router_route_t *route;
	router_resolved_t *resolved;
	router_boolean_t is_same_route;

	link = Widget_GetData(w, router_link_proto);
	resolved = router_resolve(link->router, link->to, FALSE);
	route = router_resolved_get_route(resolved);
	// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/src/components/link.js#L65
	is_same_route = router_is_same_route(to, route);
	if (is_same_route) {
		Widget_AddClass(w, link->exact_active_class);
	} else {
		Widget_RemoveClass(w, link->exact_active_class);
	}
	do {
		if (link->exact) {
			if (is_same_route) {
				Widget_AddClass(w, link->active_class);
				break;
			}
		} else if (router_is_included_route(to, route)) {
			Widget_AddClass(w, link->active_class);
			break;
		}
		Widget_RemoveClass(w, link->active_class);
	} while (0);
	router_resolved_destroy(resolved);
}

static void RouterLink_OnClick(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	RouterLink link;

	link = Widget_GetData(w, router_link_proto);
	if (link->replace) {
		router_replace(link->router, link->to);
	} else {
		router_push(link->router, link->to);
	}
}

static void RouterLink_OnSetAttribute(LCUI_Widget w, const char *name,
				      const char *value)
{
	RouterLink link;
	router_location_t *location;

	link = Widget_GetData(w, router_link_proto);
	if (strcmp(name, "to") == 0) {
		location = router_location_create(NULL, value);
		RouterLink_SetLocation(w, location);
		router_location_destroy(location);
	} else if (strcmp(name, "exact") == 0) {
		RouterLink_SetExact(w, strcmp(value, "exact") == 0);
	} else if (strcmp(name, "exact-active-class") == 0) {
		if (link->exact_active_class) {
			free(link->exact_active_class);
		}
		link->exact_active_class = strdup(value);
	} else if (strcmp(name, "active-class") == 0) {
		if (link->active_class) {
			free(link->active_class);
		}
		link->active_class = strdup(value);
	}
}

static void RouterLink_OnReady(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	router_t *router;
	RouterLink link;
	const char *name;
	LCUI_Widget parent;

	link = Widget_GetData(w, router_link_proto);
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
	link->watcher = router_watch(router, RouterLink_OnRouteUpdate, w);
	link->router = router;
	Widget_UnbindEvent(w, "ready", RouterLink_OnReady);
}

static void RouterLink_OnInit(LCUI_Widget w)
{
	RouterLink link;

	link = Widget_AddData(w, router_link_proto, sizeof(RouterLinkRec));
	link->active_class = strdup("router-link-active");
	link->exact_active_class = strdup("router-link-exact-active");
	link->to = NULL;
	link->replace = FALSE;
	link->exact = FALSE;
	link->router = NULL;
	link->watcher = NULL;
	Widget_BindEvent(w, "ready", RouterLink_OnReady, NULL, NULL);
	Widget_BindEvent(w, "click", RouterLink_OnClick, NULL, NULL);
}

static void RouterLink_OnDestroy(LCUI_Widget w)
{
	RouterLink link;

	link = Widget_GetData(w, router_link_proto);
	if (link->router) {
		router_unwatch(link->router, link->watcher);
	}
	router_location_destroy(link->to);
	router_mem_free(link->active_class);
	router_mem_free(link->exact_active_class);
	link->watcher = NULL;
	link->to = NULL;
}

void RouterLink_SetLocation(LCUI_Widget w, router_location_t *location)
{
	RouterLink link;

	link = Widget_GetData(w, router_link_proto);
	if (link->to) {
		router_location_destroy(link->to);
	}
	link->to = router_location_duplicate(location);
}

void RouterLink_SetExact(LCUI_Widget w, router_boolean_t exact)
{
	RouterLink link;

	link = Widget_GetData(w, router_link_proto);
	link->exact = exact;
}

void UI_InitRouterLink(void)
{
	router_link_proto = LCUIWidget_NewPrototype("router-link", NULL);
	router_link_proto->init = RouterLink_OnInit;
	router_link_proto->setattr = RouterLink_OnSetAttribute;
	router_link_proto->destroy = RouterLink_OnDestroy;
}
