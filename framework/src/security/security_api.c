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
#include <stdlib.h>
#include <pthread.h>
#include <security/security_api.h>
#include <tinyara/security_hal.h>
#include <tinyara/seclink.h>

#include "security_utils.h"
#include "security_common.h"


#define SECAPI_LOG printf

#define SECAPI_TAG "[SECAPI]"

#define SECTEST_ERR														\
	do {																\
		printf("[ERR] %s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);	\
	} while (0)

#define SECAPI_CALL(func)					\
    do {                                        \
        int sectest_res = func;					\
        if (sectest_res != HAL_SUCCESS) {		\
            SECTEST_ERR;                        \
        }                                       \
    } while (0)

#define SECAPI_ENTER													\
	do {																\
		SECAPI_LOG(SECAPI_TAG "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);	\
	} while (0)

#define SECAPI_H2SD(hal, sec)\
	do {\
		sec->data = hal.data;					\
		sec->length = hal.data_len;				\
	} while (0)

#define SECAPI_H2SD_MEMCPY(hal, sec)\
	do {\
		memcpy(sec->data, hal.data, hal.data_len);  \
		sec->length = hal.data_len;				    \
	} while (0)

#define SECAPI_H2SD_MEMCPY_PRIV(hal, sec)\
	do {\
		memcpy(sec->data, hal.priv, hal.priv_len);  \
		sec->length = hal.priv_len;				    \
	} while (0)

#define SS_PATH "ss/"

/*  Global */
sl_ctx g_slink_hnd = NULL;

/**
 * Private
 */

int sec_key_tb[] = {
	/*  AES */
	HAL_KEY_AES_128,// 128 bits aes algorithm
	HAL_KEY_AES_192, // 192 bits aes algorithm
	HAL_KEY_AES_256, // 256 bits aes algorithm
	/*  RSA */
	HAL_KEY_RSA_1024, // 1024 bits rsa algorithm
	HAL_KEY_RSA_2048, // 2048 bits rsa algorithm
	HAL_KEY_RSA_3072, // 3072 bits rsa algorithm
	HAL_KEY_RSA_4096,
	/*  ECC: it doesn't support whole algorithm that mbedTLS support. it's have to be added*/
	HAL_KEY_ECC_BRAINPOOL_P256R1, // ecc brainpool curve for p256r1
	HAL_KEY_ECC_BRAINPOOL_P384R1, // ecc brainpool curve for p384r1
	HAL_KEY_ECC_BRAINPOOL_P512R1, // ecc brainpool curve for p512r1
	HAL_KEY_ECC_SEC_P192R1, // nist curve for p192r1
	HAL_KEY_ECC_SEC_P224R1, // nist curve for p224r1
	HAL_KEY_ECC_SEC_P256R1, // nist curve for p256r1
	HAL_KEY_ECC_SEC_P384R1, // nist curve for p384r1
	HAL_KEY_ECC_SEC_P512R1, // nist curve for p512r1
	/*  Hmac */
	HAL_KEY_HMAC_MD5, // hmac with md5
	HAL_KEY_HMAC_SHA1, // hmac with sha1
	HAL_KEY_HMAC_SHA224, // hmac with sha224
	HAL_KEY_HMAC_SHA256, // hmac with sha256
	HAL_KEY_HMAC_SHA384, // hmac with sha384
	HAL_KEY_HMAC_SHA512, // hmac with sha512
	/* DH */
	HAL_KEY_DH_1024,
	HAL_KEY_DH_2048,
	HAL_KEY_DH_4096,
	HAL_KEY_UNKNOWN,
};

hal_key_type _convert_algo_stoh(security_algorithm sec)
{
	return sec_key_tb[sec];
}

hal_hash_type _convert_hash_stoh(security_hash_type sec)
{
	switch (sec) {
	case HASH_MD5:
		return HAL_HASH_MD5;
	case HASH_SHA1:
		return HAL_HASH_SHA1;
	case HASH_SHA224:
		return HAL_HASH_SHA224;
	case HASH_SHA256:
		return HAL_HASH_SHA256;
	case HASH_SHA384:
		return HAL_HASH_SHA384;
	case HASH_SHA512:
		return HAL_HASH_SHA512;
	default:
		return HAL_HASH_UNKNOWN;
	}
	return HAL_HASH_UNKNOWN;
}

hal_hmac_type _convert_hmac_stoh(security_algorithm sec) {
	switch (sec) {
	case HMAC_MD5:
		return HAL_HMAC_MD5;
	case HMAC_SHA1:
		return HAL_HMAC_SHA1;
	case HMAC_SHA224:
		return HAL_HMAC_SHA224;
	case HMAC_SHA256:
		return HAL_HMAC_SHA256;
	case HMAC_SHA384:
		return HAL_HMAC_SHA384;
	case HMAC_SHA512:
		return HAL_HMAC_SHA512;
	default:
		return HAL_HMAC_UNKNOWN;
	}
	return HAL_HMAC_UNKNOWN;
}

hal_aes_algo _convert_aes_stoh(security_aes_algo am)
{
	switch (am) {
	case AES_ECB_NOPAD:
		return HAL_AES_ECB_NOPAD;
	case AES_ECB_ISO9797_M1:
		return AES_ECB_ISO9797_M1;
	case AES_ECB_ISO9797_M2:
		return HAL_AES_ECB_ISO9797_M2;
	case AES_ECB_PKCS5:
		return HAL_AES_ECB_PKCS5;
	case AES_ECB_PKCS7:
		return HAL_AES_ECB_PKCS7;
	case AES_CBC_NOPAD:
		return HAL_AES_CBC_NOPAD;
	case AES_CBC_ISO9797_M1:
		return HAL_AES_CBC_ISO9797_M1;
	case AES_CBC_ISO9797_M2:
		return HAL_AES_CBC_ISO9797_M1;
	case AES_CBC_PKCS5:
		return HAL_AES_CBC_PKCS5;
	case AES_CBC_PKCS7:
		return HAL_AES_CBC_PKCS7;
	case AES_CTR:
		return HAL_AES_CTR;
	default:
		return HAL_AES_UNKNOWN;
	}
	return HAL_AES_UNKNOWN;
}

hal_ecdsa_curve _convert_curve_stoh(security_ecdsa_curve curve)
{
	switch (curve) {
	case ECDSA_BRAINPOOL_P256R1:
		return HAL_ECDSA_BRAINPOOL_P256R1;
	case ECDSA_BRAINPOOL_P384R1:
		return HAL_ECDSA_BRAINPOOL_P384R1;
	case ECDSA_BRAINPOOL_P512R1:
		return HAL_ECDSA_BRAINPOOL_P512R1;
	case ECDSA_SEC_P192R1:
		return HAL_ECDSA_SEC_P192R1;
	case ECDSA_SEC_P224R1:
		return HAL_ECDSA_SEC_P224R1;
	case ECDSA_SEC_P256R1:
		return HAL_ECDSA_SEC_P256R1;
	case ECDSA_SEC_P384R1:
		return HAL_ECDSA_SEC_P384R1;
	case ECDSA_SEC_P521R1:
		return HAL_ECDSA_SEC_P512R1;
	default: 
		return HAL_ECDSA_UNKNOWN;
	}
	return HAL_ECDSA_UNKNOWN;
}

/* Convert path to slot index of SE */
int _convert_ptos(const char *path, uint32_t *slot)
{
	if (!strncmp(path, SS_PATH, sizeof(SS_PATH))) {
		*slot = atoi(&path[3]);
		return 0;
	}
	// ToDo: check the stored location which is not SE.
	return 0;
}

/**
 * Common
 */
int security_init(void)
{
	SECAPI_ENTER;

	SECAPI_CALL(sl_init(&g_slink_hnd));

	SECURITY_RETURN(SECURITY_OK);
}

int security_deinit(void)
{
	SECAPI_ENTER;

	SECAPI_CALL(sl_deinit(g_slink_hnd));

	SECURITY_RETURN(SECURITY_OK);
}

int security_free_data(security_data *data)
{
	if (data) {
		if (data->data) {
			free(data->data);
		}
		data->data = NULL;
		data->length = 0;
	}
	return SECURITY_OK;
}

int security_get_status(int *status)
{
	//todo
	*status = 0;

	return SECURITY_OK;
}

/**
 * Key Manager
 */
int keymgr_generate_key(security_algorithm algo, const char *key_name)
{
	SECAPI_ENTER;

	// convert the security request to the seclink format.
	hal_key_type htype = _convert_algo_stoh(algo);

	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	SECAPI_CALL(sl_generate_key(g_slink_hnd, htype, key_idx));

	SECURITY_RETURN(SECURITY_OK);
}

int keymgr_set_key(security_algorithm algo, const char *key_name, security_data *pubkey, security_data *prikey)
{
	SECAPI_ENTER;

	//convert key type
	hal_key_type htype = _convert_algo_stoh(algo);

	// convert path
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	// convert key
	hal_data h_pubkey = {pubkey->data, pubkey->length, NULL, 0};
	hal_data h_prikey = {prikey->data, prikey->length, NULL, 0};

	SECAPI_CALL(sl_set_key(g_slink_hnd, htype, key_idx, &h_pubkey, &h_prikey));

	SECURITY_RETURN(SECURITY_OK);
}

int keymgr_get_key(security_algorithm algo, const char *key_name, security_data *pubkey_x, security_data *pubkey_y)
{
	SECAPI_ENTER;

	// ToDo : SEE API needs algo to get key. But it's possible to get key by key_name without algo
	// Therefore instead of setting algo. it's more reasonable to make algo to get key information from SE.
	// it should be discussed later.

	//convert key type
	hal_key_type htype = _convert_algo_stoh(algo);

	// convert path
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	// convert key
	hal_data h_pubkey = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_get_key(g_slink_hnd, htype, key_idx, &h_pubkey));

	pubkey_x->data = (unsigned char *)malloc(h_pubkey.data_len);
	if (!pubkey_x->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(h_pubkey, pubkey_x);

	//get pubkey_y for ECDH
	if (h_pubkey.priv_len > 0) {
		pubkey_y->data = (unsigned char *)malloc(h_pubkey.priv_len);
		if (!pubkey_y->data) {
			SECURITY_RETURN(SECURITY_ALLOC_ERROR);
		}
		SECAPI_H2SD_MEMCPY_PRIV(h_pubkey, pubkey_y);
	}

	SECURITY_RETURN(SECURITY_OK);
}

int keymgr_remove_key(security_algorithm algo, const char *key_name)
{
	SECAPI_ENTER;

	//convert key type
	hal_key_type htype = _convert_algo_stoh(algo);

	// convert path
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	SECAPI_CALL(sl_remove_key(g_slink_hnd, htype, key_idx));

	SECURITY_RETURN(SECURITY_OK);
}

/**
 * Crypto
 */
int crypto_aes_encryption(security_aes_param param, const char *key_name, security_data *input, security_data *output)
{
	SECAPI_ENTER;

	hal_aes_param hparam;
	hparam.mode = _convert_aes_stoh(param.mode);
	hparam.iv = param.iv;
	hparam.iv_len = param.iv_len;

	// convert path
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data dec = {input->data, input->length, NULL, 0};
	hal_data enc = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_aes_encrypt(g_slink_hnd, &dec, &hparam, key_idx, &enc));

	output->data = (unsigned char *)malloc(enc.data_len);
	if (!output->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(enc, output);

	SECURITY_RETURN(SECURITY_OK);
}

int crypto_aes_decryption(security_aes_param param, const char *key_name, security_data *input, security_data *output)
{
	SECAPI_ENTER;

	hal_aes_param hparam;
	hparam.mode = _convert_aes_stoh(param.mode);
	hparam.iv = param.iv;
	hparam.iv_len = param.iv_len;

	// convert path
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data enc = {input->data, input->length, NULL, 0};
	hal_data dec = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_aes_decrypt(g_slink_hnd, &enc, &hparam, key_idx, &dec));

	output->data = (unsigned char *)malloc(dec.data_len);
	if (!output->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(dec, output);

	SECURITY_RETURN(SECURITY_OK);
}

int crypto_rsa_encryption(security_rsa_mode mode, const char *key_name, security_data *input, security_data *output)
{
	SECAPI_ENTER;

	hal_rsa_mode hmode;
	hmode.rsa_a = mode.rsa_a;
	hmode.hash_t = mode.hash_t;
	hmode.mgf = mode.mgf;
	hmode.salt_byte_len = mode.salt_byte_len;

    /* convert path */
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data dec = {input->data, input->length, NULL, 0};
	hal_data enc = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_rsa_encrypt(g_slink_hnd, &dec, &hmode, key_idx, &enc));

	output->data = (unsigned char *)malloc(enc.data_len);
	if (!output->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(enc, output);

	SECURITY_RETURN(SECURITY_OK);
}

int crypto_rsa_decryption(security_rsa_mode mode, const char *key_name, security_data *input, security_data *output)
{
	SECAPI_ENTER;

	hal_rsa_mode hmode;
	hmode.rsa_a = mode.rsa_a;
	hmode.hash_t = mode.hash_t;
	hmode.mgf = mode.mgf;
	hmode.salt_byte_len = mode.salt_byte_len;

	/* convert path */
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data enc = {input->data, input->length, NULL, 0};
	hal_data dec = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_rsa_decrypt(g_slink_hnd, &enc, &hmode, key_idx, &dec));

	output->data = (unsigned char *)malloc(dec.data_len);
	if (!output->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(dec, output);

	SECURITY_RETURN(SECURITY_OK);
}

/**
 * Secure Storage
 */
int ss_read_secure_storage(const char *ss_name, unsigned int offset, security_data *data)
{
	SECAPI_ENTER;

	// ToDo: offset is not supported yet
	if (offset > 0) {
		SECURITY_RETURN(SECURITY_INVALID_INPUT_PARAMS);
	}

    /* convert path */
	uint32_t ss_idx = 0;
	int32_t res = _convert_ptos(ss_name, &ss_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data ss = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_read_storage(g_slink_hnd, ss_idx, &ss));

	data->data = (unsigned char *)malloc(ss.data_len);
	if (!data->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(ss, data);

	SECURITY_RETURN(SECURITY_OK);
}

int ss_write_secure_storage(const char *ss_name, unsigned int offset, security_data *input)
{
	SECAPI_ENTER;

	// ToDo: offset is not supported yet
	if (offset > 0) {
		SECURITY_RETURN(SECURITY_INVALID_INPUT_PARAMS);
	}

    /* convert path */
	uint32_t ss_idx = 0;
	int32_t res = _convert_ptos(ss_name, &ss_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data ss = {input->data, input->length, NULL, 0};

	SECAPI_CALL(sl_write_storage(g_slink_hnd, ss_idx, &ss));

	SECURITY_RETURN(SECURITY_OK);
}

int ss_delete_secure_storage(const char *ss_name)
{
	SECAPI_ENTER;

	/* convert path */
	uint32_t ss_idx = 0;
	int32_t res = _convert_ptos(ss_name, &ss_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	SECAPI_CALL(sl_delete_storage(g_slink_hnd, ss_idx));

	SECURITY_RETURN(SECURITY_OK);
}

int ss_get_size_secure_storage(const char *ss_name, unsigned int *size)
{
	SECAPI_ENTER;

	// ToDo
	*size = 0;

	SECURITY_RETURN(SECURITY_OK);
}
int ss_get_list_secure_storage(unsigned int *count, security_storage_list *list)
{
	SECAPI_ENTER;

	// ToDo
	*count = 0;
	*list = NULL;

	SECURITY_RETURN(SECURITY_OK);
}


/**
 * Authentication
 */
int auth_generate_random(unsigned int size, security_data *random)
{
	SECAPI_ENTER;

	hal_data hrand = {NULL, 0, NULL, 0};
	SECAPI_CALL(sl_generate_random(g_slink_hnd, size, &hrand));

	random->data = (unsigned char *)malloc(hrand.data_len);
	if (!random->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(hrand, random);

	SECURITY_RETURN(SECURITY_OK);
}

int auth_generate_certificate(const char *cert_name, security_csr *csr, security_data *cert)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_set_certificate(const char *cert_name, security_data *cert)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_get_certificate(const char *cert_name, security_data *cert)
{
	SECAPI_ENTER;

    /* convert path */
	uint32_t cert_idx = 0;
	int32_t res = _convert_ptos(cert_name, &cert_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data cert_out = {NULL, 0, NULL, 0};
	SECAPI_CALL(sl_get_certificate(g_slink_hnd, cert_idx, &cert_out));

	cert->data = (unsigned char *)malloc(cert_out.data_len);
	if (!cert->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(cert_out, cert);

	SECURITY_RETURN(SECURITY_OK);
}

int auth_remove_certificate(const char *cert_name)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);

}
int auth_get_rsa_signature(security_rsa_mode mode, const char *key_name, security_data *hash, security_data *sign)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);

}
int auth_verify_rsa_signature(security_rsa_mode mode, const char *key_name, security_data *hash, security_data *sign)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);

}
int auth_get_ecdsa_signature(security_ecdsa_mode mode, const char *key_name, security_data *hash, security_data *sign)
{
	SECAPI_ENTER;

	hal_ecdsa_mode hmode;
	hmode.curve = _convert_curve_stoh(mode.curve);
	hmode.hash_t = mode.hash_t;

	/* convert path */
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data h_hash = {hash->data, hash->length, NULL, 0};
	hal_data h_sign = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_ecdsa_sign_md(g_slink_hnd, hmode, &h_hash, key_idx, &h_sign));

	sign->data = (unsigned char *)malloc(h_sign.data_len);
	if (!sign->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(h_sign, sign);

	SECURITY_RETURN(SECURITY_OK);
}

int auth_verify_ecdsa_signature(security_ecdsa_mode mode, const char *key_name, security_data *hash, security_data *sign)
{
	SECAPI_ENTER;

	hal_ecdsa_mode hmode;
	hmode.curve = _convert_curve_stoh(mode.curve);
	hmode.hash_t = mode.hash_t;

	/* convert path */
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data h_hash = {hash->data, hash->length, NULL, 0};
	hal_data h_sign = {sign->data, sign->length, NULL, 0};

	SECAPI_CALL(sl_ecdsa_verify_md(g_slink_hnd, hmode, &h_hash, &h_sign, key_idx));

	SECURITY_RETURN(SECURITY_OK);
}

int auth_get_hash(security_hash_type algo, security_data *data, security_data *hash)
{
	SECAPI_ENTER;

	hal_hash_type htype = _convert_hash_stoh(algo);
	if (htype == HAL_HASH_UNKNOWN) {
		SECURITY_RETURN(SECURITY_INVALID_INPUT_PARAMS);
	}

	hal_data input = {data->data, data->length, NULL, 0};
	hal_data output = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_get_hash(g_slink_hnd, htype, &input, &output));

	hash->data = (unsigned char *)malloc(output.data_len);
	if (!hash->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(output, hash);

	SECURITY_RETURN(SECURITY_OK);
}

int auth_get_hmac(security_algorithm algo, const char *key_name, security_data *data, security_data *hmac)
{
	SECAPI_ENTER;

	hal_hmac_type htype = _convert_hmac_stoh(algo);
	if (htype == HAL_HMAC_UNKNOWN) {
		SECURITY_RETURN(SECURITY_INVALID_INPUT_PARAMS);
	}

	/* convert path */
	uint32_t key_idx = 0;
	int32_t res = _convert_ptos(key_name, &key_idx);
	if (res < 0) {
		// it doesn't point SE slot that will generate key. so return err.r
		SECURITY_RETURN(SECURITY_INVALID_KEY_INDEX);
	}

	hal_data input  = {data->data, data->length, NULL, 0};
	hal_data output = {NULL, 0, NULL, 0};

	SECAPI_CALL(sl_get_hmac(g_slink_hnd, htype, &input, key_idx, &output));

	hmac->data = (unsigned char *)malloc(output.data_len);
	if (!hmac->data) {
		SECURITY_RETURN(SECURITY_ALLOC_ERROR);
	}
	SECAPI_H2SD_MEMCPY(output, hmac);

	SECURITY_RETURN(SECURITY_OK);
}

int auth_generate_dhparams(security_dh_data *params, security_data *pub)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_set_dhparams(security_dh_data *params, security_data *pub)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_compute_dhparams(security_dh_data *params, security_data *secret)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_generate_ecdhkey(security_ecdh_data *params, security_data *pub)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}

int auth_compute_ecdhkey(security_ecdh_data *params, security_data *secret)
{
	SECAPI_ENTER;

	SECURITY_RETURN(SECURITY_NOT_SUPPORT);
}
