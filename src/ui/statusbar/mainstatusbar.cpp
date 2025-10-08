#include "mainstatusbar.h"
#include "application.h"
#include "model/objects/edition.h"

#include "ui_statusbar.h"

#include <QLabel>
#include <QProgressBar>

namespace alt
{
MainStatusBar::MainStatusBar(QWidget *parent)
    : StatusBar(parent)
    , editionLabel(std::make_unique<QLabel>())
    , editionTagLabel(std::make_unique<QLabel>())
    , line(std::make_unique<QFrame>())
{
    line->setFrameShape(QFrame::Shape::VLine);
    ui->horizontalLayout->insertWidget(0, line.get());
    ui->horizontalLayout->insertWidget(0, editionLabel.get());
    ui->horizontalLayout->insertWidget(0, editionTagLabel.get());
}

MainStatusBar::~MainStatusBar() = default;

void MainStatusBar::onStarted()
{
    StatusBar::onStarted(tr("Updating components from system state..."));
}

void MainStatusBar::onDone(Edition *current_edition, int numberOfComponentsBuilt, int editionComponentsBuilt)
{
    m_currentEdition = current_edition;
    m_componentsCount = numberOfComponentsBuilt;
    m_editionComponentsCount = editionComponentsBuilt;

    if (current_edition != nullptr)
    {
        StatusBar::onDone(tr("Components found: ") + QString::number(numberOfComponentsBuilt) + tr(", from edition: ")
                          + QString::number(m_editionComponentsCount));
        current_edition->displayName
            = current_edition->displayNameLocaleStorage[Application::getLocale().name().split('_').first()];
        showEdition(current_edition->displayName);
    }
    else
    {
        StatusBar::onDone(tr("Components found: ") + QString::number(numberOfComponentsBuilt));
    }
}

void MainStatusBar::showEdition(const QString &editionName)
{
    editionTagLabel->setVisible(true);
    editionTagLabel->setText(tr("Edition:"));
    editionLabel->setText(editionName);
    editionLabel->setVisible(true);
    line->setVisible(true);
}

void MainStatusBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        // TODO(chernigin): I don't like this method of translating labels. But don't know yet how to fix it.
        if (ui->progressBar->isVisible())
        {
            onStarted();
        }
        else if (m_componentsCount > 0)
        {
            if (m_editionComponentsCount > 0)
            {
                StatusBar::onDone(tr("Components found: ") + QString::number(m_componentsCount) + tr(", from edition: ")
                                  + QString::number(m_editionComponentsCount));
            }
            else
            {
                StatusBar::onDone(tr("Components found: ") + QString::number(m_componentsCount));
            }
        }

        if (m_currentEdition != nullptr)
        {
            m_currentEdition->displayName
                = m_currentEdition->displayNameLocaleStorage[Application::getLocale().name().split('_').first()];
            showEdition(m_currentEdition->displayName);
        }
    }

    StatusBar::changeEvent(event);
}

} // namespace alt
