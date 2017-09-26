#include "qpty.h"

qpty::qpty(QWidget *parent)
    : QWidget(parent)
{
    palette[0] = QColor("black");
    palette[1] = QColor("red");
    palette[2] = QColor("green");
    palette[3] = QColor("yellow");
    palette[4] = QColor("blue");
    palette[5] = QColor("magenta");
    palette[6] = QColor("cyan");
    palette[7] = QColor("white");

    default_color = QColor("green");
    buffer = new char[B_SIZE + RW_SIZE];
    this->readyForInput = false;
    reader.parent = this;
    buffer[0] = 0;
    fd_in = 0;
    fd_out = 0;
    buffer_cursor = 0;
    max_cursor = B_SIZE + RW_SIZE;

    frames.setInterval(17);
    frames.setSingleShot(false);

    connect(&frames,SIGNAL(timeout()),this,SLOT(updateBuffer()));

    frames.start();
}

void qpty::runProcess(char *command, char *args[], const QString & pref)
{
    this->prefix = QString::number(getpid())+pref;
    this->clear();
    QStringList largs;
    largs << this->prefix;
    for(int i=0;i<1000;i++) {
        if(args[i] == NULL)
            break;
        largs << QString(args[i]);
    }

    pty.start(QString("pseudo_run"), largs );
    if (pty.waitForStarted() == false) {
        this->processString("Error starting command pseudo_run \n");
        return ;
    }

    reader.killed = false;
    reader.start();
}

void qpty::updateBuffer() {
    if(buffer[buffer_cursor] == 0) {
        return;
    }
    if(buffer_cursor >= max_cursor)
        buffer_cursor = 0;
    this->processString(buffer+buffer_cursor);
    buffer_cursor += strlen(buffer+buffer_cursor);
}

void qpty::closeCurrentProcess() {
    reader.killed = true;
    kill(pty.pid(),SIGPIPE);
    reader.wait();
    ::close(fd_in);
    ::close(fd_out);
    pty.waitForFinished();
    std::cout << "peacefully terminated" << std::endl;
}

qpty::~qpty() {
    delete[] buffer;
    if(this->readyForInput)
        closeCurrentProcess();
}

PtyRead::PtyRead() : QThread()
{
    offset = 0;
}

void PtyRead::run()
{
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) {
        std::cout << "Failed to read /etc/passwd" << std::endl;
        kill(parent->pty.pid(),SIGPIPE);
        parent->pty.waitForFinished();
        return;
    }

    //check if pipes are ready
    for(int i=0;i<20;i++) {
        if( access( (pw->pw_dir + QString("/.germinal/")+parent->prefix+QString("_in")).toStdString().c_str(), W_OK ) != -1 )
            break;
        QThread::msleep(250);
    }

    parent->fd_in = open((pw->pw_dir + QString("/.germinal/")+parent->prefix+QString("_in")).toStdString().c_str(), O_WRONLY);
    if(parent->fd_in == -1) {
        std::cout << (pw->pw_dir + QString("/.germinal/")+parent->prefix+QString("_in")).toStdString().c_str() << std::endl;
        std::cout << strerror(errno) << std::endl;
        kill(parent->pty.pid(),SIGPIPE);
        parent->pty.waitForFinished();
        return;
    }
    parent->fd_out = open((pw->pw_dir + QString("/.germinal/")+parent->prefix+QString("_out")).toStdString().c_str(), O_RDONLY);
    if(parent->fd_out == -1) {
        std::cout << (pw->pw_dir + QString("/.germinal/")+parent->prefix+QString("_out")).toStdString().c_str() << std::endl;
        std::cout << strerror(errno) << std::endl;
        ::close(parent->fd_in);
        kill(parent->pty.pid(),SIGPIPE);
        parent->pty.waitForFinished();
        return;
    }

    parent->readyForInput = true;

    fd_set fd_in;
    struct timeval tv;

    killed = false;
    int count = 0;
    while(!killed) {
        FD_ZERO(&fd_in);
        FD_SET(parent->fdOut(),&fd_in);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if(select(parent->fdOut()+1, &fd_in, NULL, NULL, &tv) == 0)
            continue;
        count = read(parent->fdOut(), parent->buffer + offset, RW_SIZE);
        if(count == 0 || (parent->buffer + offset)[0] == 0) continue;

        for(int i=0;i<count-1;i++)
            if((parent->buffer + offset)[i] == 0)
                (parent->buffer + offset)[i] = 255;
        offset += count - 1;

        if(offset > B_SIZE) {
            parent->max_cursor = offset; //offset+1?
            offset = 0;
            parent->buffer[0] = 0;
        }
    }
}

void qpty::pressEnter()
{
    if(readyForInput) {
        write(fd_in, "\n", 1);
    }
}

void qpty::processInput(char c)
{
    char buffer[2];
    buffer[1] = 0;
    buffer[0] = c;
    if(readyForInput) {
        write(fd_in, buffer, 1);
    }
}

void qpty::processInput(const char *c)
{
    if(readyForInput) {
        write(fd_in, c, strlen(c));
        if(strlen(c) == 1 && c[0] == 3) {
            //std::cout << "interrupted" << std::endl;
        }
    }
}

void qpty::processString(const char *array) {
    static QByteArray last_bit;
    QByteArray buf = QByteArray::fromRawData(array,strlen(array));
    QByteArray buffer = last_bit + buf;
    if(strlen(buffer.data()) == 1 && buffer[0] == 7)
        return;
    if(strlen(buffer.data()) == 1 && buffer[0] == 8)
        return;

    bool stop = false;

    QList<QByteArray> liste = buffer.split(27);

    this->insertString(QString(liste[0]).toStdString().c_str());

    //CETTE PARTIE NE S'EXECUTE QUE SI LE CHARACTERE D'ECHAPPEMENT A ETE DETECTE
    for( int j=1; j<liste.count(); j++ ) {
        //interpreter les premiers octets de array
        if(liste[j][0] == 0) {
            stop = true;
            last_bit = char(27) + liste[j];
        }
        if(liste[j][0] == '[') {    //CSI
            //CSI private n1;n2(;) trailings command
            // command \in 64-126
            // ni \in 48-57
            // ; = 59
            QByteArray private_char;
            QByteArray trailing_char;
            char command;
            QList<QByteArray> nums;
            bool done = false;

            int command_pos;

            int i;
            for(i=1;i<10;i++) {
                if(liste[j][i] == 0) {
                    stop = true;
                    done = true;
                    last_bit = char(27) + liste[j];
                    break;
                }
                if(liste[j][i] >= 64 && liste[j][i] <= 126) {
                    command = liste[j][i];
                    done = true;
                    command_pos = i;
                    break;
                }
                if(liste[j][i] > 59 || liste[j][i] < 48)
                    private_char += liste[j][i];
                else
                    break;
            }
            if(!done) {
                int end_num;
                int beg_num = i; //inclu
                for(i=beg_num; i<200; i++) {
                    if(liste[j][i] == 0) {
                        stop = true;
                        done = true;
                        last_bit = char(27) + liste[j];
                        break;
                    }
                    if(liste[j][i] > 59 || liste[j][i] < 48)
                        break;
                }
                end_num = i; //exclu
                if(!stop) {
                    for(i=end_num;i<400;i++) {
                        if(liste[j][i] == 0) {
                            stop = true;
                            done = true;
                            last_bit = char(27) + liste[j];
                            break;
                        }
                        if(liste[j][i] >= 64 && liste[j][i] <= 126) {
                            command = liste[j][i];
                            command_pos = i;
                            break;
                        }
                        else {
                            trailing_char += liste[j][i];
                        }
                    }
                    QByteArray num_part = liste[j].mid(beg_num, end_num-beg_num);
                    nums = num_part.split(';');
                    for(int k=0; k<nums.size(); k++) {
                        if(nums[k].size() == 0) {
                            nums[k] == QByteArray("-1");
                        }
                    }
                }
            }
            if(!stop) {
                switch(command) {
                case 'm':
                    foreach(QByteArray num, nums) {
                        int code = num.toInt();
                        if(code == -1)
                            code = 0;
                        if(code == 0) {
                            this->setColor(default_color);
                        }
                        if(code >= 30 && code <= 37) {
                            this->setColor(palette[code%10]);
                        }
                    }
                    break;
                case 'A': { //cursor up
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursorVerticaly(-n);
                    break;
                }
                case 'B': { //cursor down
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursorVerticaly(n);
                    break;
                }
                case 'C': { //cursor forward
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursorHorizontaly(n);
                    break;
                }
                case 'D': { //cursor back
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursorHorizontaly(-n);
                    break;
                }
                case 'E': { //cursor down + beginning of line
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursor(0,this->cursorY()+n);
                    break;
                }
                case 'F': { //cursor up + beginning of line
                    int n = 1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursor(0,this->cursorY()-n);
                    break;
                }
                case 'G' : { //set to absolute horizontal
                    int n=1;
                    if(nums.size() > 0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    this->moveCursor(n-1,this->cursorY());
                    break;
                }
                case 'f':
                case 'H': { //set to row n, column m

                    //traiter n et m
                    int n=1;
                    int m=1;
                    if(nums.size()>0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 1;
                    if(nums.size()>1)
                        m = nums[1].toInt();
                    if(m == -1)
                        m = 1;
                    n--;
                    m--;
                    this->moveCursor(n,m);


                    break;
                }
                case 'J': {
                    int n=0;
                    if(nums.size()>0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 0;
                    this->clearCursorScreen(n);
                    break;
                }
                case 'K':
                    int n=0;
                    if(nums.size()>0)
                        n = nums[0].toInt();
                    if(n == -1)
                        n = 0;
                    this->clearCursorLine(n);
                    break;
                }
                this->insertString(QString(liste[j].mid(command_pos+1)).toStdString().c_str());
            }
        }
        else {  //pas CSI

        }
    }
    if(!stop)
        last_bit = QByteArray();
}


