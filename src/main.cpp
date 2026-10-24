/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: MIT, see LICENSE
# ==========================================================================*/

#include <QtGui/QApplication>
#include "culistgui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CulistGui w;
    w.show();
    
    return a.exec();
}
