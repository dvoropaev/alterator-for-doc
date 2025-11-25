#ifndef MAINSTATUSBAR_H
#define MAINSTATUSBAR_H

#include "entity/edition.h"

#include "statusbar.h"
#include <QFrame>
#include <QLabel>

namespace alt
{
class MainStatusBar : public StatusBar
{
    Q_OBJECT
public:
    explicit MainStatusBar(QWidget *parent = nullptr);
    ~MainStatusBar() override;

public:
    MainStatusBar(const MainStatusBar &) = delete;
    MainStatusBar(MainStatusBar &&) = delete;
    MainStatusBar &operator=(const MainStatusBar &) = delete;
    MainStatusBar &operator=(MainStatusBar &&) = delete;

public:
    void showEdition(const QString &editionName);

public slots:
    void onStarted();
    void onDone(Edition *current_edition, int numberOfComponentsBuilt, int editionComponentsBuilt);
    void changeEvent(QEvent *event) override;

private:
    std::unique_ptr<QLabel> editionLabel;
    std::unique_ptr<QLabel> editionTagLabel;
    std::unique_ptr<QFrame> line;
    int m_componentsCount = 0;
    int m_editionComponentsCount = 0;
    Edition *m_currentEdition = nullptr;
};

} // namespace alt

#endif // MAINSTATUSBAR_H
