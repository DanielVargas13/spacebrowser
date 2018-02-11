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
    Q_PROPERTY(bool history READ hasHistory NOTIFY historyChanged STORED false);

    struct DownloadMetadata
    {
        long long int received;
        long long int total;
        QString path;
        bool finished;
        bool paused;
    };

    BasicDownloader();
    virtual ~BasicDownloader();

    /**
     * Show dialog asking user to confirm download, optionally allow target path to be changed.
     * @param dItem pointer to WebEngineDownloadItem QML object
     * @return return true if download was accepted and should proceed, false otherwise
     */
    bool downloadRequestedDialog(QObject* dItem);

    bool hasHistory() const;

signals:
    void progressUpdated(QVariant progress);
    void historyChanged(bool hasHistory);

public slots:
    void downloadRequested(QQuickWebEngineDownloadItem* download);
    void downloadFinished(QQuickWebEngineDownloadItem* download);
    void updateProgress();
    void updateTotalSize();

private:
    std::map<unsigned int, DownloadMetadata> itemsMetadata;

    double getProgress();
};

#endif /* BASICDOWNLOADER_H_ */
