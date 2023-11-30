#ifndef CONTAINERS_MODEL_H
#define CONTAINERS_MODEL_H

#include <QAbstractListModel>
#include <QJsonObject>
#include <utility>
#include <vector>

#include "containers/containers_defs.h"
#include "settings.h"

class ContainersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ContainersModel(std::shared_ptr<Settings> settings, QObject *parent = nullptr);

    enum Roles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        DetailedDescriptionRole,
        ServiceTypeRole,
        ConfigRole,
        DockerContainerRole,

        IsEasySetupContainerRole,
        EasySetupHeaderRole,
        EasySetupDescriptionRole,
        EasySetupOrderRole,

        IsInstalledRole,
        IsCurrentlyProcessedRole,
        IsDefaultRole,
        IsSupportedRole,
        IsShareableRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(const int index, int role) const;

    Q_PROPERTY(QString defaultContainerName READ getDefaultContainerName NOTIFY defaultContainerChanged)

public slots:
    DockerContainer getDefaultContainer();
    QString getDefaultContainerName();
    void setDefaultContainer(int index);

    void setCurrentlyProcessedServerIndex(const int index);

    void setCurrentlyProcessedContainerIndex(int index);
    int getCurrentlyProcessedContainerIndex();

    QString getCurrentlyProcessedContainerName();
    QJsonObject getCurrentlyProcessedContainerConfig();
    QStringList getAllInstalledServicesName(const int serverIndex);

    ErrorCode removeAllContainers();
    ErrorCode removeCurrentlyProcessedContainer();
    void clearCachedProfiles();

    bool isAmneziaDnsContainerInstalled();
    bool isAmneziaDnsContainerInstalled(const int serverIndex);

    bool isAnyContainerInstalled();

    void updateContainersConfig();

protected:
    QHash<int, QByteArray> roleNames() const override;

signals:
    void defaultContainerChanged();
    void containersModelUpdated();

private:
    QMap<DockerContainer, QJsonObject> m_containers;

    int m_currentlyProcessedServerIndex;
    int m_currentlyProcessedContainerIndex;
    DockerContainer m_defaultContainerIndex;

    std::shared_ptr<Settings> m_settings;
};

#endif // CONTAINERS_MODEL_H
