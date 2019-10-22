#include <BasicDownloader.h>

#include <QDesktopServices>
#include <QFileDialog>
#include <QJSEngine>
#include <QJSValue>
#include <QMetaObject>
#include <QQuickView>
#include <QQuickWebEngineProfile>

#include <iostream>

BasicDownloader::BasicDownloader(std::shared_ptr<QQuickView> view)
{
    /// Connect this downloader to visual components
    /// FIXME: this will conflict in case downloads will run on several profiles at the same time
    ///
    QQuickItem* downloaderProgressBar = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadProgressBar"));
    if (!downloaderProgressBar)
        throw std::runtime_error("No downloaderProgressBar object found");

    QObject::connect(this, SIGNAL(progressUpdated(QVariant)),
            downloaderProgressBar, SLOT(updateProgress(QVariant)));
    QQuickItem* downloadHistoryButton = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryButton"));
    if (!downloadHistoryButton)
        throw std::runtime_error("No downloadHistoryButton object found");

    QObject::connect(this, &BasicDownloader::historyChanged,
            downloadHistoryButton, &QQuickItem::setVisible);
    QQuickItem* downloadHistoryView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryView"));
    if (!downloadHistoryView)
        throw std::runtime_error("No downloadHistoryView object found");

    QObject::connect(this, SIGNAL(downloadFinished(QVariant)),
            downloadHistoryView, SLOT(downloadFinished(QVariant)));
    QObject::connect(this, SIGNAL(newHistoryEntry(QVariant)),
            downloadHistoryView, SLOT(addEntry(QVariant)));
    QObject::connect(this, SIGNAL(progressUpdated(QVariant, QVariant, QVariant)),
            downloadHistoryView,
            SLOT(updateProgress(QVariant, QVariant, QVariant)));
    QObject::connect(this, SIGNAL(downloadPaused(QVariant)), downloadHistoryView,
            SLOT(downloadPaused(QVariant)));
    QObject::connect(this, SIGNAL(downloadResumed(QVariant)),
            downloadHistoryView, SLOT(downloadResumed(QVariant)));
    QObject::connect(this, SIGNAL(downloadCanceled(QVariant)),
            downloadHistoryView, SLOT(downloadCanceled(QVariant)));
    QObject::connect(downloadHistoryView, SIGNAL(openUrl(QString)), this,
            SLOT(openUrl(QString)));
    QObject::connect(downloadHistoryView, SIGNAL(pause(int)), this,
            SLOT(pause(int)));
    QObject::connect(downloadHistoryView, SIGNAL(resume(int)), this,
            SLOT(resume(int)));
    QObject::connect(downloadHistoryView, SIGNAL(cancel(int)), this,
            SLOT(cancel(int)));
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
        int id = d->property("id").toInt();
        std::cout << "DID: " << d->property("id").toInt() << std::endl;
        DownloadMetadata dm;
        dm.received = d->property("receivedBytes").toLongLong();
        dm.total = d->property("totalBytes").toLongLong();
        dm.path = d->property("path").toString();
        dm.finished = d->property("isFinished").toBool();
        dm.paused = d->property("isPaused").toBool();
        dm.dItem = d;
        std::cout << "Inserting ditem at: " << id << std::endl;
        itemsMetadata[id] = dm;
        // FIXME: can't load from db, id's will be reset on application restart

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
        val.setProperty("paused", false);
        val.setProperty("canceled", false);
        val.setProperty("finished", false);

        emit newHistoryEntry(val.toVariant());

        QMetaObject::invokeMethod(d, "accept", Qt::DirectConnection);
    }
    else
        QMetaObject::invokeMethod(d, "cancel", Qt::DirectConnection);
}

void BasicDownloader::downloadFinished(QQuickWebEngineDownloadItem* download)
{
    QObject* d = reinterpret_cast<QObject*>(download);

    /// If download was started, mark as finished and emit signal to update model
    ///
    int id = d->property("id").toInt();

    if (itemsMetadata.count(id))
    {
        DownloadMetadata& md = itemsMetadata.at(id);
        md.finished = true;

        emit downloadFinished(id);
    }
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
    std::cout << "Trying to update progress at: " << id << std::endl;
    DownloadMetadata& md = itemsMetadata.at(id);
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
    DownloadMetadata& md = itemsMetadata.at(id);
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(url));
}

void BasicDownloader::pause(int id)
{
    DownloadMetadata& md = itemsMetadata.at(id);
    if (md.finished)
        return;

    QMetaObject::invokeMethod(md.dItem, "pause", Qt::DirectConnection);

    emit downloadPaused(id);
}

void BasicDownloader::resume(int id)
{
    DownloadMetadata& md = itemsMetadata.at(id);
    if (md.finished)
        return;

    QMetaObject::invokeMethod(md.dItem, "resume", Qt::DirectConnection);

    emit downloadResumed(id);
}

void BasicDownloader::cancel(int id)
{
    DownloadMetadata& md = itemsMetadata.at(id);
    if (md.finished)
        return;

    QMetaObject::invokeMethod(md.dItem, "cancel", Qt::DirectConnection);

    emit downloadCanceled(id);
}

void BasicDownloader::setupProfile(std::shared_ptr<QQuickWebEngineProfile> profile)
{
    QObject::connect(profile.get(),
                     SIGNAL(downloadRequested(QQuickWebEngineDownloadItem*)), this,
                     SLOT(downloadRequested(QQuickWebEngineDownloadItem*)));
    QObject::connect(profile.get(),
                     SIGNAL(downloadFinished(QQuickWebEngineDownloadItem*)), this,
                     SLOT(downloadFinished(QQuickWebEngineDownloadItem*)));
}
