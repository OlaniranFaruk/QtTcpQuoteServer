#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QScrollBar>
#include <QThread>

#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__

typedef enum{
    DBG_ALERT,
    DBG_NOTIFY,
    DBG_INFO,
}dbg_severity;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbStart_clicked();
    void handleIncommingClient();
    void clientReadyRead();
    void clientClosedConnection();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    bool m_serverIsListening;
    QTcpServer *m_tcpServer;
    int m_lastQuoteIndex = 0;

private:
    QByteArray getDirectoryList(QStringList filters);
    void appendToLog(dbg_severity s, QString msg);
    QString convertToEscapeString(QByteArray ba);
};

#endif // MAINWINDOW_H
