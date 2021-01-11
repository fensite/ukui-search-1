#include <QtConcurrent>
#include <QApplication>
#include <QPalette>
#include "global-settings.h"

static GlobalSettings *global_instance_of_global_settings = nullptr;

GlobalSettings *GlobalSettings::getInstance()
{
    if (!global_instance_of_global_settings) {
        global_instance_of_global_settings = new GlobalSettings;
    }
    return global_instance_of_global_settings;
}

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings("org.ukui", "ukui-search", this);
    m_block_dirs_settings = new QSettings("org.ukui","ukui-search-block-dirs",this);
    this->forceSync();
    //the default number of transparency in mainwindow is 0.7
    //if someone changes the num in mainwindow, here should be modified too
    m_cache.insert(TRANSPARENCY_KEY, 0.7);
    if (QGSettings::isSchemaInstalled(CONTROL_CENTER_PERSONALISE_GSETTINGS_ID)) {
        m_gsettings = new QGSettings(CONTROL_CENTER_PERSONALISE_GSETTINGS_ID, QByteArray(), this);
        connect(m_gsettings, &QGSettings::changed, this, [=](const QString& key) {
            if (key == TRANSPARENCY_KEY) {
                m_cache.remove(TRANSPARENCY_KEY);
                m_cache.insert(TRANSPARENCY_KEY, m_gsettings->get(TRANSPARENCY_KEY).toDouble());
                qApp->paletteChanged(qApp->palette());
            }
        });
    }
    m_cache.remove(TRANSPARENCY_KEY);
    m_cache.insert(TRANSPARENCY_KEY, m_gsettings->get(TRANSPARENCY_KEY).toDouble());

}

GlobalSettings::~GlobalSettings()
{

}

const QVariant GlobalSettings::getValue(const QString &key)
{
    return m_cache.value(key);
}

bool GlobalSettings::isExist(const QString &key)
{
    return !m_cache.value(key).isNull();
}

void GlobalSettings::reset(const QString &key)
{
    m_cache.remove(key);
    QtConcurrent::run([=]() {
        if (m_mutex.tryLock(1000)) {
            m_settings->remove(key);
            m_settings->sync();
            m_mutex.unlock();
        }
    });
    Q_EMIT this->valueChanged(key);
}

void GlobalSettings::resetAll()
{
    QStringList tmp = m_cache.keys();
    m_cache.clear();
    for (auto key : tmp) {
        Q_EMIT this->valueChanged(key);
    }
    QtConcurrent::run([=]() {
        if (m_mutex.tryLock(1000)) {
            m_settings->clear();
            m_settings->sync();
            m_mutex.unlock();
        }
    });
}

bool GlobalSettings::setBlockDirs(const QString &path, QString &returnMessage, bool remove)
{
    //why QSetting's key can't start with "/"??
    QString pathKey = path.right(path.length()-1);
    if(remove)
    {
        m_block_dirs_settings->remove(pathKey);
        return true;
    }
    QStringList blockDirs = m_block_dirs_settings->allKeys();
    for(QString i:blockDirs)
    {
        if(pathKey.startsWith(i))
        {
            returnMessage = QString(tr("Parent folder has been blocked!"));
            return false;
        }

        if(i.startsWith(pathKey))
            m_block_dirs_settings->remove(i);
    }
    m_block_dirs_settings->setValue(pathKey,"0");
    return true;
}

QStringList GlobalSettings::getBlockDirs()
{
    return m_block_dirs_settings->allKeys();
}

//here should be override
//MouseZhangZh
void GlobalSettings::setValue(const QString &key, const QVariant &value)
{
    m_cache.insert(key, value);
    QtConcurrent::run([=]() {
//        if (m_mutex.tryLock(1000)) {
        m_mutex.lock();
            m_settings->setValue(key, value);
            m_settings->sync();
            m_mutex.unlock();
//        }
    });
}

void GlobalSettings::forceSync(const QString &key)
{
    m_settings->sync();
    if (key.isNull()) {
        m_cache.clear();
        for (auto key : m_settings->allKeys()) {
            m_cache.insert(key, m_settings->value(key));
        }
    } else {
        m_cache.remove(key);
        m_cache.insert(key, m_settings->value(key));
    }
}
