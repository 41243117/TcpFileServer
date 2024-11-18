#include "tcpfileserver.h"
#define tr QStringLiteral

TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent)
{
    totalBytes = 0;
    byteReceived = 0;
    fileNameSize = 0;

    // Initialize UI elements
    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel(QStringLiteral("伺服器端就緒"));
    startButton = new QPushButton(QStringLiteral("接收"));
    quitButton = new QPushButton(QStringLiteral("退出"));

    // New input fields for IP and Port
    ipLineEdit = new QLineEdit;
    ipLineEdit->setPlaceholderText("輸入IP");

    portLineEdit = new QLineEdit;
    portLineEdit->setPlaceholderText("輸入Port");

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // Layout configuration
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);
    mainLayout->addWidget(ipLineEdit);  // Add the IP input field to layout
    mainLayout->addWidget(portLineEdit);  // Add the Port input field to layout
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("接收檔案"));

    // Connections
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(&tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));
}

TcpFileServer::~TcpFileServer()
{
}

void TcpFileServer::start()
{
    startButton->setEnabled(false);
    byteReceived = 0;
    fileNameSize = 0;

    // Retrieve IP and Port from the input fields
    QString ipAddress = ipLineEdit->text().trimmed();
    int port = portLineEdit->text().toInt();

    // Check if the input is valid
    if (ipAddress.isEmpty() || port == 0) {
        QMessageBox::warning(this, QStringLiteral("無效的IP或Port"),
                             QStringLiteral("請輸入有效的伺服器IP地址和Port號"));
        startButton->setEnabled(true);
        return;
    }

    // Attempt to start the server on the user-provided IP and Port
    while (!tcpServer.isListening() &&
           !tcpServer.listen(QHostAddress(ipAddress), port)) {
        QMessageBox::StandardButton ret = QMessageBox::critical(this,
                                                                QStringLiteral("啟動伺服器"),
                                                                QStringLiteral("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
                                                                QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            startButton->setEnabled(true);
            return;
        }
    }
    serverStatusLabel->setText(QStringLiteral("監聽中..."));
}

void TcpFileServer::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection(); // Accept the incoming connection
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(updateServerProgress())); // Connect readyRead() signal to updateServerProgress()
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError))); // Connect error signal to displayError()
    serverStatusLabel->setText(QStringLiteral("接受連線"));
    tcpServer.close(); // Stop accepting further connections
}

void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_4_6);

    if (byteReceived <= sizeof(qint64) * 2) {
        if (fileNameSize == 0 && tcpServerConnection->bytesAvailable() >= sizeof(qint64) * 2) {
            in >> totalBytes >> fileNameSize;
            byteReceived += sizeof(qint64) * 2;
        }

        if (fileNameSize != 0 && tcpServerConnection->bytesAvailable() >= fileNameSize) {
            in >> fileName;
            byteReceived += fileNameSize;
            localFile = new QFile(fileName);
            if (!localFile->open(QFile::WriteOnly)) {
                QMessageBox::warning(this, QStringLiteral("應用程式"),
                                     QStringLiteral("無法讀取檔案 %1：\n%2.").arg(fileName)
                                         .arg(localFile->errorString()));
                return;
            }
        } else {
            return;
        }
    }

    if (byteReceived < totalBytes) {
        byteReceived += tcpServerConnection->bytesAvailable();
        inBlock = tcpServerConnection->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    serverProgressBar->setMaximum(totalBytes);
    serverProgressBar->setValue(byteReceived);
    serverStatusLabel->setText(QStringLiteral("已接收 %1 Bytes").arg(byteReceived));

    if (byteReceived == totalBytes) {
        tcpServerConnection->close();
        startButton->setEnabled(true);
        localFile->close();
        start();
    }
}

void TcpFileServer::displayError(QAbstractSocket::SocketError socketError)
{
    QObject *server = qobject_cast<QObject *>(sender());
    if (server == tcpServerConnection) {
        qDebug() << "Hi I am QTcpSocket";
    }
    if (server == &tcpServer) {
        qDebug() << "Hi I am QTcpServer";
    }

    QMessageBox::information(this, QStringLiteral("網絡錯誤"),
                             QStringLiteral("產生如下錯誤: %1.").arg(tcpServerConnection->errorString()));
    tcpServerConnection->close();
    serverProgressBar->reset();
    serverStatusLabel->setText(QStringLiteral("伺服器就緒"));
    startButton->setEnabled(true);
}






