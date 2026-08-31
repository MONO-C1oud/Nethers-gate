#include "NethersGateGUI.h"
#include <QtWidgets/QApplication>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NethersGateGUI w;
    w.show();
    return a.exec();
}
