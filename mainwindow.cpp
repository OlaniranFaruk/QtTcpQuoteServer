#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_serverIsListening = false;
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer,SIGNAL(newConnection()),this,SLOT(handleIncommingClient()));
    appendToLog(DBG_ALERT, "This is a Alert message");
    appendToLog(DBG_NOTIFY, "This is a Notify message");
    appendToLog(DBG_INFO, "This is a Info message");
    appendToLog(DBG_INFO, "Please press start to start the server!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbStart_clicked()
{
    m_serverIsListening = !m_serverIsListening;
    if(m_serverIsListening){
        ui->pbStart->setText("Stop");
        m_tcpServer->close();
        bool succes = m_tcpServer->listen(QHostAddress::Any, ui->spinBox->value() );
        if(succes){
            QString status = QString("Listening at port: %1").arg(m_tcpServer->serverPort());
            appendToLog(DBG_INFO, status);
            ui->lblStatus->setText(status);
        }else{
            m_serverIsListening = false;
            ui->pbStart->setText("Start");
            appendToLog(DBG_ALERT, QString("Error starting the server: %1").arg(m_tcpServer->errorString()));
            ui->lblStatus->setText(QString("Error starting..."));
        }
    }else{
        ui->pbStart->setText("Start");
        m_tcpServer->close();
        appendToLog(DBG_INFO, QString("Stopping the server"));
        ui->lblStatus->setText(QString("Server is Idle [Press Start]"));
    }
}

void MainWindow::handleIncommingClient()
{
    static int id = 0;
    QTcpSocket * client = m_tcpServer->nextPendingConnection();
    client->setProperty("ID",id);
    appendToLog(DBG_INFO, QString("New incomming connection from %1 (id:%2)")
                .arg(client->peerAddress().toString())
                .arg(id));
    connect(client,SIGNAL(readyRead()),this,SLOT(clientReadyRead()));
    connect(client,SIGNAL(disconnected()),this,SLOT(clientClosedConnection()));
    id++;
}

void MainWindow::clientReadyRead()
{
    QStringList quotes;
    quotes
            << "Any sufficiently advanced technology is indistinguishable from magic. - Arthur C. Clarke"
            << "I have not failed. I've just found 10,000 ways that won't work. - Thomas Edison"
            << "The bad news is time flies. The good news is you're the pilot. - Michael Altshuler"
            << "Success is not final, failure is not fatal: it is the courage to continue that counts. - Winston Churchill"
            << "Believe you can and you're halfway there. - Theodore Roosevelt"
            << "Find out who you are and be that person. That's what your soul was put on this earth to be. Find that truth, live that truth, and everything else will come. - Ellen DeGeneres"
            << "We are not our best intentions. We are what we do. - Amy Dickinson"
            << "We all need people who will give us feedback. That's how we improve. - Bill Gates"
            << "Just remember, you can't climb the ladder of success with your hands in your pockets. - Arnold Schwarzenegger"
            << "If you can't make it good, at least make it look good. - Bill Gates"
            << "I am always doing that which I cannot do, in order that I may learn how to do it. - Pablo Picasso"
            << "If everyone is moving forward together, then success takes care of itself. - Henry Ford"
    ;

    //Hier maak ik gebruik van sender() In de lessen hebben we leren werken met lambda's welke een betere manier is.
    //Here I use sender() In the lessons we learned to work with lambda which is a better way.
    QTcpSocket * client = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(client != NULL); //Client mag niet 0 zijn
    QByteArray ba = client->readAll();
    int id = client->property("ID").toInt();
    appendToLog(DBG_INFO, QString("received data from client(id:%1): %2")
                .arg(id)
                .arg(convertToEscapeString(ba)));
    bool ok = false;
    ba = ba.trimmed();
    int aantal = ba.toInt(&ok);
    QStringList tcpdata;
    if(ok){
        for(int i = 0; i < aantal; i++){
            tcpdata.append(quotes.at((m_lastQuoteIndex + i)%quotes.length()));
        }
        m_lastQuoteIndex += aantal;
        client->write(tcpdata.join("\n").toLatin1());
        client->write("\n\n");
    }else{
        client->write("400 BAD request\n\n");
        appendToLog(DBG_ALERT, QString("  (Error 400) received malformed request"));
    }
}

void MainWindow::clientClosedConnection()
{
    QTcpSocket * client = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(client != NULL); //Client mag niet 0 zijn
    int id = client->property("ID").toInt();
    appendToLog(DBG_INFO, QString("Client(id:%1) closed connection")
                .arg(id));
    client->close();
    client->deleteLater();
}

QByteArray MainWindow::getDirectoryList(QStringList filters)
{
    QByteArray ba;
    QDir dir(":/res/");
    dir.setNameFilters(filters);
    QStringList files = dir.entryList();
    foreach(QString file,files){
        file = file.mid(0,file.lastIndexOf(".")); //remove extension
        ba.append(file);
        ba.append("\n");
    }
    ba.append("\n");
    QD << ba;
    return ba;
}

void MainWindow::appendToLog(dbg_severity s, QString msg)
{
    QString formatstring;
    switch(s)
    {
        case DBG_ALERT: formatstring = "<b>%1</b>: <font color=\"Red\">%2</font><br>"; break;
        case DBG_NOTIFY: formatstring = "<b>%1</b>: <font color=\"Orange\">%2</font><br>"; break;
        case DBG_INFO: formatstring = "<b>%1</b>: <font color=\"DarkGreen\">%2</font><br>"; break;
        default: formatstring = "<b>%1</b>: %2<br>"; break;
    }
    QString time = QTime::currentTime().toString("hh:mm:ss.zzz'ms'");
    ui->DebugLog->moveCursor(QTextCursor::End);
    ui->DebugLog->insertHtml(formatstring.arg(time).arg(msg));
    ui->DebugLog->verticalScrollBar()->setValue(0x7FFFFFFF);
}

QString MainWindow::convertToEscapeString(QByteArray ba)
{
    QString ret;
    for(int i = 0; i < ba.size(); i++){
        if(ba[i] >= ' ' && ba[i] <= '~'){
            ret.append((char)ba[i]);
        }else if(ba[i] == '\n'){
            ret.append("\\n");
        }else if(ba[i] == '\r'){
            ret.append("\\r");
        }else if(ba[i] == '\t'){
            ret.append("\\t");
        }else{
            ret.append(QString("\\x")+QString::number((unsigned char)ba[i],16));
        }
    }
    QD << ret;
    return ret;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
