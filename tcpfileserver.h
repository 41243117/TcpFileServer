#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    TcpFileServer(QWidget *parent = 0);
    ~TcpFileServer();
public slots:
    void start();
    void acceptConnection();
    void updateServerProgress();
    void displayError(QAbstractSocket::SocketError socketError);
private:
    QProgressBar     *serverProgressBar;
    QLabel           *serverStatusLabel;
    QPushButton      *startButton;
    QPushButton      *quitButton;
    QDialogButtonBox *buttonBox;
    QLineEdit        *ipLineEdit;
    QLineEdit        *portLineEdit;

    QTcpServer       tcpServer;
    QTcpSocket       *tcpServerConnection;
    qint64           totalBytes;
    qint64           byteReceived;
    qint64           fileNameSize;
    QString          fileName;
    QFile            *localFile;
    QByteArray       inBlock;
};

#endif // TCPFILESERVER_H
