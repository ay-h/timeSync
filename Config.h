#pragma once
#include <QString>

class CConfig
{
private:
	CConfig() = default;
	~CConfig() = default;
public:
	static CConfig* GetInstance();

public:
	void Init();

	// 加载Json配置文件
	void LoadConfig();

	bool IsShowWindow();

	bool SetShowWindow(bool isShow);

	QString GetNtpServer();

	int GetSystemTimeCheckInterval();

	int GetTimeOffset();

	bool SaveConfig();

private:
	static CConfig* m_pInstance;

private:
	// 启动时是否显示窗口
	bool m_isShowWindow = true;

	// Ntp服务器地址
	QString m_ntpServerUrl = "ntp.aliyun.com";

	// 系统时间检测间隔，单位:秒
	int m_systemTimeCheckInterval = 120;

	// 时间校准偏差，单位:秒
	int m_timeOffset = 5;

};

#define g_pConfig  CConfig::GetInstance()
