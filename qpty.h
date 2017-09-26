#ifndef QPTY_H
#define QPTY_H

#include <QWidget>
#include <QThread>

#include <QTimer>
#include <QProcess>
#include <QLocalSocket>

#include <unistd.h>
#include <fcntl.h>

#include <pstree.h>
#include <signal.h>
#include <iostream>
#include <stdio.h>

#include <pwd.h>
#include <sys/types.h>

#define B_SIZE 150000
#define RW_SIZE 15000

class qpty;

class PtyRead : public QThread {
    Q_OBJECT
public:
    PtyRead();
    void run();

    int offset;
    bool killed;
    qpty * parent;
};

class qpty : public QWidget
{
    Q_OBJECT
    friend class PtyRead;
public:
    explicit qpty(QWidget *parent = nullptr);
    virtual ~qpty();
    void runProcess(char * command, char * args[] = NULL, const QString & pref = "");
    void closeCurrentProcess();

    void clear() {}
    void processString(const char * array);

    int fdOut() {return fd_out;}
    int fdIn() {return fd_in;}

    void pressEnter();
    void processInput(char c);
    void processInput(const char *);

    virtual void insertString(const char * string) = 0;
    virtual void setColor(const QColor &) = 0;
    virtual void moveCursor(int x, int y) = 0;
    virtual void moveCursorVerticaly(int dy) = 0;
    virtual void moveCursorHorizontaly(int dx) = 0;
    virtual void clearCursorLine(int mode) = 0; //mode = 0:clear from cursor to end of line, 1:clear from cursor to end of line, 2:clear all the line
    virtual void clearCursorScreen(int mode) = 0;

    virtual int cursorX() = 0;
    virtual int cursorY() = 0;
    virtual int sizeX() = 0;
    virtual int sizeY() = 0;
public slots:
    void updateBuffer();
protected:
    QColor palette[8];
    QColor default_color;
private:
    bool readyForInput;
    QTimer frames;
    int fd_out;
    int fd_in;

    PtyRead reader;
    QProcess pty;

    QString prefix;

    int max_cursor;
    int buffer_cursor;
    char * buffer;
};

#endif // QPTY_H
