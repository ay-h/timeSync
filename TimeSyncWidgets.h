#pragma once

#include <QtWidgets/QWidget>
#include "ui_TimeSyncWidgets.h"
#include "ntpclient/CNtpClient.h"
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>

class CTimeSyncWidgets : public QWidget
{
    Q_OBJECT

public:
    CTimeSyncWidgets(QWidget *parent = nullptr);
    ~CTimeSyncWidgets();

public:
    void InitTrayIcon();

    void SetSysTime(const QDateTime& dateTime);

    bool IsAutoStart();

    bool SetAutoStart(bool isAutoStart);

public slots:
    void StartSyncTime();

	//收到时间返回
	void OnReceiveTime(const QDateTime& dateTime);

	// 自启动设置改变
    void OnAutoRunStateChanged(int state);

    // 是否显示窗口设置改变
	void OnShowWindowStateChanged(int state);

protected:
    void changeEvent(QEvent* event) override;

    void closeEvent(QCloseEvent* event) override;

	void showEvent(QShowEvent* event) override;

	void hideEvent(QHideEvent* event) override;

private:
    Ui::TimeSyncWidgetsClass ui;

    /* 任务栏图标*/
    QSystemTrayIcon* m_trayIcon;

    /* 同步定时器*/
    QTimer* m_syncTimer;

    /* 显示时间定时器*/
    QTimer* m_timer;

    /* ntp 同步类*/
    CNtpClient* m_ntpClient;
};
