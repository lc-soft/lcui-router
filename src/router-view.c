#include "router.h"
#include "lcui-router-view.h"

typedef struct RouterViewRec_ {
	size_t index;
	router_t *router;
	router_watcher_t *watcher;
	router_boolean_t keep_alive;
	Dict *cache;
	LCUI_Widget matched_widget;
} RouterViewRec, *RouterView;

static LCUI_WidgetPrototype router_view_proto;

static LCUI_Widget RouterView_GetMatched(LCUI_Widget w,
					 const router_route_t *route)
{
	const char *name;
	const char *component_name;
	const router_route_record_t *record;
	RouterView view;
	LCUI_Widget component;

	view = Widget_GetData(w, router_view_proto);
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
			Dict_Add(view->cache, (void *)component_name,
				 component);
		}
	} else {
		component = LCUIWidget_New(component_name);
	}
	return component;
}

static void RouterView_OnRouteUpdate(void *w, const router_route_t *to,
				     const router_route_t *from)
{
	RouterView view;

	view = Widget_GetData(w, router_view_proto);
	if (view->matched_widget && view->keep_alive) {
		Widget_Unlink(view->matched_widget);
	}
	view->matched_widget = RouterView_GetMatched(w, to);
	Widget_Empty(w);
	Widget_Append(w, view->matched_widget);
}

static void RouterView_OnReady(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	size_t index;
	router_t *router;
	const char *name;
	const router_route_t *route;
	LCUI_Widget parent;
	RouterView view;

	view = Widget_GetData(w, router_view_proto);
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
	view->watcher = router_watch(router, RouterView_OnRouteUpdate, w);
	view->router = router;
	view->index = index;
	if (route) {
		view->matched_widget = RouterView_GetMatched(w, route);
		Widget_Append(w, view->matched_widget);
	} else {
		view->matched_widget = NULL;
	}
	Widget_UnbindEvent(w, "ready", RouterView_OnReady);
}

LCUI_Widget RouterView_GetMatchedWidget(LCUI_Widget w)
{
	RouterView view;

	view = Widget_GetData(w, router_view_proto);
	return view->matched_widget;
}

static void RouterView_OnInit(LCUI_Widget w)
{
	RouterView view;

	view = Widget_AddData(w, router_view_proto, sizeof(RouterViewRec));
	view->router = NULL;
	view->watcher = NULL;
	view->cache = Dict_Create(&DictType_StringCopyKey, NULL);
	;
	view->keep_alive = FALSE;
	Widget_BindEvent(w, "ready", RouterView_OnReady, NULL, NULL);
}

static void RouterView_OnDestroy(LCUI_Widget w)
{
	RouterView view;

	view = Widget_GetData(w, router_view_proto);
	if (view->router) {
		router_unwatch(view->router, view->watcher);
	}
	Dict_Release(view->cache);
	view->cache = NULL;
	view->watcher = NULL;
	view->router = NULL;
}

void UI_InitRouterView(void)
{
	router_view_proto = LCUIWidget_NewPrototype("router-view", NULL);
	router_view_proto->init = RouterView_OnInit;
	router_view_proto->destroy = RouterView_OnDestroy;
}
