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
#include <tinyara/seclink.h>
#include "seclink_drv_req.h"
#include "seclink_drv_utils.h"

/*  Debugging */
#ifdef SLDRV_TAG
#undef SLDRV_TAG
#endif
#define SLDRV_TAG "[SECLINK_DRV_CRYPTO]"

int hd_handle_crypto_request(int cmd, unsigned long arg)
{
	SLDRV_ENTER;

	struct seclink_req *req = (struct seclink_req *)arg;
	if (!req) {
		return -1;
	}

	struct seclink_crypto_info *info = (struct seclink_crypto_info *)req->req_type.crypto;
	if (!info) {
		return -1;
	}

	switch(cmd) {
	case SECLINK_HAL_AESENCRYPT:
		req->res = hal_aes_encrypt(info->input, info->aes_param, info->key_idx, info->output);
		break;
	case SECLINK_HAL_AESDECRYPT:
		req->res = hal_aes_decrypt(info->input, info->aes_param, info->key_idx, info->output);
		break;
	case SECLINK_HAL_RSAENCRYPT:
		req->res = hal_rsa_encrypt(info->input, info->rsa_mode, info->key_idx, info->output);
		break;
	case SECLINK_HAL_RSADECRYPT:
		req->res = hal_rsa_decrypt(info->input, info->rsa_mode, info->key_idx, info->output);
		break;
	}

	return 0;
}
