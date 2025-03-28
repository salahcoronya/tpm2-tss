/* SPDX-License-Identifier: BSD-2-Clause */
/*******************************************************************************
 * Copyright 2017-2018, Fraunhofer SIT sponsored by Infineon Technologies AG
 * All rights reserved.
 ******************************************************************************/
#ifndef ESYS_IUTIL_H
#define ESYS_IUTIL_H

#include <inttypes.h>         // for uint8_t
#include <stdbool.h>          // for bool
#include <string.h>           // for size_t

#include "esys_int.h"         // for RSRC_NODE_T
#include "tss2_common.h"      // for TSS2_RC
#include "tss2_esys.h"        // for ESYS_CONTEXT, ESYS_TR, ESYS_CRYPTO_CALL...
#include "tss2_sys.h"         // for TSS2L_SYS_AUTH_COMMAND, TSS2L_SYS_AUTH_...
#include "tss2_tpm2_types.h"  // for TPM2B_NAME, TPM2B_AUTH, TPM2_RH_LAST

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Start issuing ESYS_TR objects past the TPM2_RH_LAST namespace
 * and give ourselves 0x1000 handle space in case of differing
 * header files between the library build and the client build.
 *
 * Due to an API mistake, TPM2_RH constants are valid for a few
 * select ESYS API calls.
 *
 * More details can be found here:
 *   - https://github.com/tpm2-software/tpm2-tss/issues/1750
 */
#define ESYS_TR_MIN_OBJECT (TPM2_RH_LAST + 1 + 0x1000)

TSS2_RC init_session_tab(
    ESYS_CONTEXT *esysContext,
    ESYS_TR shandle1, ESYS_TR shandle2, ESYS_TR shandle3);

void iesys_DeleteAllResourceObjects(
    ESYS_CONTEXT *esys_context);

TSS2_RC iesys_compute_encrypt_nonce(
    ESYS_CONTEXT *esysContext,
    int *encryptNonceIdx,
    TPM2B_NONCE **encryptNonce);

TSS2_RC esys_CreateResourceObject(
    ESYS_CONTEXT *esys_context,
    ESYS_TR esys_handle,
    RSRC_NODE_T **node);

TSS2_RC iesys_handle_to_tpm_handle(
    ESYS_TR esys_handle,
    TPM2_HANDLE *tpm_handle);

bool
iesys_is_platform_handle(
        ESYS_TR handle);

TSS2_RC esys_GetResourceObject(
    ESYS_CONTEXT *esys_context,
    ESYS_TR rsrc_handle,
    RSRC_NODE_T **node);

TPM2_HT iesys_get_handle_type(
    TPM2_HANDLE handle);

TSS2_RC iesys_finalize(ESYS_CONTEXT *context);

bool iesys_compare_name(
    ESYS_CRYPTO_CALLBACKS *crypto_cb,
    TPM2B_PUBLIC *publicInfo,
    TPM2B_NAME *name);

TSS2_RC iesys_compute_encrypted_salt(
    ESYS_CONTEXT *esysContext,
    RSRC_NODE_T *tpmKeyNode,
    TPM2B_ENCRYPTED_SECRET *encryptedSalt);

TSS2_RC iesys_gen_caller_nonces(
    ESYS_CONTEXT *esysContext);

TSS2_RC iesys_encrypt_param(
    ESYS_CONTEXT *esysContext,
    TPM2B_NONCE **decryptNonce,
    int *decryptNonceIdx);

TSS2_RC iesys_decrypt_param(
    ESYS_CONTEXT *esysContext);

TSS2_RC iesys_check_rp_hmacs(
    ESYS_CONTEXT *esysContext,
    TSS2L_SYS_AUTH_RESPONSE *rspAuths);

void iesys_compute_bound_entity(
    const TPM2B_NAME *name,
    const TPM2B_AUTH *auth,
    TPM2B_NAME *bound_entity);

bool iesys_is_object_bound(
    const TPM2B_NAME * name,
    const TPM2B_AUTH * auth,
    RSRC_NODE_T * session);

TSS2_RC iesys_check_sequence_async(
    ESYS_CONTEXT *esysContext);

TSS2_RC check_session_feasibility(
    ESYS_TR shandle1,
    ESYS_TR shandle2,
    ESYS_TR shandle3,
    int mandatory);

void iesys_compute_session_value(
    RSRC_NODE_T *session,
    const TPM2B_NAME *name,
    const TPM2B_AUTH *auth_value);

TSS2_RC iesys_compute_hmac(
    ESYS_CONTEXT *esys_context,
    RSRC_NODE_T *session,
    TPM2B_NONCE *decryptNonce,
    TPM2B_NONCE *encryptNonce,
    TPMS_AUTH_COMMAND *auth);

TSS2_RC iesys_gen_auths(
    ESYS_CONTEXT *esysContext,
    RSRC_NODE_T *h1,
    RSRC_NODE_T *h2,
    RSRC_NODE_T *h3,
    TSS2L_SYS_AUTH_COMMAND *auths);

TSS2_RC iesys_check_response(
    ESYS_CONTEXT * esys_context);

TSS2_RC iesys_nv_get_name(
    ESYS_CRYPTO_CALLBACKS *crypto_cb,
    TPM2B_NV_PUBLIC *publicInfo,
    TPM2B_NAME *name);

TSS2_RC iesys_get_name(
    ESYS_CRYPTO_CALLBACKS *crypto_cb,
    TPM2B_PUBLIC *publicInfo,
    TPM2B_NAME *name);

bool iesys_tpm_error(
    TSS2_RC r);

TSS2_RC iesys_adapt_auth_value(
    ESYS_CRYPTO_CALLBACKS *crypto_cb,
    TPM2B_AUTH *auth_value,
    TPMI_ALG_HASH hash_alg);

void iesys_strip_trailing_zeros(
    TPM2B_AUTH *auth_value);

TSS2_RC iesys_compute_cp_hash(
    ESYS_CONTEXT *esys_context,
    TPMI_ALG_HASH hash_alg,
    uint8_t *cp_hash,
    size_t *cp_hash_size);

 TSS2_RC iesys_compute_rp_hash(
    ESYS_CONTEXT *esys_context,
    TPMI_ALG_HASH hash_alg,
    uint8_t *rp_hash,
    size_t *rp_hash_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ESYS_IUTIL_H */
