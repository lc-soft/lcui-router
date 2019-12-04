#include "router.h"

const char *router_path_parse_key(const char *path, char key[256],
				  size_t *key_len)
{
	const char *p;

	if (!path || !path[0]) {
		return NULL;
	}
	for (*key_len = 0, p = path;; ++p) {
		if (*p == '/' || *p == 0) {
			key[*key_len] = 0;
			if (*key_len < 2) {
				*key_len = 0;
				if (!*p) {
					break;
				}
				continue;
			}
			return p;
		} else if (*key_len > 0) {
			key[*key_len - 1] = *p;
			++*key_len;
		} else if (*p == ':') {
			*key_len = 1;
		}
	}
	return p;
}

size_t router_path_parse_keys(const char *path, router_linkedlist_t *keys)
{
	const char *next;
	const char *prev;
	size_t key_len;
	char key[256];

	next = prev = path;
	while ((next = router_path_parse_key(path, key, &key_len))) {
		if (key_len < 1) {
			break;
		}
		LinkedList_Append(keys, strdup(key));
	}
	return keys->length;
}

char *router_path_fill_params(const char *path, router_string_dict_t *params)
{
	const char *next;
	const char *prev;
	const char *value;
	char key[256];
	char *full_path;
	size_t key_len;
	size_t value_len;
	size_t full_path_len;
	size_t i = 0;

	full_path_len = strlen(path) + 1;
	full_path = malloc(sizeof(char) * full_path_len);
	for (prev = next = path;;
	     next = router_path_parse_key(next, key, &key_len)) {
		value_len = next - key_len - prev + 1;
		strncpy(full_path + i, prev, value_len);
		i += value_len;
		if (key_len < 1) {
			break;
		}
		value = Dict_FetchValue(params, key);
		if (!value) {
			Logger_Error(
			    "can not match parameter value by key: \"%s\"",
			    key);
			free(full_path);
			return NULL;
		}
		value_len = strlen(value);
		full_path_len += value_len;
		full_path = realloc(full_path, sizeof(char) * full_path_len);
		strcpy(full_path + i, value);
		i += value_len;
	}
	full_path[i] = 0;
	return full_path;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L401

char *router_path_resolve(const char *relative, const char *base,
			  router_boolean_t append)
{
	char *p;
	char *q;
	char *path;
	char **stack;
	char **segments;
	const char first_char = relative[0];

	size_t i;
	size_t stack_size;
	size_t segments_size;

	if (first_char == '/') {
		return strdup(relative);
	}
	if (!base || !base[0]) {
		base = "/";
	}
	path = malloc(strlen(relative) + strlen(base) + 4);
	if (first_char == '?' || first_char == '#') {
		return strcat(strcpy(path, base), relative);
	}
	stack_size = strsplit(base, "/", &stack);
	if (!append || !stack[stack_size - 1][0]) {
		stack_size--;
		router_mem_free(stack[stack_size]);
	}
	segments_size = strsplit(relative, "/", &segments);
	for (i = 0; i < segments_size; ++i) {
		if (strcmp(segments[i], "..") == 0) {
			stack_size--;
			router_mem_free(segments[i]);
			router_mem_free(stack[stack_size]);
		} else if (strcmp(segments[i], ".") != 0) {
			stack_size++;
			stack = realloc(stack, sizeof(char *) * stack_size);
			stack[stack_size - 1] = segments[i];
		} else {
			router_mem_free(segments[i]);
		}
	}
	path[0] = '/';
	path[1] = 0;
	for (i = 0, p = path; i < stack_size; ++i) {
		if (stack[i][0]) {
			*p = '/';
			p++;
			for (q = stack[i]; *q; ++q, ++p) {
				*p = *q;
			}
			*p = 0;
		}
		free(stack[i]);
	}
	free(stack);
	free(segments);
	return path;
}

// https://github.com/vuejs/vue-router/blob/65de048ee9f0ebf899ae99c82b71ad397727e55d/dist/vue-router.esm.js#L202

router_string_dict_t *router_parse_query(const char *query_str)
{
	char **pairs;
	size_t i;
	size_t split_i;
	size_t pairs_size;
	router_string_dict_t *query;

	query = router_string_dict_create();
	if (!query_str) {
		return query;
	}
	pairs_size = strsplit(query_str, "&", &pairs);
	for (i = 0; i < pairs_size; ++i) {
		for (split_i = 0; pairs[i][split_i]; ++split_i) {
			if (pairs[i][split_i] == '=') {
				pairs[i][split_i] = 0;
				break;
			}
		}
		router_string_dict_set(query, pairs[i], pairs[i] + split_i + 1);
		router_mem_free(pairs[i]);
	}
	free(pairs);
	return query;
}
