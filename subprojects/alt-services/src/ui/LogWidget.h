#pragma once

#include <QWidget>

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
    void endEntry( bool );
    void message( const QString& );
    void error( const QString& );
    void clear();
    void setExportFileName(const QString& name);

private:
    class Private;
    Private* d{};
};
