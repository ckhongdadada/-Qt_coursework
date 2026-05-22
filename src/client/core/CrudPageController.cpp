#include "CrudPageController.h"
#include "widgets/ToastNotification.h"
#include <QMessageBox>

CrudPageController::CrudPageController(QObject* parent)
    : QObject(parent)
{
}

void CrudPageController::setCreateDialog(CreateDialogFn fn) { m_createDialog = fn; }
void CrudPageController::setEditDialog(EditDialogFn fn) { m_editDialog = fn; }
void CrudPageController::setServiceCreate(ServiceCreateFn fn) { m_serviceCreate = fn; }
void CrudPageController::setServiceUpdate(ServiceUpdateFn fn) { m_serviceUpdate = fn; }
void CrudPageController::setServiceRemove(ServiceRemoveFn fn) { m_serviceRemove = fn; }
void CrudPageController::setServiceGetAll(ServiceGetAllFn fn) { m_serviceGetAll = fn; }
void CrudPageController::setServiceGetById(ServiceGetByIdFn fn) { m_serviceGetById = fn; }
void CrudPageController::setEntityName(const QString& name) { m_entityName = name; }
void CrudPageController::setDataDomain(int domain) { m_dataDomain = domain; }

void CrudPageController::executeAdd(QWidget* parent)
{
    if (!m_createDialog || !m_serviceCreate) return;
    QJsonObject data = m_createDialog(parent);
    if (!data.isEmpty()) {
        m_serviceCreate(data);
        emit dataChanged(m_dataDomain);
        emit operationCompleted(QString("%1已添加。").arg(m_entityName));
    }
}

void CrudPageController::executeEdit(QWidget* parent, int id)
{
    if (!m_editDialog || !m_serviceUpdate || !m_serviceGetById) return;
    if (id <= 0) {
        ToastNotification::display(parent, QString("请先选择一条%1。").arg(m_entityName));
        return;
    }
    QJsonObject existing = m_serviceGetById(id);
    QJsonObject data = m_editDialog(parent, existing);
    if (!data.isEmpty()) {
        m_serviceUpdate(id, data);
        emit dataChanged(m_dataDomain);
        emit operationCompleted(QString("%1已更新。").arg(m_entityName));
    }
}

void CrudPageController::executeRemove(QWidget* parent, int id)
{
    if (!m_serviceRemove) return;
    if (id <= 0) {
        ToastNotification::display(parent, QString("请先选择一条%1。").arg(m_entityName));
        return;
    }
    if (QMessageBox::question(parent, QString("删除%1").arg(m_entityName),
        QString("确定要删除这条%1吗？").arg(m_entityName)) == QMessageBox::Yes) {
        m_serviceRemove(id);
        emit dataChanged(m_dataDomain);
        emit operationCompleted(QString("%1已删除。").arg(m_entityName));
    }
}
