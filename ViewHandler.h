#ifndef VIEWHANDLER_H_
#define VIEWHANDLER_H_

#include <QObject>
#include <QQuickItem>

class ViewHandler : public QObject
{
    Q_OBJECT

public:
    ViewHandler(QQuickItem* _webViewContainer);
    virtual ~ViewHandler();

public slots:
    void viewSelected(int viewId);
    int createNewView();
    QVariant getView(int viewId);

private:
    QQuickItem* webViewContainer;
    int viewId = 0;

    std::map<int, QVariant> views;
};

#endif /* VIEWHANDLER_H_ */
