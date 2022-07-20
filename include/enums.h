#ifndef _ENUMS_H_
#define _ENUMS_H_

typedef enum
{
	BUTTON_LAYOUT_ARCADE,
	BUTTON_LAYOUT_HITBOX,
	BUTTON_LAYOUT_WASD,
} ButtonLayout;

typedef enum
{
	CONFIG_TYPE_WEB = 0,
	CONFIG_TYPE_SERIAL,
	CONFIG_TYPE_DISPLAY
} ConfigType;

#endif