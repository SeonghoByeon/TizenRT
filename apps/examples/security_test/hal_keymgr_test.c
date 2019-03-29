/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#include <tinyara/config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tinyara/security_hal.h>
#include <stress_tool/st_perf.h>
#include "hal_test_utils.h"


/*  Configuration */
#define HAL_KEYMGR_TEST_TRIAL 10
#define HAL_KEYMGR_TEST_LIMIT_TIME 100000000
#define HAL_KEYMGR_TEST_MEM_SIZE 4096

/*
 * Desc: Set key
 */
#define HAL_TEST_KEY_LEN 32
#define HAL_TEST_KEY_SLOT 1
static hal_data g_aes_key_in;
TEST_SETUP(set_key)
{
	ST_START_TEST;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_key_in, HAL_KEYMGR_TEST_MEM_SIZE));
	g_aes_key_in.data_len = HAL_TEST_KEY_LEN;
	memset(g_aes_key_in.data, 0xa5, HAL_TEST_KEY_LEN);

	ST_END_TEST;
}

TEST_TEARDOWN(set_key)
{
	ST_START_TEST;
	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));
	hal_test_free_buffer(&g_aes_key_in);
	ST_END_TEST;
}

TEST_F(set_key)
{
	ST_START_TEST;
	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_set_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT, &g_aes_key_in, NULL));
	ST_END_TEST;
}

/*
 * Desc: Get key
 * Refered https://developer.artik.io/documentation/security-api/see-authentication-test_8c-example.html
 */
static hal_data g_aes_key_out;
TEST_SETUP(get_key)
{
	ST_START_TEST;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_key_in, HAL_KEYMGR_TEST_MEM_SIZE));
	g_aes_key_in.data_len = HAL_TEST_KEY_LEN;
	memset(g_aes_key_in.data, 0xa5, HAL_TEST_KEY_LEN);

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_key_out, HAL_KEYMGR_TEST_MEM_SIZE));

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_set_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT, &g_aes_key_in, NULL));

	ST_END_TEST;
}

TEST_TEARDOWN(get_key)
{
	ST_START_TEST;

	hal_test_free_buffer(&g_aes_key_in);
	hal_test_free_buffer(&g_aes_key_out);

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	ST_END_TEST;
}

TEST_F(get_key)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_get_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT, &g_aes_key_out));

	ST_END_TEST;
}

/*
 * Desc: Remove key in Secure Storage
 */
TEST_SETUP(remove_key)
{
	ST_START_TEST;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_key_in, HAL_KEYMGR_TEST_MEM_SIZE));
	g_aes_key_in.data_len = HAL_TEST_KEY_LEN;
	memset(g_aes_key_in.data, 0xa5, HAL_TEST_KEY_LEN);

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_set_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT, &g_aes_key_in, NULL));

	ST_END_TEST;
}

TEST_TEARDOWN(remove_key)
{
	ST_START_TEST;

	hal_test_free_buffer(&g_aes_key_in);

	ST_END_TEST;
}

TEST_F(remove_key)
{
	ST_START_TEST;
	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));
	ST_END_TEST;
}

/*
 * Desc: Generate key
 * Refered https://developer.artik.io/documentation/security-api/see-authentication-test_8c-example.html
 */
TEST_SETUP(generate_key)
{
	ST_START_TEST;

	ST_END_TEST;
}

TEST_TEARDOWN(generate_key)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	ST_END_TEST;
}

TEST_F(generate_key)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_generate_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	ST_END_TEST;
}

ST_SET_SMOKE_TAIL(HAL_KEYMGR_TEST_TRIAL, HAL_KEYMGR_TEST_LIMIT_TIME, "Set key", set_key);
ST_SET_SMOKE(HAL_KEYMGR_TEST_TRIAL, HAL_KEYMGR_TEST_LIMIT_TIME, "Get key", get_key, set_key);
ST_SET_SMOKE(HAL_KEYMGR_TEST_TRIAL, HAL_KEYMGR_TEST_LIMIT_TIME, "Remove key", remove_key, get_key);
ST_SET_SMOKE(HAL_KEYMGR_TEST_TRIAL, HAL_KEYMGR_TEST_LIMIT_TIME, "Generate key", generate_key, remove_key);
ST_SET_PACK(hal_keymgr, generate_key);

int hal_keymgr_test(void)
{
	ST_RUN_TEST(hal_keymgr);
	ST_RESULT_TEST(hal_keymgr);

	return 0;
}
