#ifndef BASICDOWNLOADER_H_
#define BASICDOWNLOADER_H_

#include <QObject>
#include <QQuickItem>
#include <QQuickWebEngineProfile>

#include <map>

class BasicDownloader : public QObject
{
    Q_OBJECT
public:
    struct DownloadMetadata
    {
        long long int received;
        long long int total;
        QString path;
        bool finished;
        bool paused;
    };

    BasicDownloader(QQuickItem* _progressBar);
    virtual ~BasicDownloader();

    /**
     * Show dialog asking user to confirm download, optionally allow target path to be changed.
     * @param dItem pointer to WebEngineDownloadItem QML object
     * @return return true if download was accepted and should proceed, false otherwise
     */
    bool downloadRequestedDialog(QObject* dItem);

public slots:
    void downloadRequested(QQuickWebEngineDownloadItem* download);
    void downloadFinished(QQuickWebEngineDownloadItem* download);
    void updateProgress();
    void updateTotalSize();

private:
    QQuickItem* progressBar;
    std::map<unsigned int, DownloadMetadata> itemsMetadata;

    double getProgress();
};

#endif /* BASICDOWNLOADER_H_ */
