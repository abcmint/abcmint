#include "init.h"
#include "noui.h"
#include "util.h"

#if !defined(QT_GUI)
int main(int argc, char* argv[]) {
    bool fRet = false;

    // Connect  Abcmint signal handlers
    noui_connect();

    fRet = AppInit(argc, argv);

    if (fRet && fDaemon) return 0;

    return (fRet ? 0 : 1);
}
#endif
