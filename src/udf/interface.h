/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-08 14:10:13
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-08 14:10:13
 * @FilePath: /postgres-kernel/src/udf/interface.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#ifndef _INTERFACE_H_
#define _INTERFACE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"
#include "fmgr.h"

Datum RegisterCallback(PG_FUNCTION_ARGS);

#ifdef __cplusplus
}
#endif

#endif