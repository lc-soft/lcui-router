#ifndef LCUI_ROUTER_LINK_H
#define LCUI_ROUTER_LINK_H

#include <LCUI/gui/widget.h>

void RouterLink_SetLocation(LCUI_Widget w, router_location_t *location);

void RouterLink_SetExact(LCUI_Widget w, router_boolean_t exact);

void UI_InitRouterLink(void);

#endif
