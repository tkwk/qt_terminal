#ifndef DUMMYPTY_H
#define DUMMYPTY_H

#include "qpty.h"

class DummyPty : public qpty
{
public:
    explicit DummyPty(QWidget *parent = nullptr);

    virtual void insertString(const char * string);
    virtual void setColor(const QColor &) {}
    virtual void moveCursor(int x, int y) {}
    virtual void moveCursorVerticaly(int dy) {}
    virtual void moveCursorHorizontaly(int dx) {}
    virtual void clearCursorLine(int mode) {} //mode = 0:clear from cursor to end of line, 1:clear from cursor to end of line, 2:clear all the line
    virtual void clearCursorScreen(int mode) {}

    virtual int cursorX() {return 0;}
    virtual int cursorY() {return 0;}
    virtual int sizeX() {return 0;}
    virtual int sizeY() {return 0;}

private:
    int l;
    int c;
};

#endif // DUMMYPTY_H
