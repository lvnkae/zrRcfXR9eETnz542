/*!
 *  @file   environment.h
 *  @brief  ���ݒ�
 *  @date   2018/03/06
 *  @note   ini�t�@�C������
 */
#pragma once

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
     *  @brief  �f�[�^�f�B���N�g���𓾂�
     */
    static const std::string GetDataDir();


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
    std::shared_ptr<garnet::python_config> m_python_config;
    //! �f�[�^�o�̓f�B���N�g��
    std::string m_data_dir;

    //! ���g�̎�Q��
    static std::weak_ptr<Environment> m_pInstance;
};
