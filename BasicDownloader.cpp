#include <BasicDownloader.h>

#include <QFileDialog>
#include <QMetaObject>
//#include <QtWebEngine/5.10.0/QtWebEngine/private/qquickwebenginedownloaditem_p.h>
#include <iostream>

BasicDownloader::BasicDownloader(QQuickItem* _progressBar) : progressBar(_progressBar)
{

}

BasicDownloader::~BasicDownloader()
{

}

void BasicDownloader::downloadRequested(QQuickWebEngineDownloadItem* download)
{
    QObject* d = reinterpret_cast<QObject*>(download);
    bool accepted = downloadRequestedDialog(d);

    if (accepted)
    {
        /// Save meta information about download item
        ///
        unsigned int id = d->property("id").toUInt();
        DownloadMetadata dm;
        dm.received = d->property("receivedBytes").toLongLong();
        dm.total = d->property("totalBytes").toLongLong();
        dm.path = d->property("path").toString();
        dm.finished = d->property("isFinished").toBool();
        dm.paused = d->property("isPaused").toBool();
        itemsMetadata[id] = dm;

        /// Set-up signal handlers and accept download
        ///
        connect(d, SIGNAL(receivedBytesChanged()), SLOT(updateProgress()));
        connect(d, SIGNAL(totalBytesChanged()), SLOT(updateTotalSize()));

        QMetaObject::invokeMethod(d, "accept", Qt::DirectConnection);
    }
    else
        QMetaObject::invokeMethod(d, "cancel", Qt::DirectConnection);
}

void BasicDownloader::downloadFinished(QQuickWebEngineDownloadItem* download)
{
    QObject* d = reinterpret_cast<QObject*>(download);

    /// If download was started, mark as finished
    ///
    if (itemsMetadata.count(d->property("id").toInt()) != 1)
        return;

    itemsMetadata[d->property("id").toInt()].finished = true;
}

bool BasicDownloader::downloadRequestedDialog(QObject* dItem)
{
    /// Set-up and show QFileDialog
    ///
    QFileDialog qfd;
    qfd.setFileMode(QFileDialog::AnyFile);
    qfd.setAcceptMode(QFileDialog::AcceptSave);

    qfd.selectFile(dItem->property("path").toString());

    bool accepted = qfd.exec() == QDialog::Accepted;

    /// Set target path and filename for the accepted download
    ///
    if (accepted)
        dItem->setProperty("path", qfd.selectedFiles().first());

    return accepted;
}

void BasicDownloader::updateProgress()
{
    QObject* source = sender();

    /// Update received bytes and recalculate progress
    ///
    DownloadMetadata& md = itemsMetadata[source->property("id").toInt()];
    md.received = source->property("receivedBytes").toLongLong();

    progressBar->setProperty("progress", getProgress());
}

void BasicDownloader::updateTotalSize()
{
    QObject* source = sender();

    /// Update total bytes and recalculate progress
    ///
    DownloadMetadata& md = itemsMetadata[source->property("id").toInt()];
    md.total = source->property("totalBytes").toLongLong();

    progressBar->setProperty("progress", getProgress());
}

double BasicDownloader::getProgress()
{
    double received = 0;
    double total = 0;
    std::for_each(itemsMetadata.begin(), itemsMetadata.end(),
            [&received, &total](std::pair<const unsigned int, BasicDownloader::DownloadMetadata>& d)
    {
        if (d.second.finished || d.second.paused)
            return;

        received += d.second.received;
        total += d.second.total;
    });

    return (received / total);
}
