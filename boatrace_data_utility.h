/*!
 *  @file   boatrace_data_utility.h
 *  @brief  �����f�[�^���W�F���낢��֗�
 *  @date   2018/03/17
 */
#pragma once

#include "boatrace_data_define.h"

#include <string>

#define PRINT_MESSAGE(msg) PrintMessage(msg, __func__)

namespace boatrace
{

/*!
 *  @brief  ������(�^�O)�����[�X��ʂɕϊ�
 *  @param  tag ���[�X��ʕ�����
 */
eRaceType StrToRaceType(const std::string& tag);

/*!
 *  @brief  ���[�X��ID�𕶎���(���[�X�ꖼ)�ɕϊ�
 *  @param  st_id   ���[�X��ID
 */
std::string StadiumIDToStr(eStadiumID st_id);

/*!
 *  @brief  ���[�X�ŗLID�F��
 */
uint32_t BlankRaceID();
/*!
 *  @brief  ���[�X�ŗLID�F�L����
 *  @retval false   �s��ID
 */
bool IsValidRaceID(uint32_t rc_id);

/*!
 *  @brief  ���[�X�ԍ�(��n���[�X)�F��
 */
int32_t BlankRaceNumber();
/*!
 *  @brief  ���[�X�ԍ��F�ŏ����[�X
 */
int32_t FirstRaceNumber();
/*!
 *  @brief  ���[�X�ԍ��F�ŏI���[�X
 */
int32_t LastRaceNumber();


/*!
 *  @brief  message�o��
 *  @param  msg         message
 *  @param  func_name   �����֐���
 *  @note   �W���o��
 */
void PrintMessage(const std::string& msg, const char* func_name=nullptr);

} // namespace boatrace
