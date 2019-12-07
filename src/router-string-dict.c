#include "router.h"

static void *router_string_dict_val_dup(void *privdata, const void *val)
{
	return strdup(val);
}

static void router_string_dict_val_free(void *privdata, void *val)
{
	free(val);
}

router_string_dict_t *router_string_dict_create(void)
{
	static DictType type;

	type = DictType_StringCopyKey;
	type.valDup = router_string_dict_val_dup;
	type.valDestructor = router_string_dict_val_free;
	return Dict_Create(&type, NULL);
}

void router_string_dict_destroy(router_string_dict_t *dict)
{
	Dict_Release(dict);
}

void router_string_dict_delete(router_string_dict_t *dict, const char *key)
{
	Dict_Delete(dict, key);
}

int router_string_dict_set(router_string_dict_t *dict, const char *key,
			   const char *value)
{
	if (Dict_Add(dict, (void *)key, (void *)value) == 0) {
		return 0;
	}
	Dict_Delete(dict, (void *)key);
	return Dict_Add(dict, (void *)key, (void *)value);
}

const char *router_string_dict_get(router_string_dict_t *dict, const char *key)
{
	return Dict_FetchValue(dict, key);
}

size_t router_string_dict_extend(router_string_dict_t *target,
				 router_string_dict_t *other)
{
	size_t count = 0;
	DictEntry *entry;
	DictIterator *iter;

	if (!other) {
		return count;
	}
	iter = Dict_GetIterator(other);
	while ((entry = Dict_Next(iter))) {
		router_string_dict_set(target, entry->key, entry->v.val);
		++count;
	}
	Dict_ReleaseIterator(iter);
	return count;
}

router_string_dict_t *router_string_dict_duplicate(router_string_dict_t *target)
{
	router_string_dict_t *dict;

	dict = router_string_dict_create();
	if (target) {
		router_string_dict_extend(dict, target);
	}
	return dict;
}

router_boolean_t router_string_dict_includes(router_string_dict_t *a,
					     router_string_dict_t *b)
{
	DictEntry *entry;
	DictIterator *iter;
	const char *value;

	iter = Dict_GetIterator(b);
	while ((entry = Dict_Next(iter))) {
		value = Dict_FetchValue(a, entry->key);
		if (!value || strcmp(value, DictEntry_GetVal(entry)) != 0) {
			Dict_ReleaseIterator(iter);
			return FALSE;
		}
	}
	Dict_ReleaseIterator(iter);
	return TRUE;
}

router_boolean_t router_string_dict_equal(router_string_dict_t *a,
					  router_string_dict_t *b)
{
	if (Dict_Size(a) != Dict_Size(b)) {
		return FALSE;
	}
	return router_string_dict_includes(a, b);
}
