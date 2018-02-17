#ifndef PRINTHANDLER_H_
#define PRINTHANDLER_H_

#include <QObject>
#include <QString>
#include <QVariant>

class PrintHandler : public QObject
{
    Q_OBJECT
public:
    PrintHandler();
    virtual ~PrintHandler();

public slots:
    void printRequested(QVariant webView);
    void pdfPrintingFinished(QString filePath, bool success);

private:
    bool printingRequestedDialog(QString& filePath);
};

#endif /* PRINTHANDLER_H_ */
