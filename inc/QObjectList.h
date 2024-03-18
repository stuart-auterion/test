/****************************************************************************
 *
 *   (c) 2023-2023 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#pragma once

#include <QAbstractListModel>
#include <QMetaProperty>
#include <QObject>

#include "qdebug.h"

template <class T>
class ObjectList : public QAbstractListModel {
  public:
    ObjectList(QObject* parent = nullptr);
    enum { QObjectRole = -1 };
    /* Virtual function implementations */
    QVariant data(const QModelIndex& index, int role = QObjectRole) const;
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex& parent) const;
    /* Utility functions */
    void append(T* object);
    int count();
    void insert(int index, T* object);
    Q_INVOKABLE void move(const int from, const int to);
    void prepend(T* object);
    bool remove(T* object);
    void removeAll();
    void replace(T* oldObject, T* newObject);
    void replace(int index, T* object);
    T* at(int i);
    QList<T*>& rawList();
    void updateRoleNames(T* object = nullptr);
    /* Property functions */
    void setMixedType(bool mixedType);

    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    Qt::DropActions supportedDropActions() const;
    bool insertRows(int row, int count, const QModelIndex& parent);
    bool insertColumns(int column, int count, const QModelIndex& parent);
    bool setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles);

  private:
    QList<T*> _objects;
    bool _mixedType;
    QHash<int, QByteArray> _roleNames;
};

template <class T>
ObjectList<T>::ObjectList(QObject* parent) : QAbstractListModel(parent) {}

template <class T>
QVariant ObjectList<T>::data(const QModelIndex& index, int role) const {
    if (index.isValid() && (index.row() < _objects.length())) {
        T* object = _objects.at(index.row());
        /* Last index is the "modelData" role we added */
        if (_mixedType || (role == QObjectRole) ||
            (role == object->metaObject()->propertyCount())) {
            return QVariant::fromValue(object);
        }
        /* Each other role corresponds to a Q_PROPERTY */
        else if (role < object->metaObject()->propertyCount()) {
            return object->metaObject()->property(role).read(object);
        }
    }
    return QVariant();
}

template <class T>
QHash<int, QByteArray> ObjectList<T>::roleNames() const {
    return _roleNames;
}

template <class T>
int ObjectList<T>::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return _objects.size();
}

template <class T>
void ObjectList<T>::append(T* object) {
    insert(_objects.length(), object);
}

template <class T>
int ObjectList<T>::count() {
    return _objects.count();
}

template <class T>
void ObjectList<T>::insert(int index, T* object) {
    /* If this hasn't been done yet, use the first object's metadata to define the role names */
    if (_roleNames.isEmpty()) {
        updateRoleNames(object);
    }
    beginInsertRows(QModelIndex(), index, index);
    _objects.insert(index, object);
    endInsertRows();
}

template <class T>
void ObjectList<T>::move(const int from, const int to) {
    if (from == to) {
        return;
    }
    // Real weird note in the documentation about sequential indices here. Honestly not sure why
    // this is necessary...
    // https://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows
    int modelTo = to;
    if ((to - from) == 1) {
        modelTo++;
    }
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);
    moveRow(QModelIndex(), from, QModelIndex(), modelTo);
    _objects.move(from, to);
    endMoveRows();
    for (T* object : _objects) {
        qDebug() << *object;
    }
}

template <class T>
void ObjectList<T>::prepend(T* object) {
    insert(0, object);
}

template <class T>
bool ObjectList<T>::remove(T* object) {
    int index = _objects.indexOf(object);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        T* object = _objects.takeAt(index);
        object->deleteLater();
        endRemoveRows();
        return true;
    }
    return false;
}

template <class T>
void ObjectList<T>::removeAll() {
    beginResetModel();
    qDeleteAll(_objects.begin(), _objects.end());
    _objects.clear();
    endResetModel();
}

template <class T>
void ObjectList<T>::replace(T* oldObject, T* newObject) {
    replace(_objects.indexOf(oldObject), newObject);
}

template <class T>
void ObjectList<T>::replace(int index, T* object) {
    beginInsertRows(QModelIndex(), index, index);
    _objects.replace(index, object);
    endInsertRows();
}

template <class T>
T* ObjectList<T>::at(int i) {
    return _objects[i];
}

template <class T>
QList<T*>& ObjectList<T>::rawList() {
    return _objects;
}

template <class T>
void ObjectList<T>::updateRoleNames(T* object) {
    if ((object == nullptr) && (!_objects.isEmpty())) {
        object = _objects.first();
    }
    if (object != nullptr) {
        int i = 0;
        /* If this list contains different types (i.e. a base class with different derived classes),
         * the properties may be different for each. In this case, only the modelData property can
         * be used. */
        if (!_mixedType) {
            /* Add a role for each of it's Q_PROPERTY values */
            for (; i < object->metaObject()->propertyCount(); i++) {
                _roleNames.insert(i, object->metaObject()->property(i).name());
            }
        }
        /* Add a named role "modelData" for backwards-compatibility with QList-based models */
        _roleNames.insert(i, "modelData");
    }
}

template <class T>
void ObjectList<T>::setMixedType(bool mixedType) {
    _mixedType = mixedType;
    _roleNames.clear();
    updateRoleNames();
}

template <class T>
Qt::ItemFlags ObjectList<T>::flags(const QModelIndex& index) const {
    Q_UNUSED(index)
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

template <class T>
bool ObjectList<T>::setData(const QModelIndex& index, const QVariant& value, int role) {
    qDebug() << Q_FUNC_INFO;
    if (role == QObjectRole) {
        _objects[index.row()] = value.value<T*>();
    }
    emit dataChanged(index, index, {role});
    return true;
}

template <class T>
Qt::DropActions ObjectList<T>::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

template <class T>
bool ObjectList<T>::insertRows(int, int, const QModelIndex&) {
    qDebug() << Q_FUNC_INFO;
    return true;
}

template <class T>
bool ObjectList<T>::insertColumns(int, int, const QModelIndex&) {
    qDebug() << Q_FUNC_INFO;
    return true;
}

template <class T>
bool ObjectList<T>::setItemData(const QModelIndex&, const QMap<int, QVariant>&) {
    qDebug() << Q_FUNC_INFO;
    return true;
}
