#include <stdio.h>
#include <LCUI.h>
#include <lcui-router.h>

int main(void)
{
	router_t *router;

	router = router_create(NULL);
	LCUI_Init();
	return LCUI_Main();
}
