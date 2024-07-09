// src/serialize_example.cpp
#include <QFile>
#include <QDataStream>
#include <QDebug>

struct Person {
    QString name;
    int age;
};

// Serialization function for Person struct
QDataStream &operator<<(QDataStream &out, const Person &person) {
    out << person.name << person.age;
    return out;
}

// Deserialization function for Person struct
QDataStream &operator>>(QDataStream &in, Person &person) {
    in >> person.name >> person.age;
    return in;
}

void serializeData() {
    // Create a Person object to serialize
    Person person {"John Doe", 30};

    // Write to a file
    QFile file("person.dat");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << file.errorString();
        return;
    }

    QDataStream out(&file);
    out << person;
    file.close();
}

void deserializeData() {
    // Read from a file
    QFile file("person.dat");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << file.errorString();
        return;
    }

    QDataStream in(&file);
    Person person;
    in >> person;
    file.close();

    qDebug() << "Deserialized Person:" << person.name << person.age;
}
