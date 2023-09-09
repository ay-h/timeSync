#ifndef NTPCLIENT_H
#define NTPCLIENT_H

/**
 * Ntp校时类 作者:feiyangqingyun(QQ:517216493) 2017-02-16
 * 1. 可设置Ntp服务器IP地址。
 * 2. 推荐用默认的阿里云时间服务器 ntp1.aliyun.com
 * 3. 收到时间信号发出。
 * 4. 时间精确到秒。
 */

#include <QObject>
#include <QDateTime>
class QUdpSocket;

#ifdef quc
class Q_DECL_EXPORT CNtpClient : public QObject
#else
class CNtpClient : public QObject
#endif

{
    Q_OBJECT
public:
    explicit CNtpClient(QObject *parent = 0);

private:
    QString m_ntpServerAddr;
    QUdpSocket *udpSocket;

private slots:
    void ReadData();
    void SendNtpRequest();

public Q_SLOTS:
    //设置Ntp服务器IP
    void SetNtpServerAddr(const QString &ntpServerAddr);

    //开始同步时间
    void StartSyncTime();

Q_SIGNALS:
    //收到时间返回
    void ReceiveTime(const QDateTime &dateTime);
};

#endif // NTPCLIENT_H
