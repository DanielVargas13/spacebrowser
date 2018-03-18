#include <BasicDownloader.h>

#include <QDesktopServices>
#include <QFileDialog>
#include <QJSEngine>
#include <QJSValue>
#include <QMetaObject>

#include <iostream>

BasicDownloader::BasicDownloader()
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
        unsigned int id = itemsMetadata.size();
        std::cout << "ID: " << id << std::endl;
        std::cout << "DID: " << d->property("id").toInt() << std::endl;
        DownloadMetadata dm;
        dm.received = d->property("receivedBytes").toLongLong();
        dm.total = d->property("totalBytes").toLongLong();
        dm.path = d->property("path").toString();
        dm.finished = d->property("isFinished").toBool();
        dm.paused = d->property("isPaused").toBool();
        itemsMetadata[id] = dm; // FIXME: can't load from db, id's will be reset on application restart

        /// Set-up signal handlers, emit historyUpdated signal and accept download
        ///
        connect(d, SIGNAL(receivedBytesChanged()), SLOT(updateProgress()));
        connect(d, SIGNAL(totalBytesChanged()), SLOT(updateTotalSize()));

        if (itemsMetadata.size() == 1)
            emit historyChanged(true);

        QJSEngine engine;
        QJSValue val = engine.newObject();
        val.setProperty("url", "Source url here: Seems Qt doesn't provide that (yet)");
        val.setProperty("path", dm.path);
        val.setProperty("received", (double)dm.received);
        val.setProperty("total", (double)dm.total);
        val.setProperty("myId", id);

        emit newHistoryEntry(val.toVariant());

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
    int id = source->property("id").toInt();
    DownloadMetadata& md = itemsMetadata.at(id-1);
    md.received = source->property("receivedBytes").toLongLong();

    emit progressUpdated(getProgress());
    emit progressUpdated(id, source->property("receivedBytes").toReal(),
            source->property("totalBytes").toReal());
}

void BasicDownloader::updateTotalSize()
{
    QObject* source = sender();

    /// Update total bytes and recalculate progress
    ///
    int id = source->property("id").toInt(); // FIXME: map this to id
    DownloadMetadata& md = itemsMetadata.at(id-1);
    md.total = source->property("totalBytes").toLongLong();

    emit progressUpdated(getProgress());
    emit progressUpdated(id, source->property("receivedBytes").toReal(),
            source->property("totalBytes").toReal());
}

double BasicDownloader::getProgress()
{
    double received = 0;
    double total = 0;

    /// For each running download count received and total bytes
    ///
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

bool BasicDownloader::hasHistory() const
{
    return itemsMetadata.size() != 0;
}

void BasicDownloader::openUrl(QString url)
{
    QDesktopServices::openUrl(url);
}
