#include "dummypty.h"

#include <QGraphicsTextItem>

DummyPty::DummyPty(QWidget *parent) : qpty(parent)
{
    l = 0;
    c = 0;
}

void DummyPty::insertString(const char *string)
{
    std::cout << string << std::flush;

    for (int i = 0; string[i] != '\0'; i++) {
        switch (string[i]) {
        case '\n':
                l++;
                c = 0;
            break;
        default:
            addText(QString(string[i]))->setPos(c*10, l*15);
            c++;
        }
    }
}
