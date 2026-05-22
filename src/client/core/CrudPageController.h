#pragma once

#include <QObject>
#include <QJsonObject>
#include <functional>

class CrudPageController : public QObject {
    Q_OBJECT

public:
    using CreateDialogFn = std::function<QJsonObject(QWidget*)>;
    using EditDialogFn = std::function<QJsonObject(QWidget*, const QJsonObject&)>;
    using ServiceCreateFn = std::function<void(const QJsonObject&)>;
    using ServiceUpdateFn = std::function<void(int, const QJsonObject&)>;
    using ServiceRemoveFn = std::function<void(int)>;
    using ServiceGetAllFn = std::function<QList<QJsonObject>()>;
    using ServiceGetByIdFn = std::function<QJsonObject(int)>;

    explicit CrudPageController(QObject* parent = nullptr);

    void setCreateDialog(CreateDialogFn fn);
    void setEditDialog(EditDialogFn fn);
    void setServiceCreate(ServiceCreateFn fn);
    void setServiceUpdate(ServiceUpdateFn fn);
    void setServiceRemove(ServiceRemoveFn fn);
    void setServiceGetAll(ServiceGetAllFn fn);
    void setServiceGetById(ServiceGetByIdFn fn);
    void setEntityName(const QString& name);
    void setDataDomain(int domain);

    void executeAdd(QWidget* parent);
    void executeEdit(QWidget* parent, int id);
    void executeRemove(QWidget* parent, int id);

signals:
    void dataChanged(int domain);
    void operationCompleted(const QString& message);

private:
    CreateDialogFn m_createDialog;
    EditDialogFn m_editDialog;
    ServiceCreateFn m_serviceCreate;
    ServiceUpdateFn m_serviceUpdate;
    ServiceRemoveFn m_serviceRemove;
    ServiceGetAllFn m_serviceGetAll;
    ServiceGetByIdFn m_serviceGetById;
    QString m_entityName;
    int m_dataDomain = 0;
};
