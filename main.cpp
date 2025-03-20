#include "tooltip-manager.h"

#include <QDebug>
#include <QTimer>
#include <unistd.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int tim = 200;
    QTimer tm;
    tm.connect(&tm, &QTimer::timeout, [&] () {
        static int i = 60;
        --i;
        ToolTipManager::getInstance()->showMessage(QString("asdasdasdas的敖德萨手动阿萨德阿萨德啊大大手动阿萨德啊手动啊大的啊的阿萨德啊手动安得按时的啊大的啊但是"
                                                           "<a href='https://www.baidu.com'>sssssssssssssssssssssss</a>%1").arg(i));
        // if (i > 0 && i % 6  == 0) {
            // tim *= 2;
            // tm.setInterval(tim);
        // }
        if (!i) {tm.stop();}
    });
    tm.setInterval(tim);
    tm.start();

    return a.exec();
}
