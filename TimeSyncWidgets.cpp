#include "TimeSyncWidgets.h"
#include "Config.h"
#include <QProcess>
#include <QThread>
#include <QSettings>
#include <Windows.h>

static const QString AUTO_RUN_REGISTER_PATH = "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
static const QString AUTO_RUN_VALUE_NAME = "timeSync";

CTimeSyncWidgets::CTimeSyncWidgets(QWidget *parent)
    : QWidget(parent)
{  
    ui.setupUi(this);

	// 检查是否设置开机自启动
	ui.AutoRun->setChecked(IsAutoStart());
	ui.IsShowWindow->setChecked(g_pConfig->IsShowWindow());

	/* 连接ui点击同步按钮信号槽 */
    connect(ui.sync_bt, &QPushButton::clicked, this, &CTimeSyncWidgets::StartSyncTime);
	
	/* 连接ui点击自启动复选框信号槽 */
	connect(ui.AutoRun, &QCheckBox::stateChanged, this, &CTimeSyncWidgets::OnAutoRunStateChanged);

	/* 连接ui点击是否显示窗口复选框信号槽 */
	connect(ui.IsShowWindow, &QCheckBox::stateChanged, this, &CTimeSyncWidgets::OnShowWindowStateChanged);


    /* 连接收到同步时间信号槽 */
    m_ntpClient = new CNtpClient();
	m_ntpClient->SetNtpServerAddr(g_pConfig->GetNtpServer());
    connect(m_ntpClient, &CNtpClient::ReceiveTime, this, &CTimeSyncWidgets::OnReceiveTime);

	/* 初始化任务栏托盘*/
	InitTrayIcon();

    /* 定时同步定时器*/
    m_syncTimer = new QTimer(this);

	// 默认120秒同步一次, 和本地时间相差超过5秒则更新本地时间，可通过配置文件修改
    m_syncTimer->setInterval(g_pConfig->GetSystemTimeCheckInterval()*1000);  
	connect(m_syncTimer, &QTimer::timeout, this, &CTimeSyncWidgets::StartSyncTime);
    
    /* 启动定时器*/
    m_syncTimer->start();

	/* 时间显示定时器*/
	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, &QTimer::timeout, [&] {
		// 最小化后不用更新时间
		QDateTime currentTime = QDateTime::currentDateTime();
		ui.label_4->setText(currentTime.toString("  yyyy-MM-dd HH:mm:ss"));
	});

	if (g_pConfig->IsShowWindow())
	{
		m_timer->start();
	}	
}

CTimeSyncWidgets::~CTimeSyncWidgets()
{

}

void CTimeSyncWidgets::InitTrayIcon()
{
	// 创建任务栏图标
	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setIcon(QIcon(":/TimeSyncWidgets/timsync.jfif"));
	m_trayIcon->setToolTip("时间同步小程序");

	QAction* syncAction = new QAction("立即同步", this);
	connect(syncAction, &QAction::triggered, this, &CTimeSyncWidgets::StartSyncTime);

	QAction* quitAction = new QAction("退出", this);
	connect(quitAction, &QAction::triggered, [&]() {
		// 退出程序
		QApplication::quit();
	});

	// 将菜单项添加到菜单中
	QMenu* trayMenu = new QMenu(this);
	//trayMenu->addAction(showAction);
	trayMenu->addAction(syncAction);
	trayMenu->addAction(quitAction);

	// 将菜单设置给任务栏托盘
	m_trayIcon->setContextMenu(trayMenu);
	connect(m_trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger)
		{
			if (isMinimized())
			{
				// 已经最小化后正常显示
				this->showNormal();      // 单击托盘图标时显示窗口
				this->activateWindow(); //显示到最顶层
			}
			else if (!this->isVisible())
			{
				this->showNormal();  // 单击托盘图标时显示窗口
				this->activateWindow(); //显示到最顶层
				//this->show();
			}
			else
			{	
				// 不是最小化时最小化到任务栏
				showMinimized();
				hide();  // 隐藏窗口
			}
			
		}
	});

	// 显示托盘图标
	m_trayIcon->show();
}

void CTimeSyncWidgets::SetSysTime(const QDateTime& dateTime)
{
	// 获取当前时间
	QDateTime currentTime = QDateTime::currentDateTime();

	// 计算要调整的时间差
	qint64 milliseconds = currentTime.msecsTo(dateTime);

	// 如果时间差大于5秒，则更新本地时间
	if (abs(milliseconds) > (g_pConfig->GetTimeOffset()*1000))
	{
		// 创建QProcess对象
		QProcess process;

		// 设置要执行的命令和参数
		QString program = "cmd.exe";
		QStringList arguments;
		arguments << "/C" << "time" << dateTime.time().toString("hh:mm:ss");

		// 设置QProcess以管理员身份运行
		process.setProgram(program);
		process.setArguments(arguments);
		process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args) {
			args->flags |= CREATE_NEW_CONSOLE;
			args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
													});

		// 启动QProcess
		process.start();

		// 等待命令执行完成
		process.waitForFinished(1000);

		// 输出命令执行结果
		//QByteArray output = process.readAllStandardOutput();
		//qDebug() << output;

		// 记录更新系统时间
		ui.label_6->setText(dateTime.toString("  yyyy-MM-dd HH:mm:ss"));
	}

}

bool CTimeSyncWidgets::IsAutoStart()
{
	// 构造一个QSettings对象来操作注册表
	QSettings registrySettings(AUTO_RUN_REGISTER_PATH, QSettings::NativeFormat);

	// 获取注册表中的所有键
	QStringList keys = registrySettings.allKeys();

	// 获取当前可执行程序路径
	auto currentAppPath = "\"" + QCoreApplication::applicationFilePath().replace("/", "\\") + "\"";

	// 遍历所有键，并检查注册表值是否存在
	foreach(const QString & key, keys) 
	{
		QString value = registrySettings.value(key).toString();
		if (value == currentAppPath) 
		{
			// 找到了
			return true;
		}
	}

	return false;
}

bool CTimeSyncWidgets::SetAutoStart(bool isAutoStart)
{
	QSettings registerSetting(AUTO_RUN_REGISTER_PATH, QSettings::NativeFormat);
	if (isAutoStart)
	{
		// 获取当前可执行程序路径
		auto currentAppPath = "\"" + QCoreApplication::applicationFilePath().replace("/", "\\") + "\"";

		// 写入注册表
		registerSetting.setValue(AUTO_RUN_VALUE_NAME, currentAppPath);

		// 检查写入是否成功
		QString readValue = registerSetting.value(AUTO_RUN_VALUE_NAME).toString();
		return  readValue == currentAppPath;
	}
	else
	{
		// 获取注册表中的所有键
		QStringList keys = registerSetting.allKeys();

		// 获取当前可执行程序路径
		auto currentAppPath = "\"" + QCoreApplication::applicationFilePath().replace("/", "\\") + "\"";

		// 遍历所有键，并检查注册表值是否存在
		foreach(const QString & key, keys)
		{
			QString value = registerSetting.value(key).toString();
			if (value == currentAppPath)
			{
				// 找到了对应的值定移除该项
				registerSetting.remove(key);
				return !registerSetting.contains(key);
			}
		}		
	}

	return false;
}

void CTimeSyncWidgets::StartSyncTime()
{
	m_ntpClient->StartSyncTime();
}

void CTimeSyncWidgets::OnReceiveTime(const QDateTime& dateTime)
{
    ui.label_2->setText(dateTime.toString("  yyyy-MM-dd HH:mm:ss"));
	SetSysTime(dateTime);
}

void CTimeSyncWidgets::OnAutoRunStateChanged(int state)
{
	if (state == Qt::Checked)
	{
		SetAutoStart(true);
	}
	else if (state == Qt::Unchecked)
	{
		SetAutoStart(false);
	}
}

void CTimeSyncWidgets::OnShowWindowStateChanged(int state)
{
	const auto isShow = state == Qt::Checked;
	g_pConfig->SetShowWindow(isShow);
}

void CTimeSyncWidgets::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (isMinimized())
		{
			hide();  // 隐藏窗口
			m_trayIcon->show();  // 显示托盘图标
		}
		else
		{
			//m_trayIcon->hide();  // 隐藏托盘图标
			show();  // 显示窗口
			setWindowState(Qt::WindowNoState);  // 确保窗口不处于最小化状态
		}
	}
	QWidget::changeEvent(event);
}

void CTimeSyncWidgets::closeEvent(QCloseEvent* event)
{
	event->ignore();  // 忽略关闭事件
	hide();  // 隐藏窗口到任务栏
}

void CTimeSyncWidgets::showEvent(QShowEvent* event)
{
	QDateTime currentTime = QDateTime::currentDateTime();
	ui.label_4->setText(currentTime.toString("  yyyy-MM-dd HH:mm:ss"));
	m_timer->start();

	QWidget::showEvent(event);
}

void CTimeSyncWidgets::hideEvent(QHideEvent* event)
{
	m_timer->stop();

	QWidget::hideEvent(event);
}
