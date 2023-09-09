#include "Config.h"
#include "json/json.hpp"
#include <fstream>

using json = nlohmann::json;

// 配置文件路径
static const std::string CONFIG_FILE = "config.json";

CConfig* CConfig::GetInstance()
{
	static CConfig instance;
	return &instance;
}

void CConfig::Init()
{
	LoadConfig();
}

void CConfig::LoadConfig()
{
	// 读取配置文件
	std::ifstream ifs(CONFIG_FILE);
	if (!ifs.is_open())
	{
		return;	
	}

	auto data = json::parse(ifs);		
	if (data.contains("isShowWindow"))
	{
		m_isShowWindow = data["isShowWindow"];
	}

	if (data.contains("ntpServer"))
	{
	    const auto str = data["ntpServer"].get<std::string>();
		m_ntpServerUrl = QString::fromStdString(str);
	}

	if (data.contains("systemTimeCheckInterval"))
	{
		m_systemTimeCheckInterval = data["systemTimeCheckInterval"];
	}

	if (data.contains("timeOffset"))
	{
		m_timeOffset = data["timeOffset"];
	}
}

bool CConfig::IsShowWindow()
{
	return m_isShowWindow;
}

bool CConfig::SetShowWindow(bool isShow)
{
	if (m_isShowWindow != isShow)
	{
		m_isShowWindow = isShow;
		return SaveConfig();
	}

	return true;
}


QString CConfig::GetNtpServer()
{
	return m_ntpServerUrl;
}

int CConfig::GetSystemTimeCheckInterval()
{
	return m_systemTimeCheckInterval;
}

int CConfig::GetTimeOffset()
{
	return m_timeOffset;
}

bool CConfig::SaveConfig()
{
	std::ofstream ofs(CONFIG_FILE);
	if (!ofs.is_open())
	{
		return false;
	}

	auto data = json::object();
	data["isShowWindow"] = m_isShowWindow;
	data["ntpServer"] = m_ntpServerUrl.toStdString();
	data["systemTimeCheckInterval"] = m_systemTimeCheckInterval;
	data["timeOffset"] = m_timeOffset;

	ofs << data.dump(4);
	return true;
}
