#include "tooltabviewdelegate.h"
#include "vars/adtvarsinterface.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>

#include <QVBoxLayout>

ToolTabViewDelegate::ToolTabViewDelegate(ADTTool *tool, int vPadding, QObject *parent)
    : m_tool(tool)
{}

void ToolTabViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        painter->save();

        auto model   = index.model();
        auto varName = model->data(model->index(index.row(), 5), Qt::DisplayRole).toString();

        ADTVarInterface *var = m_tool->getVar(varName);
        if (!var)
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QString boldText   = var->getDisplayName();
        QString italicText = var->getComment();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        opt.text.clear();

        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

        QRect textRect = opt.rect - QMargins{3,3,3,3};

        QFont boldFont = opt.font;
        boldFont.setBold(true);

        QFont italicFont = opt.font;
        italicFont.setItalic(true);
        //italicFont.setPointSize(opt.font.pointSize() - 2); // Уменьшаем размер шрифта на 2 пункта

        QFontMetrics boldMetrics(boldFont);
        QFontMetrics italicMetrics(italicFont);

        QString elidedBoldText   = boldMetrics.elidedText(boldText, Qt::ElideRight, textRect.width());
        QString elidedItalicText = italicMetrics.elidedText(italicText, Qt::ElideRight, textRect.width());

        painter->setFont(boldFont);

        painter->drawText(textRect.adjusted(0, 0, 0, -textRect.height() / 2),
                          Qt::AlignLeft | Qt::AlignTop,
                          elidedBoldText);

        painter->setFont(italicFont);

        painter->drawText(textRect.adjusted(0, textRect.height() / 2, 0, 0),
                          Qt::AlignLeft | Qt::AlignBottom,
                          elidedItalicText);

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ToolTabViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        auto model   = index.model();
        auto varName = model->data(model->index(index.row(), 5), Qt::DisplayRole).toString();

        ADTVarInterface *var = m_tool->getVar(varName);

        QString boldText   = var->getDisplayName();
        QString italicText = var->getComment();

        QFont boldFont = option.font;
        boldFont.setBold(true);

        QFont italicFont = option.font;
        italicFont.setItalic(true);
        //italicFont.setPointSize(option.font.pointSize() - 2);

        QFontMetrics boldMetrics(boldFont);
        QFontMetrics italicMetrics(italicFont);

        int boldHeight   = boldMetrics.height();
        int italicHeight = italicMetrics.height();

        int totalHeight = boldHeight + italicHeight;

        int maxWidth = qMax(boldMetrics.horizontalAdvance(boldText), italicMetrics.horizontalAdvance(italicText));

        return QSize(maxWidth, totalHeight).grownBy(QMargins{3,3,3,3});
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *ToolTabViewDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        if (!index.isValid())
            return QStyledItemDelegate::createEditor(parent, option, index);

        auto model   = index.model();
        auto varName = model->data(model->index(index.row(), 5), Qt::DisplayRole).toString();

        ADTVarInterface *var = m_tool->getVar(varName);
        if (!var)
            return QStyledItemDelegate::createEditor(parent, option, index);

        auto type = var->getType();

        QList<QVariant> vals;
        var->getEnumValues(&vals);

        auto container = std::make_unique<QWidget>(parent);
        auto* layout = new QVBoxLayout{};
        layout->setContentsMargins(1,1,1,1);
        container->setLayout(layout);
        QWidget* editor = nullptr;

        if (index.column() == 2) //value
        {
            switch (type)
            {
                case ADTVarInterface::ADTVarType::INT:
                {
                    QSpinBox *spinBox = new QSpinBox(container.get());
                    spinBox->setRange(INT_MIN, INT_MAX);
                    connect(spinBox, &QSpinBox::textChanged, this, &ToolTabViewDelegate::commit);

                    editor = spinBox;
                    break;
                }
                case ADTVarInterface::ADTVarType::STRING:
                {
                    QLineEdit *lineEdit = new QLineEdit(container.get());
                    connect(lineEdit, &QLineEdit::textChanged, this, &ToolTabViewDelegate::commit);

                    editor = lineEdit;
                    break;
                }
                case ADTVarInterface::ADTVarType::ENUM_INT:
                {
                    QComboBox *comboBox = new QComboBox(container.get());

                    QStringList valsStrings;
                    for (auto c : vals)
                    {
                        QString cvs = c.toString();
                        if (!cvs.isEmpty())
                            valsStrings.append(cvs);
                    }

                    for (auto &valString : valsStrings)
                    {
                        comboBox->addItem(valString);
                    }
                    connect(comboBox, &QComboBox::currentIndexChanged, this, &ToolTabViewDelegate::commit);

                    editor = comboBox;
                    break;
                }
                case ADTVarInterface::ADTVarType::ENUM_STRING:
                {
                    QComboBox *comboBox = new QComboBox(container.get());

                    QStringList valsStrings;
                    for (auto c : vals)
                    {
                        QString cvs = c.toString();
                        if (!cvs.isEmpty())
                            valsStrings.append(cvs);
                    }

                    for (auto &valString : valsStrings)
                    {
                        comboBox->addItem(valString);
                    }
                    connect(comboBox, &QComboBox::currentIndexChanged, this, &ToolTabViewDelegate::commit);

                    editor = comboBox;
                    break;
                }
                default:
                    break;
            }

            if (editor)
            {
                container->layout()->addWidget(editor);
                return container.release();
            }

        }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ToolTabViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto model   = index.model();
    auto varName = model->data(model->index(index.row(), 5), Qt::DisplayRole).toString();

    ADTVarInterface *var = m_tool->getVar(varName);
    auto type            = var->getType();

    if (!var)
        return;

    if (index.column() == 2 && editor && editor->layout())
    { //value

        if (auto* item = editor->layout()->itemAt(0))
        {
            if ( auto* widget = editor->layout()->itemAt(0)->widget())
            {
                switch (type)
                {
                    case ADTVarInterface::ADTVarType::INT:
                    {
                        if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget))
                        {
                            int val;
                            if (var->get(&val))
                                spinBox->setValue(val);
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::STRING:
                    {
                        if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget))
                        {
                            QString val;
                            if (var->get(&val))
                                lineEdit->setText(val);
                            else
                                lineEdit->setText("");
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::ENUM_INT:
                    {
                        if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget) )
                        {
                            int val;
                            if (var->get(&val))
                            {
                                int intIndex = comboBox->findText(QString::number(val));
                                comboBox->setCurrentIndex(intIndex);
                            }
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::ENUM_STRING:
                    {
                        if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget))
                        {
                            QString val;
                            if (var->get(&val))
                            {
                                int intIndex = comboBox->findText((val));
                                comboBox->setCurrentIndex(intIndex);
                            }
                            else
                                comboBox->setCurrentIndex(0);
                        }
                        break;
                    }
                    default:
                    {
                        qWarning() << "undefined type";
                    }
                }
            }
        }
    }
}

void ToolTabViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto varName = model->data(model->index(index.row(), 5), Qt::DisplayRole).toString();

    ADTVarInterface *var = m_tool->getVar(varName);

    auto type = var->getType();

    if (index.column() == 2 && editor && editor->layout())
    { //value

        if (auto* item = editor->layout()->itemAt(0))
        {
            if ( auto* widget = editor->layout()->itemAt(0)->widget())
            {
                switch (type)
                {
                    case ADTVarInterface::ADTVarType::INT:
                    {
                        if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget))
                        {
                            var->set(spinBox->value());
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::STRING:
                    {
                        if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget))
                        {
                            var->set(lineEdit->text());
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::ENUM_INT:
                    {
                        if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget) )
                        {
                            bool success = false;
                            int value    = comboBox->currentText().toInt(&success, 10);
                            if (success)
                                var->set(value);
                        }
                        break;
                    }
                    case ADTVarInterface::ADTVarType::ENUM_STRING:
                    {
                        if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget))
                        {
                            var->set(comboBox->currentText());
                        }
                        break;
                    }
                    default:
                    {
                        qWarning() << "undefined type";
                    }
                }
            }
        }
    }
}

void ToolTabViewDelegate::updateEditorGeometry(QWidget *editor,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ToolTabViewDelegate::commit()
{
    if ( auto* widget = qobject_cast<QWidget*>(QObject::sender()) )
        emit commitData(widget->parentWidget());
}
