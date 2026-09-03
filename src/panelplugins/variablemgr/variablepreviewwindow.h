#ifndef VARIABLEPREVIEWWINDOW_H
#define VARIABLEPREVIEWWINDOW_H

#include "variablepreview.h"

#include <QPointer>
#include <QPixmap>
#include <QWidget>

namespace Cantor
{
class DefaultVariableModel;
class VariablePreviewRequest;
}

class QLabel;
class QCloseEvent;
class QPushButton;
class QResizeEvent;
class QScrollArea;
class QStackedWidget;
class QStandardItemModel;
class QTableView;

class VariablePreviewWindow : public QWidget
{
    Q_OBJECT

public:
    VariablePreviewWindow(Cantor::DefaultVariableModel* model, Cantor::VariablePreviewData::Reference reference, QWidget* parent = nullptr);

    const Cantor::VariablePreviewData::Reference& reference() const;
    void markStale();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:
    void previewRequested(const Cantor::VariablePreviewData::Reference& reference);

private Q_SLOTS:
    void refresh();
    void loadMore();
    void requestFinished();
    void tableCellActivated(const QModelIndex& index);

private:
    void requestPage(qsizetype offset, bool append);
    void showData(const Cantor::VariablePreviewData& data, bool append);
    void showError(const QString& message);
    void setLoading(bool loading);
    void updateDetails(const Cantor::VariablePreviewData& data);
    void updateImage();
    void adjustInitialSize(bool compact = false, const Cantor::VariablePreviewData* data = nullptr);

private:
    QPointer<Cantor::DefaultVariableModel> m_model;
    QPointer<Cantor::VariablePreviewRequest> m_request;
    Cantor::VariablePreviewData::Reference m_reference;
    QStandardItemModel* m_tableModel{nullptr};
    QTableView* m_tableView{nullptr};
    QLabel* m_messageLabel{nullptr};
    QLabel* m_detailsLabel{nullptr};
    QLabel* m_imageLabel{nullptr};
    QLabel* m_zoomLabel{nullptr};
    QLabel* m_staleLabel{nullptr};
    QScrollArea* m_imageScrollArea{nullptr};
    QStackedWidget* m_stack{nullptr};
    QPushButton* m_refreshButton{nullptr};
    QPushButton* m_loadMoreButton{nullptr};
    QPushButton* m_fitImageButton{nullptr};
    QPixmap m_imagePixmap;
    QString m_sizeConfigKey;
    qsizetype m_nextOffset{0};
    bool m_appendRequest{false};
    bool m_initialSizeAdjusted{false};
};

#endif // VARIABLEPREVIEWWINDOW_H
