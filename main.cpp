#include "src/mainwindow.h"
#include "src/appsettings.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

Q_DECLARE_METATYPE(QVector<Point>)

int main(int argc, char *argv[])
{

    qRegisterMetaType<QVector<Point>>();

    QApplication a(argc, argv);
    MainWindow w;

    AppSettings& appset = AppSettings::getInstance();
    QVariant vGeometry = appset.value("Windows.primary/geometry");
    QVariant vFullscreen = appset.value("Windows.primary/fullscreen");

    bool bFullscreen = false;
    if (vFullscreen.canConvert<bool>())
    {
        bFullscreen = qvariant_cast<bool>(vFullscreen);
    }

    if (bFullscreen)
    {
        w.showMaximized();
    }
    else
    {
        if (vGeometry.canConvert<QRect>())
        {
            /* Checking show window in currents monitors
             * if 'true' - set size and position
             * else - open default size
             */
            QRect rGeometry = qvariant_cast<QRect>(vGeometry);
            if (rGeometry.width()>0 && rGeometry.height()>0)
            {
                QList<QScreen *> screens = QGuiApplication::screens();
                for (QScreen *screen: screens)
                {
                    QRect screenRect = screen->availableGeometry();
                    QRect iRect = rGeometry.intersected(screenRect);
                    if (iRect.isValid())
                    {
                        w.setGeometry(rGeometry);
                        break;
                    }
                }
            }
        }

        w.show();
    }

    return a.exec();
}
