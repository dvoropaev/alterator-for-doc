#ifndef AB_PUSH_BUTTON_H
#define AB_PUSH_BUTTON_H

#include "../aobuilder/objects/object.h"

#include <QToolButton>
#include <QWidget>

namespace ab
{
class MainWindow;

class PushButton : public QToolButton
{
    Q_OBJECT

public:
    explicit PushButton(MainWindow *w, QWidget *parent = nullptr);
    ~PushButton() override = default;

    void setObject(ao_builder::Object *obj);
    ao_builder::Object *getObject();

    PushButton(const PushButton &) = delete;
    PushButton(PushButton &&) = delete;
    PushButton &operator=(const PushButton &) = delete;
    PushButton &operator=(PushButton &&) = delete;

    bool event(QEvent* event) override;

private:
    ao_builder::Object *object = nullptr;
    MainWindow *window = nullptr;

    void onClicked(bool b);
    void setColor();

signals:
    void moduleClicked(PushButton *button);
};
} // namespace ab

#endif // AB_PUSH_BUTTON_H
