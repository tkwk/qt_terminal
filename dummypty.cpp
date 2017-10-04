#include "dummypty.h"

#include <QGraphicsTextItem>
#include <QFontDatabase>
#include <QDebug>

DummyPty::DummyPty(QWidget *parent) : qpty(parent)
{
    l = 0;
    c = 0;

    int id = QFontDatabase::addApplicationFont(":/fonts/ter-u16n.bdf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    font = QFont(family);
}

void DummyPty::insertString(const char *string)
{
    qDebug() << string;

    for (int i = 0; string[i] != '\0'; i++) {
        switch (string[i]) {
        case '\n':
                l++;
                c = 0;
            break;
        default:
            addText(QString(string[i]), font)->setPos(c*8, l*16);
            c++;
        }
    }
}
