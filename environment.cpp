/*!
 *  @file   environment.cpp
 *  @brief  環境設定
 *  @date   2018/03/06
 */
#include "environment.h"

#include "http/http_proxy_config.h"
#include "python/python_config.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"
#include "boost/optional.hpp"
#include <codecvt>

//! 自身の弱参照
std::weak_ptr<Environment> Environment::m_pInstance;

/*!
 *  @brief  インスタンス生成(static)
 *  @return インスタンス共有ポインタ
 */
std::shared_ptr<Environment> Environment::Create()
{
    if (m_pInstance.lock()) {
        std::shared_ptr<Environment> _empty_instance;
        return _empty_instance;
    } else {
        std::shared_ptr<Environment> _instance(new Environment());
        m_pInstance = _instance;

        _instance->initialize();

        return _instance;
    }
}

/*!
 *  @brief  python設定を得る(static)
 */
const garnet::python_config_ref Environment::GetPythonConfig()
{
    std::shared_ptr<const Environment> p = m_pInstance.lock();
    if (nullptr != p) {
        return p->m_python_config;
    } else {
        return garnet::python_config_ref();
    }
}

/*!
 *  @brief  proxy設定を得る(static)
 */
const web::http::proxy_config_ref Environment::GetProxyConfig()
{
    std::shared_ptr<const Environment> p = m_pInstance.lock();
    if (nullptr != p) {
        return p->m_proxy_config;
    } else {
        return web::http::proxy_config_ref();
    }
}

/*!
 *  @brief  データディレクトリを得る(static)
 */
const std::string Environment::GetDataDir()
{
    std::shared_ptr<const Environment> p = m_pInstance.lock();
    if (nullptr != p) {
        return p->m_data_dir;
    } else {
        return std::string();
    }
}

/*!
 *  @brief  開催スケジュール取得に使うURLを得る(static)
 */
const std::wstring Environment::GetScheduleSiteURL()
{
    std::shared_ptr<const Environment> p = m_pInstance.lock();
    if (nullptr != p) {
        return p->m_schedule_url;
    } else {
        return std::wstring();
    }
}
/*!
 *  @brief  オッズ取得に使うURLを得る(static)
 */
const std::wstring Environment::GetOddsSiteURL()
{
    std::shared_ptr<const Environment> p = m_pInstance.lock();
    if (nullptr != p) {
        return p->m_odds_url;
    } else {
        return std::wstring();
    }
}

/*!
 */
Environment::Environment()
: m_python_config()
, m_data_dir()
{
}

/*!
 *  @brief  初期化
 */
void Environment::initialize()
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini("boatrace_data_collector_config.ini", pt);

    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Config.Python");
        m_python_config.reset(new garnet::python_config(str.get()));
    }
    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Config.Proxy");
        m_proxy_config.reset(new web::http::proxy_config(str.get()));
    }
    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Data.Directory");
        m_data_dir = str.get();
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utfconv;
    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Data.ScheduleURL");
        m_schedule_url = std::move(utfconv.from_bytes(str.get()));
    }
    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Data.OddsURL");
        m_odds_url = std::move(utfconv.from_bytes(str.get()));
    }
}
