#include "TimeSyncWidgets.h"
#include <QtWidgets/QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include "Config.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// 创建或打开共享内存,使用共享内存来实现只能运行一个实例
	QSharedMemory sharedMemory("timeSynShareMemory");
	if (!sharedMemory.create(1))
	{
		// 共享内存已经存在，表示已经有一个实例在运行
		QMessageBox::information(nullptr, "重复运行", "不允许启动多个实例!");
		return 0;
	}

	// 加载配置文件
	g_pConfig->Init();


	CTimeSyncWidgets widgetWindow;

	// 判断启动时显示主窗口
	if (g_pConfig->IsShowWindow())
	{
		widgetWindow.show();
	}

	// 启动后先同步一次
	widgetWindow.StartSyncTime();

	// 在程序退出时清理共享内存
	QObject::connect(&a, &QApplication::aboutToQuit, [&]() {
		sharedMemory.detach();
	});

    return a.exec();
}
