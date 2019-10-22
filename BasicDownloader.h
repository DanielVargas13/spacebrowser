#ifndef BASICDOWNLOADER_H_
#define BASICDOWNLOADER_H_

#include <QObject>
#include <QQuickItem>

#include <map>
#include <memory>

class QQuickView;
class QQuickWebEngineDownloadItem;
class QQuickWebEngineProfile;

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
        QObject* dItem = nullptr;
    };

    BasicDownloader(std::shared_ptr<QQuickView> view);
    virtual ~BasicDownloader();

    /**
     * Show dialog asking user to confirm download, optionally allow target path to be changed.
     * @param dItem pointer to WebEngineDownloadItem QML object
     * @return return true if download was accepted and should proceed, false otherwise
     */
    bool downloadRequestedDialog(QObject* dItem);
    bool hasHistory() const;

    void setupProfile(std::shared_ptr<QQuickWebEngineProfile> profile);

signals:
    void progressUpdated(QVariant progress);
    void progressUpdated(QVariant id, QVariant received, QVariant total);
    void historyChanged(bool hasHistory);
    void newHistoryEntry(QVariant entry);
    void downloadFinished(QVariant id);
    void downloadPaused(QVariant id);
    void downloadResumed(QVariant id);
    void downloadCanceled(QVariant id);

public slots:
    void downloadRequested(QQuickWebEngineDownloadItem* download);
    void downloadFinished(QQuickWebEngineDownloadItem* download);
    void openUrl(QString url);
    void updateProgress();
    void updateTotalSize();
    void pause(int id);
    void resume(int id);
    void cancel(int id);

private:
    std::map<unsigned int, DownloadMetadata> itemsMetadata;

    double getProgress();
};

#endif /* BASICDOWNLOADER_H_ */
