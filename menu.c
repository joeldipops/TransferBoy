#include "utils.h"
#include "menu.h"
#include <libdragon.h>

/**
 * Draws the menu including current cursor position.
 * @param cursorPosition which menu item is currently selected.
 */
void menuRender(int cursorPosition)
{
    const natural CURSOR_OFFSET = 10;
    static display_context_t frame = null;
    while (!(frame = display_lock()));

    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);
    graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);

    graphics_draw_text(frame, HORIZONTAL_MARGIN + CURSOR_OFFSET, VERTICAL_MARGIN + VERTICAL_MENU_SPACING * 0, "Start");
    graphics_draw_text(frame, HORIZONTAL_MARGIN + CURSOR_OFFSET, VERTICAL_MARGIN + VERTICAL_MENU_SPACING * 1, "Options");
    graphics_draw_text(frame, HORIZONTAL_MARGIN + CURSOR_OFFSET, VERTICAL_MARGIN + VERTICAL_MENU_SPACING * 2, "Exit");
    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN + VERTICAL_MENU_SPACING * cursorPosition, ">");

    display_show(frame);
}

/**
 * Shows the menu and allows selection of items
 * @param controllerState The full state of the controller.
 * @param buttons Array of buttons currently pressed.
 * @return State representing selected menu item.
 */
MenuState menuLoop() {
    natural cursorPosition = 0;
    int cursorMax = 2;

    MenuState state = MenuStatePending;

    menuRender(cursorPosition);
    while (state == MenuStatePending) {
        controller_scan();
        struct controller_data buttons = get_keys_pressed();

        bool aPressed = buttons.c[0].A;
        bool startPressed = buttons.c[0].start;
        bool downPressed = buttons.c[0].down;
        bool upPressed = buttons.c[0].up;

        if (aPressed || startPressed) {
            switch(cursorPosition) {
                case 0: state = MenuStateStart; break;
                case 1: state = MenuStateOptions; break;
                case 2: state = MenuStateExit; break;
                default: break;
            }
        }

        if (upPressed) {
            if (cursorPosition > 0) {
                cursorPosition--;
            } else {
                cursorPosition = cursorMax;
            }
        }
        if (downPressed) {
            if (cursorPosition < cursorMax) {
                cursorPosition++;
            } else {
                cursorPosition = 0;
            }
        }

        if (aPressed || startPressed || upPressed || downPressed) {
            menuRender(cursorPosition);
        }
    }

    return state;
};
