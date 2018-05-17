/*!
 *  @file   environment.h
 *  @brief  ���ݒ�
 *  @date   2018/03/06
 *  @note   ini�t�@�C������
 */
#pragma once

#include "http/http_proxy_config_fwd.h"
#include "python/python_config_fwd.h"

#include <memory>
#include <string>

/*!
 *  @note   singleton
 *  @note   �C���X�^���X�͖����I�ɐ�������(�ďo����shared_ptr�ŕێ�)
 *  @note   "����C���X�^���X�擾������"�ł͂Ȃ�
 */
class Environment;
class Environment
{
public:
    /*!
     *  @brief  �C���X�^���X���� + ������
     *  @return �C���X�^���X���L�|�C���^
     *  @note   �����ς݂Ȃ��shared_ptr��Ԃ�
     */
    static std::shared_ptr<Environment> Create();
    /*!
     *  @brief  �C���X�^���X�擾
     *  @return �C���X�^���X��Q��(const)
     *  @note   �����O�ɌĂ΂ꂽ���weak_ptr��Ԃ�
     */
    static std::weak_ptr<const Environment> GetInstance() { return m_pInstance; }

    /*!
     *  @brief  python�ݒ�𓾂�
     */
    static const garnet::python_config_ref GetPythonConfig();
    /*!
     *  @brief  proxy�ݒ�𓾂�
     */
    static const web::http::proxy_config_ref GetProxyConfig();
    /*!
     *  @brief  �f�[�^�f�B���N�g���𓾂�
     */
    static const std::string GetDataDir();
    /*!
     *  @brief  �J�ÃX�P�W���[���擾�Ɏg��URL�𓾂�
     */
    static const std::wstring GetScheduleSiteURL();
    /*!
     *  @brief  �I�b�Y�擾�Ɏg��URL�𓾂�
     */
    static const std::wstring GetOddsSiteURL();

private:
    Environment();
    Environment(const Environment&);
    Environment(Environment&&);
    Environment& operator= (const Environment&);

    /*!
     *  @brief  ������
     */
    void initialize();

    //! python�ݒ�  
    garnet::python_config_ptr m_python_config;
    //! proxy�ݒ�  
    web::http::proxy_config_ptr m_proxy_config;
    //! �f�[�^�o�̓f�B���N�g��
    std::string m_data_dir;
    //! �J�ÃX�P�W���[���擾URL
    std::wstring m_schedule_url;
    //! �I�b�Y�擾URL
    std::wstring m_odds_url;

    //! ���g�̎�Q��
    static std::weak_ptr<Environment> m_pInstance;
};
