/*!
 *  @file   environment.cpp
 *  @brief  ���ݒ�
 *  @date   2018/03/06
 */
#include "environment.h"

#include "python/python_config.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"
#include "boost/optional.hpp"

//! ���g�̎�Q��
std::weak_ptr<Environment> Environment::m_pInstance;

/*!
 *  @brief  �C���X�^���X����(static)
 *  @return �C���X�^���X���L�|�C���^
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
 *  @brief  python�ݒ�𓾂�(static)
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
 *  @brief  �f�[�^�f�B���N�g���𓾂�(static)
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
 */
Environment::Environment()
: m_python_config()
, m_data_dir()
{
}

/*!
 *  @brief  ������
 */
void Environment::initialize()
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini("boatrace_data_collector.ini", pt);

    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Initialize.Python");
        m_python_config.reset(new garnet::python_config(str.get()));
    }
    {
        boost::optional<std::string> str = pt.get_optional<std::string>("Data.Directory");
        m_data_dir = str.get();
    }
}
