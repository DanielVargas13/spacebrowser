#include <PrintHandler.h>

#include <QFileDialog>

PrintHandler::PrintHandler()
{

}

PrintHandler::~PrintHandler()
{

}

void PrintHandler::printRequested(QVariant webView)
{
    QString filePath;
    if (!printingRequestedDialog(filePath))
        return;

    bool s = QMetaObject::invokeMethod(qvariant_cast<QObject *>(webView),
            "printToPdf", Q_ARG(QString, filePath));
}

void PrintHandler::pdfPrintingFinished(QString filePath, bool success)
{
}

bool PrintHandler::printingRequestedDialog(QString& filePath)
{
    /// Set-up and show QFileDialog
    ///
    QFileDialog qfd;
    qfd.setFileMode(QFileDialog::AnyFile);
    qfd.setAcceptMode(QFileDialog::AcceptSave);

    qfd.selectFile("page.pdf");

    bool accepted = qfd.exec() == QDialog::Accepted;

    filePath = qfd.selectedFiles().first();

    return accepted;
}
