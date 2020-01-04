#include <stdio.h>
#include "../src/router.c"
#include "../src/router-history.c"
#include "../src/router-matcher.c"
#include "../src/router-config.c"
#include "../src/router-location.c"
#include "../src/router-route-record.c"
#include "../src/router-route.c"
#include "../src/router-string-dict.c"
#include "../src/router-utils.c"
#include "../src/router-link.c"
#include "../src/router-view.c"

#ifdef _WIN32
#define COLOR_NONE
#define COLOR_RED
#define COLOR_GREEN
#else
#define COLOR_NONE "\e[0m"
#define COLOR_RED "\e[0;31m"
#define COLOR_GREEN "\e[0;32m"
#endif
#define RED(TEXT) COLOR_RED TEXT COLOR_NONE
#define GREEN(TEXT) COLOR_GREEN TEXT COLOR_NONE

static size_t tests_passed = 0;
static size_t tests_total = 0;

#define describe(NAME, FUNC)    \
	printf("\n%s\n", NAME); \
	FUNC();

void it_i(const char *name, int actual, int expected)
{
	tests_total++;
	if (actual == expected) {
		printf(GREEN("  √ ") "%s == %d\n", name, expected);
		tests_passed++;
		return;
	}
	printf(RED("  × %s == %d\n"), name, expected);
	printf(RED("    AssertionError: %d == %d\n"), actual, expected);
	printf(GREEN("    + expected ") RED("- actual\n\n"));
	printf(RED("    - %d\n"), actual);
	printf(RED("    + %d\n\n"), expected);
}

void it_b(const char *name, router_boolean_t actual, router_boolean_t expected)
{
	const char *actual_str = actual ? "true" : "false";
	const char *expected_str = expected ? "true" : "false";

	tests_total++;
	if (!actual == !expected) {
		printf(GREEN("  √ ") "%s\n", name);
		tests_passed++;
		return;
	}
	printf(RED("  × %s\n"), name);
	printf(RED("    AssertionError: %s == %s\n"), actual_str, expected_str);
	printf(GREEN("    + expected ") RED("- actual\n\n"));
	printf(RED("    - %s\n"), actual_str);
	printf(GREEN("    + %s\n\n"), expected_str);
}

void it_s(const char *name, const char *actual, const char *expected)
{
	tests_total++;
	if ((actual && expected && strcmp(actual, expected) == 0) ||
	    actual == expected) {
		printf(GREEN("  √ ") "%s == '%s'\n", name, expected);
		tests_passed++;
		return;
	}
	printf(RED("   × %s == '%s'\n"), name, expected);
	if (expected) {
		printf(RED("    AssertionError: '%s' == '%s'\n"), actual, expected);
	} else {
		printf(RED("    AssertionError: '%s' == null\n"), actual);
	}
	printf(GREEN("    + expected ") RED("- actual\n\n"));
	printf(RED("    - %s\n"), actual);
	printf(GREEN("    + %s\n\n"), expected);
}

void test_router_location(void)
{
	router_location_t *raw;
	router_location_t *location;
	const char *str;

	raw = router_location_create(NULL, "/search?type=issue&order=desc");
	location = router_location_normalize(raw, NULL, FALSE);

	str = router_string_dict_get(location->query, "type");
	it_s("location.query.type", str, "issue");

	str = router_string_dict_get(location->query, "order");
	it_s("location.query.order", str, "desc");

	str = location->path;
	it_s("location.path", str, "/search");

	router_location_destroy(raw);
	router_location_destroy(location);

	raw = router_location_create(NULL, "/search?type=issue#pagination");
	location = router_location_normalize(raw, NULL, FALSE);

	str = location->hash;
	it_s("location.hash", str, "#pagination");

	router_location_destroy(raw);
	router_location_destroy(location);
}

void test_router_route(void)
{
	router_route_t *route;
	router_location_t *location;
	router_location_t *raw;
	const char *full_path =
	    "/user/profile?tab=repos&order=desc&q=test#pagination";

	raw = router_location_create(NULL, full_path);
	location = router_location_normalize(raw, NULL, FALSE);
	route = router_route_create(NULL, location);

	it_s("route.path", router_route_get_path(route), "/user/profile");
	it_s("route.query.q", router_route_get_query(route, "q"), "test");
	it_s("route.query.tab", router_route_get_query(route, "tab"), "repos");
	it_s("route.query.order", router_route_get_query(route, "order"),
	     "desc");
	it_s("route.query.other", router_route_get_query(route, "other"), NULL);
	it_s("route.hash", router_route_get_hash(route), "#pagination");
	it_s("route.fullPath", router_route_get_full_path(route), full_path);
	router_location_destroy(raw);
	router_location_destroy(location);
	router_route_destroy(route);
}

void test_router_matcher(void)
{
	router_t *router;
	router_config_t *config;
	router_route_t *route;
	router_route_record_t *route_user_show;
	router_resolved_t *resolved;
	router_location_t *location;
	const router_route_record_t *record;
	const char *str;

	router = router_create(NULL);
	config = router_config_create();
	router_config_set_path(config, "/home");
	router_config_set_component(config, NULL, "home");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/files");
	router_config_set_component(config, NULL, "files");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/files/*");
	router_config_set_component(config, NULL, "files");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/users");
	router_config_set_component(config, NULL, "user-index");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/users/:username");
	router_config_set_component(config, NULL, "user-show");
	route_user_show = router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "");
	router_config_set_component(config, NULL, "user-overview");
	router_add_route_record(router, config, route_user_show);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "profile");
	router_config_set_component(config, NULL, "user-profile");
	router_add_route_record(router, config, route_user_show);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "posts");
	router_config_set_name(config, "user#posts");
	router_config_set_component(config, NULL, "user-posts");
	router_add_route_record(router, config, route_user_show);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/about");
	router_config_set_component(config, NULL, "about");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "*");
	router_config_set_component(config, NULL, "not-found");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	location = router_location_create(NULL, "/users/root");
	resolved = router_resolve(router, location, FALSE);
	router_location_destroy(location);
	route = router_resolved_get_route(resolved);
	str = router_route_get_param(route, "username");
	it_s("match('/users/root').route.params.username", str, "root");
	router_resolved_destroy(resolved);

	location = router_location_create("user#posts", NULL);
	resolved = router_resolve(router, location, FALSE);
	it_b("match({ name: 'user#posts' })", !resolved, FALSE);
	router_location_destroy(location);
	router_resolved_destroy(resolved);

	location = router_location_create(NULL, "/users/root");
	router_push(router, location);
	router_location_destroy(location);
	location = router_location_create("user#posts", NULL);
	resolved = router_resolve(router, location, FALSE);
	it_b("match({ name: 'user#posts' })", !!resolved, TRUE);
	route = router_resolved_get_route(resolved);
	str = router_route_get_param(route, "username");
	it_s("match({ name: 'user#posts' }).route.params.username",
	     router_route_get_param(route, "username"), "root");
	it_s("match({ name: 'user#posts' }).route.fullPath",
	     router_route_get_full_path(route), "/users/root/posts");
	router_location_destroy(location);
	router_resolved_destroy(resolved);

	location = router_location_create(NULL, "/users/root/posts");
	resolved = router_resolve(router, location, FALSE);
	router_location_destroy(location);
	route = router_resolved_get_route(resolved);
	record = router_route_get_matched_record(route, 0);
	it_b("match('/users/root/posts').route.matched[0] != null", !!record,
	     TRUE);
	it_s("match('/users/root/posts').route.matched[0].components.default",
	     record ? router_route_record_get_component(record, NULL) : NULL,
	     "user-show");
	route = router_resolved_get_route(resolved);
	record = router_route_get_matched_record(route, 1);
	it_b("match('/users/root/posts').route.matched[1] != null", !!record,
	     TRUE);
	it_s("match('/users/root/posts').route.matched[1].components.default",
	     record ? router_route_record_get_component(record, NULL) : NULL,
	     "user-posts");
	router_resolved_destroy(resolved);

	location = router_location_create(NULL, "/other/path/to/file");
	resolved = router_resolve(router, location, FALSE);
	router_location_destroy(location);
	route = router_resolved_get_route(resolved);
	record = router_route_get_matched_record(route, 0);
	it_b("match('/other/path/to/file').route.matched[0] != null", !!record,
	     TRUE);
	it_s("match('/other/path/to/file').route.matched[0].components.default",
	     record ? router_route_record_get_component(record, NULL) : NULL,
	     "not-found");
	it_s("match('/other/path/to/file').route.params.pathMatch",
	     route ? router_route_get_param(route, "pathMatch") : NULL,
	     "other/path/to/file");
	router_resolved_destroy(resolved);

	location = router_location_create(NULL, "/files/path/to/file");
	resolved = router_resolve(router, location, FALSE);
	router_location_destroy(location);
	route = router_resolved_get_route(resolved);
	record = router_route_get_matched_record(route, 0);
	it_s("match('/files/path/to/file').route.matched[0].components.default",
	     record ? router_route_record_get_component(record, NULL) : NULL,
	     "files");
	it_s("match('/files/path/to/file').route.params.pathMatch",
	     route ? router_route_get_param(route, "pathMatch") : NULL,
	     "path/to/file");
	router_resolved_destroy(resolved);

	router_destroy(router);
}

void test_router_utils(void)
{
	char *str;
	char *p;
	char key[256];
	size_t key_len;
	router_string_dict_t *a;
	router_string_dict_t *b;
	router_string_dict_t *params;

	a = router_string_dict_create();
	b = router_string_dict_create();
	router_string_dict_set(a, "id", "404");
	router_string_dict_set(b, "id", "404");
	router_string_dict_set(a, "name", "git");
	router_string_dict_set(b, "name", "git");

	it_b("isObjectEqual({ id: '404', name: 'git' }, { id: '404', name: "
	     "'git' })",
	     router_string_dict_equal(a, b), TRUE);

	router_string_dict_set(b, "id", "200");
	it_b("isObjectEqual({ id: '404', name: 'git' }, { id: '200', name: "
	     "'git' })",
	     router_string_dict_equal(a, b), FALSE);

	router_string_dict_delete(b, "id");
	it_b("isObjectIncludes({ id: '404', name: 'git' }, { name: 'git' })",
	     router_string_dict_includes(a, b), TRUE);
	router_string_dict_delete(a, "name");
	it_b("isObjectIncludes({ id: '404' }, { name: '200' })",
	     router_string_dict_includes(a, b), FALSE);
	router_string_dict_delete(a, "id");
	router_string_dict_delete(b, "name");
	it_b("isObjectEqual({}, {})", router_string_dict_equal(a, b), TRUE);
	it_i("string.compare('', '')", router_string_compare("", ""), 0);
	it_b("string.compare(null, 'a') != 0",
	     router_string_compare(NULL, "a") != 0, TRUE);
	it_i("string.compare(null, null)", router_string_compare(NULL, NULL),
	     0);
	router_string_dict_destroy(a);
	router_string_dict_destroy(b);

	str = router_path_resolve("", NULL, TRUE);
	it_s("path.resolve('', null, true) == '/'", str, "/");
	free(str);

	str = router_path_resolve("", "/root", TRUE);
	it_s("path.resolve('', '/root', true)", str, "/root");
	free(str);

	str = router_path_resolve("hello/../world/./", NULL, TRUE);
	it_s("path.resolve('hello/../world/./', null, true)", str, "/world");
	free(str);

	str = router_path_resolve("/root/path", "base/path", TRUE);
	it_s("path.resolve('/root/path', 'base/path', true)", str,
	     "/root/path");
	free(str);

	str = router_path_resolve("../../profile", "base/path/to/file", TRUE);
	it_s("path.resolve('../../profile', 'base/path/to/file', true)", str,
	     "/base/path/profile");
	free(str);

	str = router_path_resolve("profile", "base/file", FALSE);
	it_s("path.resolve('profile', 'base/profile', false)", str,
	     "/base/profile");
	free(str);

	str = router_path_resolve("/profile", "base/file", FALSE);
	it_s("path.resolve('/profile', 'base/profile', false)", str,
	     "/profile");
	free(str);

	it_b("path.compare('/a/b', '/a/b/')",
	     router_path_compare("/a/b", "/a/b/"), TRUE);
	it_b("path.compare('/a/b/', '/a/b/')",
	     router_path_compare("/a/b/", "/a/b"), TRUE);
	it_b("path.compare('', '')", router_path_compare("", "") == 0, TRUE);
	it_b("path.compare('a', '')", router_path_compare("a", "") != 0, TRUE);
	it_b("path.compare('', 'b')", router_path_compare("", "b") != 0, TRUE);
	it_b("path.compare('a', 'b')", router_path_compare("a", "b") != 0,
	     TRUE);

	it_b("path.startsWith('/profile/events', '/profile/event')",
	     router_path_starts_with("/profile/events", "/profile/event"),
	     FALSE);
	it_b("path.startsWith('/profile/events', '/profile/')",
	     router_path_starts_with("/profile/events", "/profile/"), TRUE);
	it_b("path.startsWith('/profile', '/profile/')",
	     router_path_starts_with("/profile", "/profile/"), TRUE);
	it_b("path.startsWith('/profile/', '/profile')",
	     router_path_starts_with("/profile/", "/profile"), TRUE);

	p = str = "/:username/:repo/settings";
	p = router_path_parse_key(p, key, &key_len);
	it_s("path.keys('/:username/:repo/settings')[0].key", key, "username");
	it_i("path.keys('/:username/:repo/settings')[0].index", p ? p - str : 0,
	     11);
	p = router_path_parse_key(p, key, &key_len);
	it_s("path.keys('/:username/:repo/settings')[1].key", key, "repo");
	it_i("path.keys('/:username/:repo/settings')[1].index", p ? p - str : 0,
	     17);
	p = router_path_parse_key(p, key, &key_len);
	it_s("path.keys('/:username/:repo/settings')[2].key", key, "");
	it_i("path.keys('/:username/:repo/settings')[2].index",
	     p ? p - str : -1, -1);

	params = router_string_dict_create();
	router_string_dict_set(params, "1", "one");
	router_string_dict_set(params, "2", "two");
	router_string_dict_set(params, "3", "three");
	router_string_dict_set(params, "word", "good");
	str = router_path_fill_params("/:1/:2/:3/foo", params);
	it_s("path.fillParams('/:1/:2/:3/foo', { 1: 'one', 2: 'two', 3: "
	     "'three' })",
	     str, "/one/two/three/foo");
	free(str);

	str = router_path_fill_params("/:1:2:3/foo", params);
	it_s("path.fillParams('/:1:2:3/foo', { 1: 'one', 2: 'two', 3: "
	     "'three' })",
	     str, NULL);

	router_string_dict_destroy(params);
	str = router_path_fill_params("/foo/bar", NULL);
	it_s("path.fillParams('/foo/bar')", str, "/foo/bar");
	free(str);
}

void test_router_components(void)
{
	router_t *router;
	router_config_t *config;
	LCUI_Widget root;
	LCUI_Widget link_foo;
	LCUI_Widget link_foobar;
	LCUI_Widget link_bar;
	LCUI_Widget view;
	LCUI_Widget matched_widget;
	LCUI_WidgetEventRec e;

	router = router_create(NULL);
	config = router_config_create();
	router_config_set_path(config, "/foo");
	router_config_set_component(config, NULL, "foo");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/foo/bar");
	router_config_set_component(config, NULL, "foobar");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	config = router_config_create();
	router_config_set_path(config, "/bar");
	router_config_set_component(config, NULL, "bar");
	router_add_route_record(router, config, NULL);
	router_config_destroy(config);

	LCUI_Init();
	LCUIWidget_NewPrototype("foo", NULL);
	LCUIWidget_NewPrototype("foobar", NULL);
	LCUIWidget_NewPrototype("bar", NULL);
	UI_InitRouterLink();
	UI_InitRouterView();
	root = LCUIWidget_GetRoot();
	view = LCUIWidget_New("router-view");
	link_foo = LCUIWidget_New("router-link");
	link_foobar = LCUIWidget_New("router-link");
	link_bar = LCUIWidget_New("router-link");
	Widget_SetAttribute(link_foo, "to", "/foo");
	Widget_SetAttribute(link_foobar, "to", "/foo/bar");
	Widget_SetAttribute(link_bar, "to", "/bar");
	Widget_Append(root, link_foo);
	Widget_Append(root, link_foobar);
	Widget_Append(root, link_bar);
	Widget_Append(root, view);
	LCUIWidget_Update();

	LCUI_InitWidgetEvent(&e, "click");
	Widget_TriggerEvent(link_foo, &e, NULL);
	matched_widget = RouterView_GetMatchedWidget(view);
	it_b("[/foo] <router-view> widget should load <foo> widget",
	     strcmp(matched_widget->type, "foo") == 0, TRUE);
	it_b("[/foo] linkFoo should has active classes",
	     (Widget_HasClass(link_foo, "router-link-active") &&
	      Widget_HasClass(link_foo, "router-link-exact-active")),
	     TRUE);
	Widget_TriggerEvent(link_bar, &e, NULL);
	matched_widget = RouterView_GetMatchedWidget(view);
	it_b("[/bar] <router-view> widget should load <bar> widget",
	     strcmp(matched_widget->type, "bar") == 0, TRUE);
	it_b("[/bar] linkFoo should not has active classes",
	     (!Widget_HasClass(link_foo, "router-link-exact-active") &&
	      !Widget_HasClass(link_foo, "router-link-active")),
	     TRUE);

	Widget_TriggerEvent(link_foobar, &e, NULL);
	matched_widget = RouterView_GetMatchedWidget(view);
	it_b("[/foo/bar] <router-view> widget should load <foobar> widget",
	     strcmp(matched_widget->type, "foobar") == 0, TRUE);
	it_b("[/foo/bar] linkFooBar should has active classes",
	     (Widget_HasClass(link_foobar, "router-link-exact-active") &&
	      Widget_HasClass(link_foobar, "router-link-active")),
	     TRUE);
	it_b("[/foo/bar] linkFoo should only has the exact active class",
	     (!Widget_HasClass(link_foo, "router-link-exact-active") &&
	      Widget_HasClass(link_foo, "router-link-active")),
	     TRUE);
	Widget_SetAttribute(link_foo, "exact", "exact");
	Widget_TriggerEvent(link_foobar, &e, NULL);
	it_b("[/foo/bar] linkFoo should not has any active classes (exact)",
	     (!Widget_HasClass(link_foo, "router-link-exact-active") &&
	      !Widget_HasClass(link_foo, "router-link-active")),
	     TRUE);

	LCUI_Destroy();
	router_destroy(router);
}

int main(void)
{
	Logger_SetLevel(LOGGER_LEVEL_OFF);
	describe("router utils", test_router_utils);
	describe("router location", test_router_location);
	describe("router route", test_router_route);
	describe("router matcher", test_router_matcher);
	describe("router components", test_router_components);
	printf("\n%zu tests, %zu passed.\n", tests_total, tests_passed);
	return tests_total - tests_passed;
}
