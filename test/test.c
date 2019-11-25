#include "../src/router.h"
#include "../src/router.c"
#include "../src/router-history.c"
#include "../src/router-matcher.c"
#include "../src/router-config.c"
#include "../src/router-location.c"
#include "../src/router-route-record.c"
#include "../src/router-route.c"
#include "../src/router-string-dict.c"
#include "../src/router-utils.c"

static size_t tests_passed = 0;
static size_t tests_total = 0;

#define describe(NAME, FUNC)       \
	Logger_Info("%s\n", NAME); \
	FUNC();

#define it(NAME, ACTUAL, EXCEPT)                  \
	tests_total++;                            \
	if ((ACTUAL) == (EXCEPT)) {               \
		Logger_Info("    √ %s\n", NAME);  \
		tests_passed++;                   \
	} else {                                  \
		Logger_Error("    × %s\n", NAME); \
	}

void test_router_location(void)
{
	router_location_t *raw;
	router_location_t *location;
	const char *str;

	raw = router_location_create(NULL, "/search?type=issue&order=desc");
	location = router_location_normalize(raw, NULL);

	str = router_string_dict_get(location->query, "type");
	it("location.query.type == 'issue'", strcmp(str, "issue"), 0);
	str = router_string_dict_get(location->query, "order");
	it("location.query.order == 'desc'", strcmp(str, "desc"), 0);
	str = location->path;
	it("location.path == '/search'", strcmp(str, "/search"), 0);

	router_location_destroy(raw);
	router_location_destroy(location);

	raw = router_location_create(NULL, "/search?type=issue#pagination");
	location = router_location_normalize(raw, NULL);

	str = location->hash;
	it("location.hash == '#pagination'", strcmp(str, "#pagination"), 0);

	router_location_destroy(raw);
	router_location_destroy(location);
}

int main(void)
{
	describe("router location", test_router_location);
	Logger_Info("\n%u tests, %u passed.\n", tests_total, tests_passed);
	return tests_total - tests_passed;
}
