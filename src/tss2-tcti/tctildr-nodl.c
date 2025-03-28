/* SPDX-License-Identifier: BSD-2-Clause */
/*******************************************************************************
 * Copyright 2017-2018, Fraunhofer SIT sponsored by Infineon Technologies AG
 * All rights reserved.
 * Copyright 2019, Intel Corporation
 * Copyright (c) 2019, Wind River Systems.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"            // for TCTI_DEVICE, TCTI_MSSIM, TCTI_SWTPM
#endif

#include <stdlib.h>            // for NULL, size_t
#include <string.h>            // for strcmp

#include "tctildr.h"           // for tcti_from_init
#include "tss2_common.h"       // for TSS2_RC, TSS2_RC_SUCCESS, TSS2_TCTI_RC...
#include "tss2_tcti.h"         // for TSS2_TCTI_CONTEXT, TSS2_TCTI_INFO, TSS...
#include "tss2_tcti_mssim.h"   // for Tss2_Tcti_Mssim_Init
#include "tss2_tcti_swtpm.h"   // for Tss2_Tcti_Swtpm_Init
#include "util/aux_util.h"     // for UNUSED
#ifdef _WIN32
#include "tss2_tcti_tbs.h"
#else /* _WIN32 */
#include "tss2_tcti_device.h"  // for Tss2_Tcti_Device_Init
#endif
#define LOGMODULE tcti
#include "util/log.h"          // for LOG_ERROR, LOG_DEBUG

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof((X)[0]))
#endif
#define NAME_ARRAY_SIZE 3

struct {
    const char *names [NAME_ARRAY_SIZE];
    TSS2_TCTI_INIT_FUNC init;
    char *conf;
    char *description;
} tctis [] = {
#ifdef _WIN32
    {
        .names = {
            "libtss2-tcti-tbs.so.0",
            "libtss2-tcti-tbs.so",
            "tbs",
        },
        .init = Tss2_Tcti_Tbs_Init,
        .description = "Access to TBS",
    },
#elif defined (__VXWORKS__)
    {
        .names = {
            "libtss2-tcti-device.so.0",
            "libtss2-tcti-device.so",
            "device",
        },
        .init = Tss2_Tcti_Device_Init,
        .conf = "/tpm0",
        .description = "Access to /tpm0",
    },
#else /* _WIN32 */
#ifdef TCTI_DEVICE
    {
        .names = {
            "libtss2-tcti-device.so.0",
            "libtss2-tcti-device.so",
            "device",
        },
        .init = Tss2_Tcti_Device_Init,
        .conf = "/dev/tpmrm0",
        .description = "Access to /dev/tpmrm0",
    },
    {
        .names = {
            "libtss2-tcti-device.so.0",
            "libtss2-tcti-device.so",
            "device",
        },
        .init = Tss2_Tcti_Device_Init,
        .conf = "/dev/tpm0",
        .description = "Access to /dev/tpm0",
    },
    {
        .names = {
            "libtss2-tcti-device.so.0",
            "libtss2-tcti-device.so",
            "device",
        },
        .init = Tss2_Tcti_Device_Init,
        .conf = "/dev/tcm0",
        .description = "Access to /dev/tcm0",
    },
#endif /* TCTI_DEVICE */
#endif /* _WIN32 */
#ifdef TCTI_SWTPM
    {
        .names = {
            "libtss2-tcti-swtpm.so.0",
            "libtss2-tcti-swtpm.so",
            "swtpm",
        },
        .init = Tss2_Tcti_Swtpm_Init,
        .description = "Access to TPM software simulator, default conf",
    },
#endif /* TCTI_SWTPM */
#ifdef TCTI_MSSIM
    {
        .names = {
            "libtss2-tcti-mssim.so.0",
            "libtss2-tcti-mssim.so",
            "mssim",
        },
        .init = Tss2_Tcti_Mssim_Init,
        .description = "Access to simulator using MS protocol, default conf",
    },
#endif /* TCTI_MSSIM */
};
TSS2_RC
tctildr_get_default(TSS2_TCTI_CONTEXT ** tcticontext, void **dlhandle)
{
    TSS2_RC rc;

    UNUSED(dlhandle);
    if (tcticontext == NULL) {
        LOG_ERROR("tcticontext must not be NULL");
        return TSS2_TCTI_RC_BAD_REFERENCE;
    }
    *tcticontext = NULL;

    if (ARRAY_SIZE(tctis) == 0) {
        LOG_ERROR("No default TCTIs configured during compilation");
        return TSS2_TCTI_RC_IO_ERROR;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    for (size_t i = 0; i < ARRAY_SIZE(tctis); i++) {
#pragma GCC diagnostic pop
        LOG_DEBUG("Attempting to connect using standard TCTI: %s",
                  tctis[i].description);
        rc = tcti_from_init (tctis[i].init,
                             tctis[i].conf,
                             tcticontext);
        if (rc == TSS2_RC_SUCCESS)
            return TSS2_RC_SUCCESS;
        LOG_DEBUG("Failed to load standard TCTI number %zu", i);
    }

    LOG_ERROR("No standard TCTI could be loaded");
    return TSS2_TCTI_RC_IO_ERROR;
}
TSS2_RC
tctildr_get_tcti (const char *name,
                  const char* conf,
                  TSS2_TCTI_CONTEXT **tcti,
                  void **data)
{
    TSS2_RC rc;

    if (tcti == NULL) {
        LOG_ERROR("tcticontext must not be NULL");
        return TSS2_TCTI_RC_BAD_REFERENCE;
    }
    *tcti = NULL;
    if (name == NULL) {
        return tctildr_get_default (tcti, data);
    }

    if (ARRAY_SIZE(tctis) == 0) {
        LOG_ERROR("No default TCTIs configured during compilation");
        return TSS2_TCTI_RC_IO_ERROR;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    for (size_t i = 0; i < ARRAY_SIZE(tctis); ++i) {
#pragma GCC diagnostic pop
        for (size_t j = 0; j < NAME_ARRAY_SIZE; ++j) {
            if (strcmp (name, tctis[i].names[j]) != 0)
                continue;
            LOG_DEBUG("initializing TCTI with name \"%s\"",
                      tctis[i].names[j]);
            rc = tcti_from_init (tctis[i].init, conf, tcti);
            if (rc == TSS2_RC_SUCCESS)
                return TSS2_RC_SUCCESS;
        }
    }
    LOG_ERROR("Unable to initialize TCTI with name: \"%s\"", name);
    return TSS2_TCTI_RC_IO_ERROR;
}

void
tctildr_finalize_data(void **data)
{
    UNUSED(data);
    return;
}

TSS2_RC
tctildr_get_info (const char *name,
                  const TSS2_TCTI_INFO **info,
                  void **data)
{
    UNUSED(name);
    UNUSED(info);
    UNUSED(data);
    return TSS2_TCTI_RC_NOT_SUPPORTED;
}
