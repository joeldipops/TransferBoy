#ifndef MENU_INCLUDED
#define MENU_INCLUDED

typedef enum { MenuStatePending, MenuStateExit, MenuStateStart, MenuStateOptions } MenuState;
MenuState menuLoop();

#endif
