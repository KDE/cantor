#include "variablepreviewwindow.h"

#include "defaultvariablemodel.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QCloseEvent>
#include <QColor>
#include <QFont>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

#include <utility>

namespace
{
constexpr qsizetype PreviewPageSize = 200;

enum DataRole
{
    VariableNameRole = Qt::UserRole + 1,
    DisplayNameRole,
    BackendDataRole,
    PreviewTypeRole
};

QString columnTitle(const QString& name)
{
    if (name == QLatin1String("@index"))
        return i18nc("@title:column", "Index");
    if (name == QLatin1String("@key"))
        return i18nc("@title:column", "Key");
    if (name == QLatin1String("@type"))
        return i18nc("@title:column", "Type");
    if (name == QLatin1String("@size"))
        return i18nc("@title:column", "Size [Bytes]");
    if (name == QLatin1String("@value"))
        return i18nc("@title:column", "Value");
    return name;
}

QColor blendedColor(const QColor& base, const QColor& foreground, qreal foregroundRatio)
{
    return QColor::fromRgbF(base.redF() * (1.0 - foregroundRatio) + foreground.redF() * foregroundRatio, base.greenF() * (1.0 - foregroundRatio) + foreground.greenF() * foregroundRatio, base.blueF() * (1.0 - foregroundRatio) + foreground.blueF() * foregroundRatio, base.alphaF());
}
}

VariablePreviewWindow::VariablePreviewWindow(Cantor::DefaultVariableModel* model, Cantor::VariablePreviewData::Reference reference, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , m_model(model)
    , m_reference(std::move(reference))
    , m_tableModel(new QStandardItemModel(this))
    , m_tableView(new QTableView(this))
    , m_messageLabel(new QLabel(this))
    , m_detailsLabel(new QLabel(this))
    , m_imageLabel(new QLabel(this))
    , m_zoomLabel(new QLabel(this))
    , m_staleLabel(new QLabel(i18n("The variable may have changed."), this))
    , m_imageScrollArea(new QScrollArea(this))
    , m_stack(new QStackedWidget(this))
    , m_refreshButton(new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Refresh"), this))
    , m_loadMoreButton(new QPushButton(QIcon::fromTheme(QStringLiteral("go-down")), i18n("Load More"), this))
    , m_fitImageButton(new QPushButton(QIcon::fromTheme(QStringLiteral("zoom-fit-best")), i18n("Fit to Window"), this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18nc("@title:window", "%1 — Variable Preview", m_reference.displayName));
    resize(640, 400);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() + 1.0);
    titleLabel->setFont(titleFont);
    titleLabel->setText(m_reference.displayName);
    titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(titleLabel);

    m_detailsLabel->setVisible(false);
    m_detailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QPalette detailsPalette = m_detailsLabel->palette();
    detailsPalette.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
    m_detailsLabel->setPalette(detailsPalette);
    layout->addWidget(m_detailsLabel);

    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    m_stack->addWidget(m_messageLabel);

    m_tableView->setModel(m_tableModel);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setTextElideMode(Qt::ElideRight);
    m_tableView->setWordWrap(false);
    m_tableView->setShowGrid(true);
    m_tableView->setGridStyle(Qt::SolidLine);
    m_tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QPalette tablePalette = m_tableView->palette();
    const QColor tableBase = tablePalette.color(QPalette::Base);
    const QColor tableText = tablePalette.color(QPalette::Text);
    tablePalette.setColor(QPalette::AlternateBase, blendedColor(tableBase, tableText, 0.035));
    tablePalette.setColor(QPalette::Mid, blendedColor(tableBase, tableText, 0.12));
    m_tableView->setPalette(tablePalette);

    auto* horizontalHeader = m_tableView->horizontalHeader();
    QFont headerFont = horizontalHeader->font();
    headerFont.setBold(true);
    horizontalHeader->setFont(headerFont);
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader->setHighlightSections(false);
    horizontalHeader->setSectionsClickable(false);
    horizontalHeader->setTextElideMode(Qt::ElideRight);
    horizontalHeader->setMinimumHeight(horizontalHeader->fontMetrics().height() + 12);

    auto* verticalHeader = m_tableView->verticalHeader();
    verticalHeader->setDefaultSectionSize(m_tableView->fontMetrics().height() + 12);
    verticalHeader->hide();
    m_stack->addWidget(m_tableView);

    m_imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap transparencyBackground(16, 16);
    const QColor backgroundColor = palette().color(QPalette::Base);
    const QColor alternateColor = backgroundColor.lightness() < 128 ? backgroundColor.lighter(140) : backgroundColor.darker(115);
    transparencyBackground.fill(backgroundColor);
    {
        QPainter painter(&transparencyBackground);
        painter.fillRect(0, 0, 8, 8, alternateColor);
        painter.fillRect(8, 8, 8, 8, alternateColor);
    }
    QPalette imagePalette = m_imageLabel->palette();
    imagePalette.setBrush(QPalette::Window, QBrush(transparencyBackground));
    m_imageLabel->setPalette(imagePalette);
    m_imageLabel->setAutoFillBackground(true);
    m_imageScrollArea->setAlignment(Qt::AlignCenter);
    m_imageScrollArea->setWidget(m_imageLabel);
    m_imageScrollArea->setWidgetResizable(false);
    m_stack->addWidget(m_imageScrollArea);
    layout->addWidget(m_stack, 1);

    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
    m_staleLabel->setVisible(false);
    buttonLayout->addWidget(m_staleLabel);
    buttonLayout->addStretch();
    m_loadMoreButton->setVisible(false);
    buttonLayout->addWidget(m_loadMoreButton);
    m_zoomLabel->setVisible(false);
    buttonLayout->addWidget(m_zoomLabel);
    m_fitImageButton->setCheckable(true);
    m_fitImageButton->setChecked(true);
    m_fitImageButton->setVisible(false);
    buttonLayout->addWidget(m_fitImageButton);
    buttonLayout->addWidget(m_refreshButton);
    auto* closeButton = new QPushButton(QIcon::fromTheme(QStringLiteral("window-close")), i18n("Close"), this);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    connect(m_refreshButton, &QPushButton::clicked, this, &VariablePreviewWindow::refresh);
    connect(m_loadMoreButton, &QPushButton::clicked, this, &VariablePreviewWindow::loadMore);
    connect(m_fitImageButton, &QPushButton::toggled, this, [this]() {
        updateImage();
    });
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_tableView, &QTableView::doubleClicked, this, &VariablePreviewWindow::tableCellActivated);
    connect(model, &QObject::destroyed, this, &QWidget::close);

    refresh();
}

void VariablePreviewWindow::closeEvent(QCloseEvent* event)
{
    if (!m_sizeConfigKey.isEmpty())
    {
        KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("VariablePreviewWindow"));
        config.writeEntry(m_sizeConfigKey, size());
        config.sync();
    }
    QWidget::closeEvent(event);
}

void VariablePreviewWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_fitImageButton && m_fitImageButton->isChecked())
        QTimer::singleShot(0, this, &VariablePreviewWindow::updateImage);
}

const Cantor::VariablePreviewData::Reference& VariablePreviewWindow::reference() const
{
    return m_reference;
}

void VariablePreviewWindow::markStale()
{
    m_staleLabel->setVisible(true);
}

void VariablePreviewWindow::refresh()
{
    m_tableModel->clear();
    m_nextOffset = 0;
    m_staleLabel->setVisible(false);
    requestPage(0, false);
}

void VariablePreviewWindow::loadMore()
{
    requestPage(m_nextOffset, true);
}

void VariablePreviewWindow::requestPage(qsizetype offset, bool append)
{
    if (!m_model || m_request)
        return;

    m_appendRequest = append;
    setLoading(true);
    m_request = m_model->requestVariablePreview(m_reference, offset, PreviewPageSize, this);
    if (!m_request)
    {
        showError(i18n("The backend could not create a preview request."));
        setLoading(false);
        return;
    }

    connect(m_request, &Cantor::VariablePreviewRequest::finished, this, &VariablePreviewWindow::requestFinished);
}

void VariablePreviewWindow::requestFinished()
{
    if (!m_request)
        return;

    const QString error = m_request->errorMessage();
    if (error.isEmpty())
        showData(m_request->data(), m_appendRequest);
    else
        showError(error);

    m_request->deleteLater();
    m_request = nullptr;
    setLoading(false);
}

void VariablePreviewWindow::showData(const Cantor::VariablePreviewData& data, bool append)
{
    updateDetails(data);

    if (data.type == Cantor::VariablePreviewData::Type::Image)
    {
        if (!m_imagePixmap.loadFromData(data.imageData))
        {
            showError(i18n("The image data could not be decoded."));
            return;
        }

        m_fitImageButton->setVisible(true);
        m_zoomLabel->setVisible(true);
        m_stack->setCurrentWidget(m_imageScrollArea);
        m_loadMoreButton->setVisible(false);
        adjustInitialSize(false, &data);
        QTimer::singleShot(0, this, &VariablePreviewWindow::updateImage);
        return;
    }

    m_fitImageButton->setVisible(false);
    m_zoomLabel->setVisible(false);

    if (data.type != Cantor::VariablePreviewData::Type::Table && data.type != Cantor::VariablePreviewData::Type::Dictionary)
    {
        showError(i18n("Preview is not supported for this variable."));
        return;
    }

    if (!append && data.totalRows == 0 && data.rows.isEmpty())
    {
        QPalette messagePalette = m_messageLabel->palette();
        messagePalette.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
        m_messageLabel->setPalette(messagePalette);
        m_messageLabel->setText(i18n("This variable is empty."));
        m_stack->setCurrentWidget(m_messageLabel);
        m_loadMoreButton->setVisible(false);
        adjustInitialSize(true, &data);
        return;
    }

    if (!append)
    {
        m_tableModel->clear();
        m_tableModel->setColumnCount(data.columnNames.size());
        const QPalette tablePalette = m_tableView->palette();
        const QColor tableBase = tablePalette.color(QPalette::Base);
        const QColor tableText = tablePalette.color(QPalette::Text);
        const QColor headerBackground = blendedColor(tableBase, tableText, 0.08);
        const QFont headerFont = m_tableView->horizontalHeader()->font();
        for (int column = 0; column < data.columnNames.size(); ++column)
        {
            const QString& name = data.columnNames.at(column);
            auto* headerItem = new QStandardItem(columnTitle(name));
            headerItem->setFont(headerFont);
            headerItem->setBackground(headerBackground);
            headerItem->setForeground(tableText);
            bool numericHeader = false;
            name.toInt(&numericHeader);
            Qt::Alignment alignment = Qt::AlignLeft;
            if (name == QLatin1String("@index"))
                alignment = Qt::AlignHCenter;
            else if (name == QLatin1String("@size"))
                alignment = Qt::AlignRight;
            else if (numericHeader)
                alignment = Qt::AlignHCenter;
            headerItem->setTextAlignment(alignment | Qt::AlignVCenter);
            m_tableModel->setHorizontalHeaderItem(column, headerItem);
        }
    }

    for (const auto& row : data.rows)
    {
        QList<QStandardItem*> items;
        items.reserve(row.size());
        for (int column = 0; column < row.size(); ++column)
        {
            const auto& cell = row.at(column);
            auto* item = new QStandardItem(cell.value);
            QString toolTip = cell.value;
            const QString type = cell.type.toLower();
            const QString columnName = data.columnNames.value(column);
            if (columnName == QLatin1String("@index"))
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            else if (type.contains(QLatin1String("bool")))
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            else if (columnName == QLatin1String("@size") || type.contains(QLatin1String("int")) || type.contains(QLatin1String("float")) || type.contains(QLatin1String("double")) || type.contains(QLatin1String("number")) || type.contains(QLatin1String("real")))
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            if (cell.reference.isPreviewable())
            {
                item->setData(cell.reference.variableName, VariableNameRole);
                item->setData(cell.reference.displayName, DisplayNameRole);
                item->setData(cell.reference.backendData, BackendDataRole);
                item->setData(static_cast<int>(cell.reference.type), PreviewTypeRole);
                if (!toolTip.isEmpty())
                    toolTip += QLatin1Char('\n');
                toolTip += i18n("Double-click to preview this value.");
            }
            item->setToolTip(toolTip);
            items.append(item);
        }
        m_tableModel->appendRow(items);
    }

    m_nextOffset = data.offset + data.rows.size();
    m_loadMoreButton->setVisible(data.hasMore);
    m_stack->setCurrentWidget(m_tableView);
    if (!append)
    {
        auto* header = m_tableView->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::ResizeToContents);
        const int valueColumn = data.columnNames.indexOf(QStringLiteral("@value"));
        if (valueColumn >= 0)
            header->setSectionResizeMode(valueColumn, QHeaderView::Stretch);
        adjustInitialSize(false, &data);
    }
}

void VariablePreviewWindow::showError(const QString& message)
{
    m_detailsLabel->clear();
    m_detailsLabel->setVisible(false);
    m_fitImageButton->setVisible(false);
    m_zoomLabel->setVisible(false);
    QPalette messagePalette = m_messageLabel->palette();
    messagePalette.setColor(QPalette::WindowText, palette().color(QPalette::Text));
    m_messageLabel->setPalette(messagePalette);
    m_messageLabel->setText(message);
    m_stack->setCurrentWidget(m_messageLabel);
    m_loadMoreButton->setVisible(false);
    adjustInitialSize(true);
}

void VariablePreviewWindow::updateDetails(const Cantor::VariablePreviewData& data)
{
    QStringList details;
    if (!data.typeName.isEmpty())
        details.append(data.typeName);

    QString dimensions = data.dimensions;
    dimensions.replace(QChar(0x00d7), QLatin1Char('x'));
    const QStringList dimensionParts = dimensions.split(QLatin1Char('x'), Qt::SkipEmptyParts);
    if (data.type == Cantor::VariablePreviewData::Type::Image && dimensionParts.size() == 2)
        details.append(i18n("%1 × %2 pixels", dimensionParts.at(0).trimmed(), dimensionParts.at(1).trimmed()));
    else if (data.type == Cantor::VariablePreviewData::Type::Table && dimensionParts.size() == 2)
    {
        details.append(i18n("%1 rows × %2 columns", dimensionParts.at(0).trimmed(), dimensionParts.at(1).trimmed()));
    }

    if (data.type == Cantor::VariablePreviewData::Type::Dictionary)
        details.append(i18np("%1 entry", "%1 entries", data.totalRows));
    else if (data.type == Cantor::VariablePreviewData::Type::Table && dimensionParts.size() != 2)
        details.append(i18np("%1 item", "%1 items", data.totalRows));
    else if (data.type == Cantor::VariablePreviewData::Type::Image && !data.mimeType.isEmpty())
        details.append(data.mimeType);

    m_detailsLabel->setText(details.join(QStringLiteral(" · ")));
    m_detailsLabel->setVisible(!details.isEmpty());
}

void VariablePreviewWindow::updateImage()
{
    if (m_imagePixmap.isNull())
        return;

    if (m_fitImageButton->isChecked())
    {
        QSize availableSize = m_imageScrollArea->viewport()->size() - QSize(4, 4);
        availableSize.setWidth(qMax(1, availableSize.width()));
        availableSize.setHeight(qMax(1, availableSize.height()));
        const QPixmap scaled = m_imagePixmap.scaled(availableSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_imageLabel->setPixmap(scaled);
        m_imageLabel->resize(scaled.size());
        m_zoomLabel->setText(i18nc("image zoom level", "%1%", qRound(100.0 * scaled.width() / m_imagePixmap.width())));
    }
    else
    {
        m_imageLabel->setPixmap(m_imagePixmap);
        m_imageLabel->resize(m_imagePixmap.size());
        m_zoomLabel->setText(i18nc("image zoom level", "%1%", 100));
    }
}

void VariablePreviewWindow::adjustInitialSize(bool compact, const Cantor::VariablePreviewData* data)
{
    if (m_initialSizeAdjusted)
        return;

    m_initialSizeAdjusted = true;
    if (compact)
        m_sizeConfigKey.clear();
    else if (m_stack->currentWidget() == m_imageScrollArea)
        m_sizeConfigKey = QStringLiteral("ImageSizeV2");
    else if (m_reference.type == Cantor::VariablePreviewData::Type::Dictionary)
        m_sizeConfigKey = QStringLiteral("DictionarySizeV2");
    else if (data && data->columnNames.contains(QStringLiteral("@value")))
        m_sizeConfigKey = data->totalRows <= 20 ? QStringLiteral("SmallSequenceSizeV2") : QStringLiteral("LargeSequenceSizeV2");
    else if (data)
        m_sizeConfigKey = data->totalRows <= 20 && data->columnNames.size() <= 10 ? QStringLiteral("SmallMatrixSizeV2") : QStringLiteral("LargeMatrixSizeV2");
    else
        m_sizeConfigKey = QStringLiteral("TableSizeV2");

    if (!m_sizeConfigKey.isEmpty())
    {
        const KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("VariablePreviewWindow"));
        const QSize storedSize = config.readEntry(m_sizeConfigKey, QSize());
        if (storedSize.isValid())
        {
            resize(storedSize);
            return;
        }
    }

    if (compact)
    {
        resize(520, 260);
        return;
    }

    if (!m_imagePixmap.isNull() && m_stack->currentWidget() == m_imageScrollArea)
    {
        resize(qBound(520, m_imagePixmap.width() + 80, 1000), qBound(320, m_imagePixmap.height() + 150, 760));
        return;
    }

    int width = 80;
    for (int column = 0; column < m_tableModel->columnCount(); ++column)
        width += m_tableView->columnWidth(column);
    const int visibleRows = qMin(12, m_tableModel->rowCount());
    int height = 170 + m_tableView->horizontalHeader()->height();
    for (int row = 0; row < visibleRows; ++row)
        height += m_tableView->rowHeight(row);
    resize(qBound(520, width, 1000), qBound(300, height, 720));
}

void VariablePreviewWindow::setLoading(bool loading)
{
    m_refreshButton->setEnabled(!loading);
    m_loadMoreButton->setEnabled(!loading);
    if (loading && !m_appendRequest)
    {
        m_messageLabel->setText(i18n("Loading preview…"));
        m_stack->setCurrentWidget(m_messageLabel);
    }
}

void VariablePreviewWindow::tableCellActivated(const QModelIndex& index)
{
    QModelIndex previewIndex = index;
    int type = previewIndex.data(PreviewTypeRole).toInt();
    if (type == static_cast<int>(Cantor::VariablePreviewData::Type::Unsupported))
    {
        for (int column = 0; column < m_tableModel->columnCount(); ++column)
        {
            const QModelIndex candidate = m_tableModel->index(index.row(), column);
            const int candidateType = candidate.data(PreviewTypeRole).toInt();
            if (candidateType != static_cast<int>(Cantor::VariablePreviewData::Type::Unsupported))
            {
                previewIndex = candidate;
                type = candidateType;
                break;
            }
        }
    }
    if (type == static_cast<int>(Cantor::VariablePreviewData::Type::Unsupported))
        return;

    Cantor::VariablePreviewData::Reference reference;
    reference.variableName = previewIndex.data(VariableNameRole).toString();
    reference.displayName = previewIndex.data(DisplayNameRole).toString();
    reference.backendData = previewIndex.data(BackendDataRole).toByteArray();
    reference.type = static_cast<Cantor::VariablePreviewData::Type>(type);
    Q_EMIT previewRequested(reference);
}
