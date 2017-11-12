#ifndef BASICDOWNLOADER_H_
#define BASICDOWNLOADER_H_

#include <QQuickWebEngineProfile>

class BasicDownloader
{
public:
    BasicDownloader();
    virtual ~BasicDownloader();

    void downloadRequested(QQuickWebEngineDownloadItem* download);
    void downloadFinished(QQuickWebEngineDownloadItem* download);
};

#endif /* BASICDOWNLOADER_H_ */
