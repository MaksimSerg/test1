#ifndef APPSETTINGS_H
#define APPSETTINGS_H


#include <QSettings>
#include <QStandardPaths>
#include <QDebug>


#define APP_SETTINGS_FILE "/app.ini"

class AppSettings: public QSettings
{
    public:
        static AppSettings& getInstance()
        {
            QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            static AppSettings instance(path + APP_SETTINGS_FILE);

            return instance;
        }
    private:
        AppSettings(QString aPath) :
            QSettings(aPath, QSettings::IniFormat)
        { }
    public:
        AppSettings(AppSettings const&)     = delete;
        void operator=(AppSettings const&)  = delete;
};

#endif // APPSETTINGS_H
