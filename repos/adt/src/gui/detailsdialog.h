#ifndef DETAILS_DIALOG_H
#define DETAILS_DIALOG_H

#include <QDialog>

#include <model/adttest.h>

namespace Ui
{
class DetailsDialog;
}

class DetailsDialog : public QDialog
{
    Q_OBJECT
public:
    DetailsDialog(QWidget *parent = nullptr);
    ~DetailsDialog();

    void setDetailsText(const QString &text);
    void clearDetailsText();

    void setTestId(const QString &toolId, const QString &testId);
    QString getToolId() const;
    QString getTestId() const;

    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_closePushButton_clicked();

private:
    Ui::DetailsDialog *ui{};
    QString m_toolId{};
    QString m_testId{};

private:
    DetailsDialog(const DetailsDialog &)            = delete;
    DetailsDialog(DetailsDialog &&)                 = delete;
    DetailsDialog &operator=(const DetailsDialog &) = delete;
    DetailsDialog &operator=(DetailsDialog &&)      = delete;
};

#endif // DETAILS_DIALOG_H
