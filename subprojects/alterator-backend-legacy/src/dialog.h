#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <memory>

namespace alt
{
namespace Ui
{
class Dialog;
} // namespace Ui

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Dialog(const Dialog &) = delete;
    Dialog(Dialog &&) = delete;
    Dialog &operator=(const Dialog &) = delete;
    Dialog &operator=(Dialog &&) = delete;

private:
    std::unique_ptr<Ui::Dialog> m_ui;
};
} // namespace alt
#endif // DIALOG_H
