#include "router.h"

router_config_t *router_config_create(void)
{
	router_config_t *config;

	config = malloc(sizeof(router_config_t));
	config->components = router_string_dict_create();
	config->name = NULL;
	config->path = NULL;
	return config;
}

void router_config_destroy(router_config_t *config)
{
	router_mem_free(config->name);
	router_mem_free(config->path);
	router_string_dict_destroy(config->components);
	config->components = NULL;
	free(config);
}

void router_config_set_name(router_config_t *config, const char *name)
{
	router_mem_free(config->name);
	if (name) {
		config->name = strdup(name);
	}
}

void router_config_set_path(router_config_t *config, const char *path)
{
	router_mem_free(config->path);
	if (path) {
		config->path = strdup(path);
	}
}

void router_config_set_component(router_config_t *config, const char *name,
				 const char *component)
{
	if (!name) {
		name = "default";
	}
	router_string_dict_set(config->components, name, component);
}
