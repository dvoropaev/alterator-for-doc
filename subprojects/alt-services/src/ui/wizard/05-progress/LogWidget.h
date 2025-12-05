#pragma once

#include <QWidget>
#include "controller/Controller.h"

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = nullptr);
    ~LogWidget();

    QAction* searchAction() const;
    QAction* exportAction() const;

public slots:
    void beginEntry( const QString& );
    void endEntry( Controller::Result );
    void message( const QString& );
    void error( const QString& );
    void clear();
    void setExportFileName(const QString& name);

private:
    class Private;
    Private* d{};
};
