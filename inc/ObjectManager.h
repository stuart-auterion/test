
#pragma once

#include <QObject>

#include "CustomObject.h"
#include "QObjectList.h"

typedef ObjectList<CustomObject> CustomObjectList;

class ObjectManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(CustomObjectList* objects MEMBER _objects CONSTANT)
  public:
    ObjectManager(QObject* parent = nullptr) : QObject(parent) {
        qRegisterMetaType<CustomObjectList*>("CustomObjectList*");
        _objects = new CustomObjectList(this);
        _objects->append(new CustomObject(nullptr, 1));
        _objects->append(new CustomObject(nullptr, 2));
        _objects->append(new CustomObject(nullptr, 3));
        _objects->append(new CustomObject(nullptr, 4));
        _objects->append(new CustomObject(nullptr, 5));
        _objects->append(new CustomObject(nullptr, 6));
    }
    ~ObjectManager() {}
    Q_INVOKABLE void move(int from, int to) {
        _objects->move(from, to);

    }
  signals:

  private:
    CustomObjectList* _objects;
};
