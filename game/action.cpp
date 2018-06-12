#include "action.h"
#include "debug.h"

using namespace std;

void Action::disabled(const char *func, int line) {
    disable = true;
#ifdef  _DEBUG
    log("Disable Action Player:%d, fire(%d,%d,%d), move(%d,%d), priority(%d,%d,%d,%d).[%s,%d]\n",
            player_id,
            fire.col, fire.row, _is_super_bullet,
            move.col, move.row,
            priEscape, priDiamon, priDeadEnd,priSiege,
            func, line
            );
#endif
}
