#include <BasicDownloader.h>

#include <iostream>

BasicDownloader::BasicDownloader()
{

}

BasicDownloader::~BasicDownloader()
{

}

void BasicDownloader::downloadRequested(QQuickWebEngineDownloadItem* download)
{
    std::cout << "downloadRequested\n";
}

void BasicDownloader::downloadFinished(QQuickWebEngineDownloadItem* download)
{
}
