
#pragma once

#include <QObject>

class CustomObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(int value MEMBER _value NOTIFY valueChanged)
  public:
    CustomObject(QObject* parent = nullptr, int value = 0) : QObject(parent) { _value = value; }
    CustomObject(const CustomObject&) {}
    ~CustomObject() {}
    operator QString() const { return QString::number(_value); }
  signals:
    void valueChanged();

  private:
    int _value;
};

Q_DECLARE_METATYPE(CustomObject)
