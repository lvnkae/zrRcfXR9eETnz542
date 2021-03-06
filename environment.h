/*!
 *  @file   environment.h
 *  @brief  環境設定
 *  @date   2018/03/06
 *  @note   iniファイル仲介
 */
#pragma once

#include "http/http_proxy_config_fwd.h"
#include "python/python_config_fwd.h"

#include <memory>
#include <string>

/*!
 *  @note   singleton
 *  @note   インスタンスは明示的に生成する(呼出側がshared_ptrで保持)
 *  @note   "初回インスタンス取得時生成"ではない
 */
class Environment;
class Environment
{
public:
    /*!
     *  @brief  インスタンス生成 + 初期化
     *  @return インスタンス共有ポインタ
     *  @note   生成済みなら空shared_ptrを返す
     */
    static std::shared_ptr<Environment> Create();
    /*!
     *  @brief  インスタンス取得
     *  @return インスタンス弱参照(const)
     *  @note   生成前に呼ばれたら空weak_ptrを返す
     */
    static std::weak_ptr<const Environment> GetInstance() { return m_pInstance; }

    /*!
     *  @brief  python設定を得る
     */
    static const garnet::python_config_ref GetPythonConfig();
    /*!
     *  @brief  proxy設定を得る
     */
    static const web::http::proxy_config_ref GetProxyConfig();
    /*!
     *  @brief  データディレクトリを得る
     */
    static const std::string GetDataDir();
    /*!
     *  @brief  開催スケジュール取得に使うURLを得る
     */
    static const std::wstring GetScheduleSiteURL();
    /*!
     *  @brief  オッズ取得に使うURLを得る
     */
    static const std::wstring GetOddsSiteURL();

private:
    Environment();
    Environment(const Environment&);
    Environment(Environment&&);
    Environment& operator= (const Environment&);

    /*!
     *  @brief  初期化
     */
    void initialize();

    //! python設定  
    garnet::python_config_ptr m_python_config;
    //! proxy設定  
    web::http::proxy_config_ptr m_proxy_config;
    //! データ出力ディレクトリ
    std::string m_data_dir;
    //! 開催スケジュール取得URL
    std::wstring m_schedule_url;
    //! オッズ取得URL
    std::wstring m_odds_url;

    //! 自身の弱参照
    static std::weak_ptr<Environment> m_pInstance;
};
