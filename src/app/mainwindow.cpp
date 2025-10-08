#include "mainwindow.h"
#include "../aobuilder/constants.h"
#include "categorywidget.h"
#include "controller.h"
#include "mainwindowsettings.h"
#include "pushbutton.h"
#include "ui_infodialog.h"
#include "ui_mainwindow.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDialog>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QShortcut>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

#include <QActionGroup>
#include <QFile>
#include <QMenu>
#include <QResource>
#include <QSettings>
#include <QStyleFactory>
#include <QStyleHints>

#include <map>
#include <memory>
#include <vector>

namespace ab
{
class MainWindowPrivate
{
public:
    std::unique_ptr<Ui::MainWindow> ui = nullptr;
    std::unique_ptr<MainWindowSettings> settings = nullptr;
    model::Model *model = nullptr;
    Controller *controller = nullptr;
    CategoryWidget *defaultCategory = nullptr;
    QShortcut *quitShortcut = nullptr;
    bool useBranding = true;
    QString brandingStyle{qApp->style()->name()};
    QString systemStyle{qApp->style()->name()};
    bool isSystemStyleChange{true};
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate())
{
    d->ui = std::make_unique<Ui::MainWindow>();
    d->ui->setupUi(this);
    d->quitShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(close()));

    d->settings = std::make_unique<MainWindowSettings>(this, d->ui.get());
    d->settings->restoreSettings();
    d->isSystemStyleChange = true;

    auto categoryLayout = std::make_unique<QVBoxLayout>();
    categoryLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    d->ui->scrollArea->widget()->setLayout(categoryLayout.release());

    {
        QMenu *viewMenu = new QMenu(this);
        viewMenu->addSection(tr("style"));

        viewMenu->addAction(d->ui->actionAlteratorStyle);
        viewMenu->addAction(d->ui->actionSystemStyle);

        auto group = new QActionGroup(viewMenu);
        group->setExclusive(true);
        group->addAction(d->ui->actionAlteratorStyle);
        group->addAction(d->ui->actionSystemStyle);

        connect(group, &QActionGroup::triggered, this, [this] {
            setBrandingEnabled(d->ui->actionAlteratorStyle->isChecked());
        });

        d->ui->viewButton->setMenu(viewMenu);
        viewMenu->setFont(d->ui->viewButton->font());
    }

    d->ui->actionSwitchLegacy->setVisible(!QStandardPaths::findExecutable(Controller::legacy_app).isEmpty());

    auto infoWindow = new QDialog(this);
    Ui::InfoDialog dialogUi;
    dialogUi.setupUi(infoWindow);

    connect(d->ui->actionInfo, &QAction::triggered, infoWindow, &QDialog::show);

    setWindowTitle(tr("Alterator Explorer"));
    setWindowIcon(QIcon("alterator"));

    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::onColorSchemeChanged);
    connect(this, &MainWindow::styleChanged, this, &MainWindow::preserveStyle);
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    d->settings->saveSettings();
    QMainWindow::closeEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange)
        for (auto *w : findChildren<CategoryWidget *>())
            qApp->postEvent(w, event->clone());

    return QMainWindow::event(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange)
        emit styleChanged();

    return QMainWindow::changeEvent(event);
}

void MainWindow::preserveStyle()
{
    // do not rewrite stored name if style changed manually
    if (!d->isSystemStyleChange)
    {
        d->isSystemStyleChange = true;
        return;
    }

    // QTBUG-116337: temporarily remove stylesheet to get correct style name
    d->isSystemStyleChange = false;
    qApp->setStyleSheet("");

    // remember system style
    d->systemStyle = qApp->style()->name();

    setBrandingEnabled(d->useBranding);
}

void MainWindow::setController(Controller *newContoller)
{
    d->controller = newContoller;
    connect(d->ui->actionSwitchLegacy, &QAction::triggered, d->controller, &Controller::switchBack);
    connect(d->ui->actionRefresh, &QAction::triggered, this, [this] {
        d->ui->centralwidget->setEnabled(false);
        setCursor(Qt::WaitCursor);

        clearUi();
        d->controller->buildModel();

        d->ui->centralwidget->setEnabled(true);
        unsetCursor();
    });
}

void MainWindow::setModel(model::Model *newModel)
{
    d->model = newModel;
    d->defaultCategory = nullptr;

    QLayout *categoryLayout = d->ui->scrollArea->widget()->layout();

    for (auto category : d->model->getCategories())
    {
        categoryLayout->addWidget(new CategoryWidget{this, d->model, category});
    }
}

void MainWindow::clearUi()
{
    QLayout *categoryLayout = d->ui->scrollArea->widget()->layout();
    while (categoryLayout->itemAt(0) != nullptr)
    {
        QWidget *categoryWidget = categoryLayout->itemAt(0)->widget();
        categoryLayout->removeWidget(categoryWidget);
        delete categoryWidget;
    }
}

void MainWindow::onModuleClicked(PushButton *button)
{
    d->controller->moduleClicked(button->getObject());
}

void MainWindow::showInfo() {}

/*
 *  original loadBranding() from alterator-browser:
 *  https://git.altlinux.org/gears/a/alterator-browser-qt6.git
 */
void MainWindow::loadBranding()
{
    QResource::unregisterResource(QStringLiteral("/etc/alterator/design-browser-qt"));
    QResource::registerResource(QStringLiteral("/etc/alterator/design-browser-qt"));

    QFont font_def(QGuiApplication::font());

    if (QFile::exists(QStringLiteral(":/design/design.ini")))
    {
        QSettings settings(QStringLiteral(":/design/design.ini"), QSettings::IniFormat, qApp);
        settings.setFallbacksEnabled(false);
        QStringList keys(settings.allKeys());
        QString key;

        // set Qt style
        key = QStringLiteral("style");
        if (!keys.contains(key))
        {
            key = QStringLiteral("Style");
        }

        d->brandingStyle = keys.contains(key) ? settings.value(key).toString() : "fusion";

        if (QStyleFactory::keys().contains(d->brandingStyle, Qt::CaseInsensitive))
        {
            d->isSystemStyleChange = false;
            qApp->setStyle(d->brandingStyle);
        }

        // set icons theme
        key = QStringLiteral("icons");
        if (!keys.contains(key))
        {
            key = QStringLiteral("Icons");
        }
        if (keys.contains(key))
        {
            QString iconsTheme = settings.value(key).toString();
            if (!iconsTheme.isEmpty())
                QIcon::setThemeName(iconsTheme);
        }
        else
        {
            auto paths = QIcon::themeSearchPaths();
            paths.prepend(QStringLiteral("/usr/share/alterator/design/images/"));
            QIcon::setThemeSearchPaths(paths);
            QIcon::setFallbackThemeName("");
        }

        key = QStringLiteral("icons_fallback");
        if (!keys.contains(key))
        {
            key = QStringLiteral("Icons_fallback");
        }
        if (!keys.contains(key))
        {
            key = QStringLiteral("Icons_Fallback");
        }
        if (keys.contains(key))
        {
            QString fallIconsTheme = settings.value(key).toString();
            if (!fallIconsTheme.isEmpty())
                QIcon::setFallbackThemeName(fallIconsTheme);
        }
        else
            QIcon::setFallbackThemeName("");

        // set palette
        QPalette pal(Qt::black);
        int groupCount = 0;
        QHash<QString, QPalette::ColorGroup> in_opts({{QStringLiteral("Palette/active"), QPalette::Active},
                                                      {QStringLiteral("Palette/inactive"), QPalette::Inactive},
                                                      {QStringLiteral("Palette/disabled"), QPalette::Disabled}});
        QHashIterator<QString, QPalette::ColorGroup> i_opt(in_opts);
        while (i_opt.hasNext())
        {
            i_opt.next();
            QString parm = i_opt.key();
            if (!keys.contains(parm))
            {
                parm = parm.toLower();
            }
            QStringList colors = settings.value(parm).toStringList();
            while (colors.count() < QPalette::NColorRoles)
            {
                // add one fake color for probably Qt5 pallette to fit Qt6 palette
                colors << QStringLiteral("#ff0000");
            }
            if (colors.count() >= QPalette::NColorRoles)
            {
                ++groupCount;
                for (int i = 0; i < qMin(colors.count(), static_cast<int>(QPalette::NColorRoles)); i++)
                    pal.setColor(i_opt.value(), static_cast<QPalette::ColorRole>(i), QColor(colors[i]));
            }
        }

        if (groupCount == QPalette::NColorGroups)
            qApp->setPalette(pal);

        // get font
        key = QStringLiteral("font");
        if (!keys.contains(key))
        {
            key = QStringLiteral("Font");
        }
        if (keys.contains(key))
        {
            QString str = settings.value(key).toString();
            if (!str.isEmpty())
            {
                font_def.fromString(str);
            }
        }
    }
    QGuiApplication::setFont(font_def);

    // set style
    QString new_style;
    QFile file(QStringLiteral(":/design/design.qss"));
    if (file.exists())
    {
        if (file.open(QFile::ReadOnly))
        {
            QString styleContent = QLatin1String(file.readAll());
            if (styleContent.size() < 10)
            {
                qDebug("Too small file: \"%s\"", qPrintable(file.fileName()));
                return;
            }
            new_style = styleContent;
        }
        else
        {
            qDebug("Unable to read file: \"%s\"", qPrintable(file.fileName()));
            return;
        }
    }
    new_style.replace(".ACenterFace", "#MainWindow");
    new_style.replace("#centerface", "#MainWindow");
    new_style.replace("#module_section", ".CategoryWidget");

    d->isSystemStyleChange = false;
    qApp->setStyleSheet(new_style);
}

void MainWindow::onColorSchemeChanged()
{
    if (d->useBranding)
        loadBranding();
}

bool MainWindow::isBrandingEnabled()
{
    return d->useBranding;
}

void MainWindow::setBrandingEnabled(bool enable)
{
    d->useBranding = enable;
    d->settings->saveSettings();

    (d->useBranding ? d->ui->actionAlteratorStyle : d->ui->actionSystemStyle)->setChecked(true);

    if (!d->useBranding)
    {
        d->isSystemStyleChange = false;
        qApp->setStyleSheet("");
        d->isSystemStyleChange = false;
        qApp->setStyle(d->systemStyle);
        qApp->setPalette(qApp->style()->standardPalette());
        QIcon::setThemeName("");
        QIcon::setFallbackThemeName("");
    }

    onColorSchemeChanged();

    d->ui->refreshButton->setIcon(QIcon::fromTheme("view-refresh", QIcon::fromTheme("view-refresh-symbolic")));
    d->ui->legacyButton->setIcon(QIcon::fromTheme("view-refresh", QIcon::fromTheme("view-refresh-symbolic")));
    d->ui->infoButton->setIcon(QIcon::fromTheme("help-about", QIcon::fromTheme("help-about-symbolic")));
}

} // namespace ab
