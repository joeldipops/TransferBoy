#include "rsp.h"
#include "logger.h"

void ppuStep(PlayerState* state) {
    if (isRspBusy()) {
        return;
    }
}