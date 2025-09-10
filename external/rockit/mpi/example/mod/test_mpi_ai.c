/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "rk_defines.h"
#include "rk_debug.h"
#include "rk_mpi_ai.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"
#include "rk_osal.h"

#ifndef OS_RTT
#include <sys/poll.h>
#else
#include <sys/select.h>
#include <poll.h>
#endif

#include "test_comm_argparse.h"

static RK_BOOL gAiExit = RK_FALSE;
#define TEST_AI_WITH_FD 0
#define AI_ALGO_FRAMES 256  // baed on 16kHz, it's  128 during 8kHz

typedef struct _rkMpiAICtx {
    const char *srcFilePath;
    const char *dstFilePath;
    RK_S32      s32LoopCount;
    RK_S32      s32ChnNum;
    RK_S32      s32DeviceSampleRate;
    RK_S32      s32SampleRate;
    RK_S32      s32DeviceChannel;
    RK_S32      s32Channel;
    RK_S32      s32BitWidth;
    RK_S32      s32DevId;
    RK_S32      s32FrameNumber;
    RK_S32      s32FrameLength;
    char       *chCardName;
    RK_S32      s32ChnIndex;
    RK_S32      s32SetVolumeCurve;
    RK_S32      s32SetVolume;
    RK_S32      s32SetMute;
    RK_S32      s32SetFadeRate;
    RK_S32      s32SetTrackMode;
    RK_S32      s32GetVolume;
    RK_S32      s32GetMute;
    RK_S32      s32GetTrackMode;
    RK_S32      s32LoopbackMode;
    RK_S32      s32DevFd;
    RK_S32      s32DataReadEnable;
    RK_S32      s32AedEnable;
    RK_S32      s32BcdEnable;
    RK_S32      s32BuzEnable;
    RK_S32      s32GbsEnable;
    RK_S32      s32AedLoudCount;
    RK_S32      s32BcdCount;
    RK_S32      s32BuzCount;
    RK_S32      s32GbsCount;
    RK_S32      s32VqeGapMs;
    RK_S32      s32VqeEnable;
    RK_S32      s32VqeResult;
    RK_S32      s32DumpAlgo;
    const char *pBcdModelPath;
    const char *pVqeCfgPath;
    RK_S32      s32DevQueLen;
    RK_S32      s32SedQueLen;
    RK_S32      s32FdGetEnable;
    RK_S32      s32QuitCnt;
} TEST_AI_CTX_S;

static void sigterm_handler(int sig) {
    RK_PRINT("Catched SIGINT %d\n", sig);
    gAiExit = RK_TRUE;
}

static AUDIO_SOUND_MODE_E ai_find_sound_mode(RK_S32 ch) {
    AUDIO_SOUND_MODE_E channel = AUDIO_SOUND_MODE_BUTT;
    switch (ch) {
      case 1:
        channel = AUDIO_SOUND_MODE_MONO;
        break;
      case 2:
        channel = AUDIO_SOUND_MODE_STEREO;
        break;
      case 4:
        channel = AUDIO_SOUND_MODE_4_CHN;
        break;
      case 6:
        channel = AUDIO_SOUND_MODE_6_CHN;
        break;
      case 8:
        channel = AUDIO_SOUND_MODE_8_CHN;
        break;
      default:
        RK_LOGE("channel = %d not support", ch);
        return AUDIO_SOUND_MODE_BUTT;
    }

    return channel;
}

static AUDIO_BIT_WIDTH_E ai_find_bit_width(RK_S32 bit) {
    AUDIO_BIT_WIDTH_E bitWidth = AUDIO_BIT_WIDTH_BUTT;
    switch (bit) {
      case 8:
        bitWidth = AUDIO_BIT_WIDTH_8;
        break;
      case 16:
        bitWidth = AUDIO_BIT_WIDTH_16;
        break;
      case 32:
        bitWidth = AUDIO_BIT_WIDTH_32;
        break;
      default:
        RK_LOGE("bitwidth(%d) not support", bit);
        return AUDIO_BIT_WIDTH_BUTT;
    }

    return bitWidth;
}

RK_S32 test_ai_poll_event(RK_S32 timeoutMsec, RK_S32 fd) {
    RK_S32 num_fds = 1;
    struct pollfd pollFds[num_fds];
    RK_S32 ret = 0;

    RK_ASSERT(fd > 0);
    memset(pollFds, 0, sizeof(pollFds));
    pollFds[0].fd = fd;
    pollFds[0].events = (POLLPRI | POLLIN | POLLERR | POLLNVAL | POLLHUP);

    ret = poll(pollFds, num_fds, timeoutMsec);

    if (ret > 0 && (pollFds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
        RK_LOGE("fd:%d polled error", fd);
        return -1;
    }

    return ret;
}

RK_S32 test_open_device_ai(TEST_AI_CTX_S *ctx) {
    AUDIO_DEV aiDevId = ctx->s32DevId;
    AUDIO_SOUND_MODE_E soundMode;

    AIO_ATTR_S aiAttr;
    RK_S32 result;
    RK_S32 chnNum = 0;
    memset(&aiAttr, 0, sizeof(AIO_ATTR_S));

    if (ctx->chCardName) {
        snprintf((char *)(aiAttr.u8CardName),
                 sizeof(aiAttr.u8CardName), "%s", ctx->chCardName);
    }

    aiAttr.soundCard.channels = ctx->s32DeviceChannel;
    aiAttr.soundCard.sampleRate = ctx->s32DeviceSampleRate;
    AUDIO_BIT_WIDTH_E bitWidth = ai_find_bit_width(ctx->s32BitWidth);
    if (bitWidth == AUDIO_BIT_WIDTH_BUTT) {
        goto __FAILED;
    }


    if (ctx->s32ChnNum > AI_MAX_CHN_NUM) {
        RK_LOGE("error: s32ChnNum(%d) > max chn count(%d)",
                 ctx->s32ChnNum, AI_MAX_CHN_NUM);
        goto __FAILED;
    }

    aiAttr.soundCard.bitWidth = bitWidth;
    aiAttr.enBitwidth = bitWidth;
    aiAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)ctx->s32SampleRate;
    soundMode = ai_find_sound_mode(ctx->s32Channel);
    if (soundMode == AUDIO_SOUND_MODE_BUTT) {
        goto __FAILED;
    }
    aiAttr.enSoundmode = soundMode;
    aiAttr.u32FrmNum = ctx->s32FrameNumber;
    aiAttr.u32PtNumPerFrm = ctx->s32FrameLength;
    for (int i = 0; i < ctx->s32ChnNum; i++) {
        aiAttr.u8MapOutChns[i] = ctx->s32DeviceChannel;
        for (int j = 0; j < ctx->s32DeviceChannel; j++)
            aiAttr.u8MapChns[i][j] = j;
    }

    if (ctx->s32ChnNum == 1) {
        for (int i = 0; i < AI_MAX_CHN_NUM; i++) {
            if (aiAttr.u8MapOutChns[i] > 0 &&
                aiAttr.u8MapOutChns[i] == ctx->s32DeviceChannel) {
                for (int j = 0; j < ctx->s32DeviceChannel; j++) {
                    if (aiAttr.u8MapChns[i][j] != j) {
                        chnNum = 0;
                        break;
                    }

                    if (j == ctx->s32DeviceChannel - 1)
                        chnNum++;
                }
            }
        }

        if (chnNum > 1)
            aiAttr.bMultichnFlag = RK_TRUE;
    }

    if (!ctx->s32DataReadEnable)
        aiAttr.u32EXFlag = 0;
    else
        aiAttr.u32EXFlag = 1;

    aiAttr.u32ChnCnt = 2;
    aiAttr.s32DevQueLen = ctx->s32DevQueLen;

    result = RK_MPI_AI_SetPubAttr(aiDevId, &aiAttr);
    if (result != 0) {
        RK_LOGE("ai set attr fail, reason = %X", result);
        goto __FAILED;
    }

    result = RK_MPI_AI_Enable(aiDevId);
    if (result != 0) {
        RK_LOGE("ai enable fail, reason = %X", result);
        goto __FAILED;
    }

    return RK_SUCCESS;
__FAILED:
    result =  RK_MPI_AI_Disable(aiDevId);
    if (result != 0) {
        RK_LOGE("ai disable fail, reason = %X", result);
        return RK_FAILURE;
    }

    return RK_FAILURE;
}

RK_S32 test_init_ai_data_read(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32DataReadEnable) {
        result = RK_MPI_AI_EnableDataRead(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_EnableDataRead(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

RK_S32 test_init_ai_aed(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32AedEnable) {
        AI_AED_CONFIG_S stAiAedConfig, stAiAedConfig2;
        memset(&stAiAedConfig, 0, sizeof(AI_AED_CONFIG_S));
        memset(&stAiAedConfig2, 0, sizeof(AI_AED_CONFIG_S));

        stAiAedConfig.fSnrDB = 10.0f;
        stAiAedConfig.fLsdDB = -35.0f;
        stAiAedConfig.s32Policy = 1;
        stAiAedConfig.fSmoothParam = 0.9;

        result = RK_MPI_AI_SetAedAttr(params->s32DevId, params->s32ChnIndex, &stAiAedConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetAedAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetAedAttr(params->s32DevId, params->s32ChnIndex, &stAiAedConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetAedAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiAedConfig, &stAiAedConfig2, sizeof(AI_AED_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableAed(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableAed(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_aed2(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32AedEnable) {
        AI_AED_CONFIG_S stAiAedConfig, stAiAedConfig2;
        memset(&stAiAedConfig, 0, sizeof(AI_AED_CONFIG_S));
        memset(&stAiAedConfig2, 0, sizeof(AI_AED_CONFIG_S));

        stAiAedConfig.fSnrDB = 10.0f;
        stAiAedConfig.fLsdDB = -35.0f;
        stAiAedConfig.s32Policy = 2;
        stAiAedConfig.fSmoothParam = 0.9;

        result = RK_MPI_AI_SetAedAttr(params->s32DevId, params->s32ChnIndex, &stAiAedConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetAedAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetAedAttr(params->s32DevId, params->s32ChnIndex, &stAiAedConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetAedAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiAedConfig, &stAiAedConfig2, sizeof(AI_AED_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableAed(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableAed(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_bcd(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32BcdEnable) {
        AI_BCD_CONFIG_S stAiBcdConfig, stAiBcdConfig2;
        memset(&stAiBcdConfig, 0, sizeof(AI_BCD_CONFIG_S));
        memset(&stAiBcdConfig2, 0, sizeof(AI_BCD_CONFIG_S));

        stAiBcdConfig.mFrameLen = 60;
        stAiBcdConfig.mConfirmProb = 0.85f;
        switch (params->s32DeviceChannel) {
        case 4:
            // just for example: 2mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x03;
            break;
        case 6:
            // just for example: 4mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x0f;
            break;
        case 8:
            // just for example: 6mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x3f;
            break;
        default:
            // by default is 1mic + 1ref, it will be set by internal if is not specified.
            // stAiBcdConfig.stSedCfg.s64RecChannelType = 0x01;
            break;
        }
        // stAiBcdConfig.stSedCfg.s32FrameLen = 90; // by default is 90 if is not specified.
        if (stAiBcdConfig.stSedCfg.s64RecChannelType != 0 ||
            stAiBcdConfig.stSedCfg.s32FrameLen != 0)
            stAiBcdConfig.stSedCfg.bUsed = RK_TRUE;
        if (params->pBcdModelPath != RK_NULL)
            memcpy(stAiBcdConfig.aModelPath, params->pBcdModelPath, strlen(params->pBcdModelPath));

        result = RK_MPI_AI_SetBcdAttr(params->s32DevId, params->s32ChnIndex, &stAiBcdConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBcdAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetBcdAttr(params->s32DevId, params->s32ChnIndex, &stAiBcdConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBcdAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiBcdConfig, &stAiBcdConfig2, sizeof(AI_BCD_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableBcd(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableBcd(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_bcd2(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32BcdEnable) {
        AI_BCD_CONFIG_S stAiBcdConfig, stAiBcdConfig2;
        memset(&stAiBcdConfig, 0, sizeof(AI_BCD_CONFIG_S));
        memset(&stAiBcdConfig2, 0, sizeof(AI_BCD_CONFIG_S));

        stAiBcdConfig.mFrameLen = 40;
        stAiBcdConfig.mConfirmProb = 0.85f;
        switch (params->s32DeviceChannel) {
        case 4:
            // just for example: 2mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x03;
            break;
        case 6:
            // just for example: 4mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x0f;
            break;
        case 8:
            // just for example: 6mic + 2ref
            stAiBcdConfig.stSedCfg.s64RecChannelType = 0x3f;
            break;
        default:
            // by default is 1mic + 1ref, it will be set by internal if is not specified.
            // stAiBcdConfig.stSedCfg.s64RecChannelType = 0x01;
            break;
        }
        stAiBcdConfig.stSedCfg.s32FrameLen = 80;
        if (stAiBcdConfig.stSedCfg.s64RecChannelType != 0 ||
            stAiBcdConfig.stSedCfg.s32FrameLen != 0)
            stAiBcdConfig.stSedCfg.bUsed = RK_TRUE;
        if (params->pBcdModelPath != RK_NULL)
            memcpy(stAiBcdConfig.aModelPath, params->pBcdModelPath, strlen(params->pBcdModelPath));

        result = RK_MPI_AI_SetBcdAttr(params->s32DevId, params->s32ChnIndex, &stAiBcdConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBcdAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetBcdAttr(params->s32DevId, params->s32ChnIndex, &stAiBcdConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBcdAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiBcdConfig, &stAiBcdConfig2, sizeof(AI_BCD_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableBcd(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableBcd(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_buz(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32BuzEnable) {
        AI_BUZ_CONFIG_S stAiBuzConfig, stAiBuzConfig2;
        memset(&stAiBuzConfig, 0, sizeof(AI_BUZ_CONFIG_S));
        memset(&stAiBuzConfig2, 0, sizeof(AI_BUZ_CONFIG_S));

        stAiBuzConfig.mFrameLen = 100;
        stAiBuzConfig.mConfirmProb = 0.5;

        result = RK_MPI_AI_SetBuzAttr(params->s32DevId, params->s32ChnIndex, &stAiBuzConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBuzAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetBuzAttr(params->s32DevId, params->s32ChnIndex, &stAiBuzConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBuzAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiBuzConfig, &stAiBuzConfig2, sizeof(AI_BUZ_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableBuz(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableBuz(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_buz2(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32BuzEnable) {
        AI_BUZ_CONFIG_S stAiBuzConfig, stAiBuzConfig2;
        memset(&stAiBuzConfig, 0, sizeof(AI_BUZ_CONFIG_S));
        memset(&stAiBuzConfig2, 0, sizeof(AI_BUZ_CONFIG_S));

        stAiBuzConfig.mFrameLen = 100;
        stAiBuzConfig.mConfirmProb = 0.5;

        result = RK_MPI_AI_SetBuzAttr(params->s32DevId, params->s32ChnIndex, &stAiBuzConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBuzAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetBuzAttr(params->s32DevId, params->s32ChnIndex, &stAiBuzConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetBuzAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiBuzConfig, &stAiBuzConfig2, sizeof(AI_BUZ_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableBuz(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableBuz(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_gbs(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32GbsEnable) {
        AI_GBS_CONFIG_S stAiGbsConfig, stAiGbsConfig2;
        memset(&stAiGbsConfig, 0, sizeof(AI_GBS_CONFIG_S));
        memset(&stAiGbsConfig2, 0, sizeof(AI_GBS_CONFIG_S));

        stAiGbsConfig.mFrameLen = 30;
        stAiGbsConfig.mConfirmProb = 0.8;

        result = RK_MPI_AI_SetGbsAttr(params->s32DevId, params->s32ChnIndex, &stAiGbsConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetGbsAttr(params->s32DevId, params->s32ChnIndex, &stAiGbsConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiGbsConfig, &stAiGbsConfig2, sizeof(AI_GBS_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableGbs(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableGbs(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_gbs2(TEST_AI_CTX_S *params) {
    RK_S32 result;

    if (params->s32GbsEnable) {
        AI_GBS_CONFIG_S stAiGbsConfig, stAiGbsConfig2;
        memset(&stAiGbsConfig, 0, sizeof(AI_GBS_CONFIG_S));
        memset(&stAiGbsConfig2, 0, sizeof(AI_GBS_CONFIG_S));

        stAiGbsConfig.mFrameLen = 30;
        stAiGbsConfig.mConfirmProb = 0.8;

        result = RK_MPI_AI_SetGbsAttr(params->s32DevId, params->s32ChnIndex, &stAiGbsConfig);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_GetGbsAttr(params->s32DevId, params->s32ChnIndex, &stAiGbsConfig2);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = memcmp(&stAiGbsConfig, &stAiGbsConfig2, sizeof(AI_GBS_CONFIG_S));
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: set/get aed config is different: %d", __FUNCTION__, result);
            return result;
        }

        result = RK_MPI_AI_EnableGbs(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: EnableGbs(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_sed_agc(TEST_AI_CTX_S *params) {
    RK_S32 result;
    AUDIO_AGC_CONFIG_S stSedAgcConfig;
    memset(&stSedAgcConfig, 0, sizeof(AUDIO_AGC_CONFIG_S));

    stSedAgcConfig.fAttackTime = 200.0;
    stSedAgcConfig.fReleaseTime = 400.0;
    stSedAgcConfig.fMaxGain = 30;
    stSedAgcConfig.fMaxPeak = -3.0;
    stSedAgcConfig.fRk0 = 2;
    stSedAgcConfig.fRth0 = -70;
    stSedAgcConfig.fRth1 = -45;
    stSedAgcConfig.fRth2 = -40;

    stSedAgcConfig.fAttenuateTime = 1000;
    stSedAgcConfig.fRk1 = 0.8;
    stSedAgcConfig.fRk2 = 0.4;
    stSedAgcConfig.fLineGainDb = -25.0f;
    stSedAgcConfig.s32SwSmL0 = 40;
    stSedAgcConfig.s32SwSmL1 = 80;
    stSedAgcConfig.s32SwSmL2 = 80;

    result = RK_MPI_AI_SetSedAgcAttr(params->s32DevId, params->s32ChnIndex, &stSedAgcConfig);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    result = RK_MPI_AI_GetSedAgcAttr(params->s32DevId, params->s32ChnIndex, &stSedAgcConfig);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: SetGbsAttr(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    result = RK_MPI_AI_EnableSedAgc(params->s32DevId, params->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: EnableSedAgc(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    return RK_SUCCESS;
}

static RK_S32 test_init_ai_sed_fir(TEST_AI_CTX_S *params) {
    RK_S32 result;
    result = RK_MPI_AI_EnableSedFir(params->s32DevId, params->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: EnableSedFir(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    return RK_SUCCESS;
}

RK_S32 test_init_ai_vqe(TEST_AI_CTX_S *params) {
    AI_VQE_CONFIG_S stAiVqeConfig, stAiVqeConfig2;
    AI_VQE_MOD_ENABLE_S stAiVqeModEnable;
    RK_S32 result;

    if (params->s32VqeEnable == 0)
        return RK_SUCCESS;

    if (params->s32VqeEnable == 2) {
        //When it is necessary to configure module switches separately, this interface needs to be called.
        //By default, it can be ignored.
        memset(&stAiVqeModEnable, 0, sizeof(AI_VQE_MOD_ENABLE_S));
        stAiVqeModEnable.bAec = RK_TRUE;
        stAiVqeModEnable.bBf = RK_TRUE;
        stAiVqeModEnable.bFastAec = RK_TRUE;
        stAiVqeModEnable.bAes = RK_TRUE;
        stAiVqeModEnable.bGsc = RK_TRUE;
        stAiVqeModEnable.bAgc = RK_TRUE;
        stAiVqeModEnable.bAnr = RK_TRUE;
        stAiVqeModEnable.bNlp = RK_FALSE; //not support
        stAiVqeModEnable.bDereverb = RK_TRUE;
        stAiVqeModEnable.bCng = RK_TRUE;
        stAiVqeModEnable.bDtd = RK_TRUE;
        stAiVqeModEnable.bHowling = RK_TRUE;
        stAiVqeModEnable.bDoa = RK_TRUE;
        stAiVqeModEnable.bWakeup = RK_FALSE;

        //When it is necessary to configure module switches separately, this interface needs to be called.
        //By default, it can be ignored.
        result = RK_MPI_AI_SetVqeModuleEnable(params->s32DevId, params->s32ChnIndex, &stAiVqeModEnable);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_SetVqeModuleEnable(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    // Need to config enCfgMode to VQE attr even the VQE is not enabled
    memset(&stAiVqeConfig, 0, sizeof(AI_VQE_CONFIG_S));
    if (params->pVqeCfgPath != RK_NULL) {
        stAiVqeConfig.enCfgMode = AIO_VQE_CONFIG_LOAD_FILE;
        memcpy(stAiVqeConfig.aCfgFile, params->pVqeCfgPath, strlen(params->pVqeCfgPath));
    }

    if (params->s32VqeGapMs != 16 && params->s32VqeGapMs != 10) {
        RK_LOGE("Invalid gap: %d, just supports 16ms or 10ms for AI VQE", params->s32VqeGapMs);
        return RK_FAILURE;
    }

    stAiVqeConfig.s32WorkSampleRate = params->s32SampleRate;
    stAiVqeConfig.s32FrameSample = params->s32SampleRate * params->s32VqeGapMs / 1000;
    stAiVqeConfig.s64RefChannelType = 2;
    stAiVqeConfig.s64RecChannelType = 1;
    for (int i = 0; i < params->s32DeviceChannel; i++)
        stAiVqeConfig.s64ChannelLayoutType |= (1 << i);

    result = RK_MPI_AI_SetVqeAttr(params->s32DevId, params->s32ChnIndex, 0, 0, &stAiVqeConfig);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: SetVqeAttr(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    result = RK_MPI_AI_GetVqeAttr(params->s32DevId, params->s32ChnIndex, &stAiVqeConfig2);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: SetVqeAttr(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    result = memcmp(&stAiVqeConfig, &stAiVqeConfig2, sizeof(AI_VQE_CONFIG_S));
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: set/get vqe config is different: %d", __FUNCTION__, result);
        return result;
    }

    result = RK_MPI_AI_EnableVqe(params->s32DevId, params->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("%s: EnableVqe(%d,%d) failed with %#x",
            __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
        return result;
    }

    return RK_SUCCESS;
}

RK_S32 test_init_mpi_ai(TEST_AI_CTX_S *params) {
    RK_S32 result;

    result = test_init_ai_data_read(params);
    if (result != 0) {
        RK_LOGE("ai file read init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    if (params->s32SetTrackMode) {
        RK_LOGI("test info : set track mode = %d", params->s32SetTrackMode);
        RK_MPI_AI_SetTrackMode(params->s32DevId, (AUDIO_TRACK_MODE_E)params->s32SetTrackMode);
        params->s32SetTrackMode = 0;
    }

    result = test_init_ai_aed(params);
    if (result != 0) {
        RK_LOGE("ai aed init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    result = test_init_ai_bcd(params);
    if (result != 0) {
        RK_LOGE("ai bcd init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    result = test_init_ai_buz(params);
    if (result != 0) {
        RK_LOGE("ai buz init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    result = test_init_ai_gbs(params);
    if (result != 0) {
        RK_LOGE("ai gbs init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    if (params->s32AedEnable || params->s32BcdEnable ||
        params->s32BuzEnable || params->s32GbsEnable) {
        result = test_init_ai_sed_agc(params);
        if (result != 0) {
            RK_LOGE("ai sed agc init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
            return RK_FAILURE;
        }

        result = test_init_ai_sed_fir(params);
        if (result != 0) {
            RK_LOGE("ai sed fir init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
            return RK_FAILURE;
        }
    }

    result = test_init_ai_vqe(params);
    if (result != 0) {
        RK_LOGE("ai vqe init fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    result =  RK_MPI_AI_EnableChn(params->s32DevId, params->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ai enable channel fail, aiChn = %d, reason = %x", params->s32ChnIndex, result);
        return RK_FAILURE;
    }

#if TEST_AI_WITH_FD
    // open fd immediate after enable chn will be better.
    params->s32DevFd = RK_MPI_AI_GetFd(params->s32DevId, params->s32ChnIndex);
    RK_LOGI("ai (devId: %d, chnId: %d), selectFd:%d", params->s32DevId, params->s32ChnIndex, params->s32DevFd);
#endif

    RK_BOOL needResample = (params->s32DeviceSampleRate != params->s32SampleRate) ? RK_TRUE : RK_FALSE;

    if (needResample == RK_TRUE) {
        RK_LOGI("need to resample %d -> %d", params->s32DeviceSampleRate, params->s32SampleRate);
        result = RK_MPI_AI_EnableReSmp(params->s32DevId, params->s32ChnIndex,
                                      (AUDIO_SAMPLE_RATE_E)params->s32SampleRate);
        if (result != 0) {
            RK_LOGE("ai enable channel fail, reason = %x, aiChn = %d", result, params->s32ChnIndex);
            return RK_FAILURE;
        }
    }

    RK_LOGI("Set volume curve type: %d", params->s32SetVolumeCurve);
    if ((params->s32SetVolumeCurve == AUDIO_CURVE_LOGARITHM) ||
        (params->s32SetVolumeCurve == AUDIO_CURVE_CUSTOMIZE)) {
        AUDIO_VOLUME_CURVE_S volumeCurve;

        volumeCurve.enCurveType = (AUDIO_VOLUME_CURVE_E)params->s32SetVolumeCurve;
        volumeCurve.s32Resolution = 101;
        volumeCurve.fMinDB = -51.0f;
        volumeCurve.fMaxDB = 0.0f;
        volumeCurve.pCurveTable = RK_NULL; // here none means using default logarithm curve by default.
        if (volumeCurve.enCurveType == AUDIO_CURVE_CUSTOMIZE) {
            volumeCurve.pCurveTable = (RK_U32 *)calloc(volumeCurve.s32Resolution, sizeof(RK_U32));
            RK_ASSERT(volumeCurve.pCurveTable != RK_NULL);
            // TODO: fill your customize table of volume curve folllowing to:
            // volumeCurve.pCurveTable[0, resolution-1]
        }
        RK_MPI_AI_SetVolumeCurve(params->s32DevId, &volumeCurve);
    }

    return RK_SUCCESS;
}

RK_S32 test_deinit_mpi_ai(TEST_AI_CTX_S *params) {
    RK_S32 result;

    RK_MPI_AI_DisableReSmp(params->s32DevId, params->s32ChnIndex);
    if (params->s32BuzEnable) {
        result = RK_MPI_AI_DisableBuz(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableBuz(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32BuzEnable = 0;
    }

    if (params->s32BcdEnable) {
        result = RK_MPI_AI_DisableBcd(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableBcd(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32BcdEnable = 0;
    }

    if (params->s32AedEnable) {
        result = RK_MPI_AI_DisableAed(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableAed(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32AedEnable = 0;
    }

    if (params->s32GbsEnable) {
        result = RK_MPI_AI_DisableGbs(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableGbs(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32GbsEnable = 0;
    }

    if (params->s32AedEnable || params->s32BcdEnable ||
        params->s32BuzEnable || params->s32GbsEnable) {
        result = RK_MPI_AI_DisableSedAgc(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableSedAgc(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }

        result = RK_MPI_AI_DisableSedFir(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableSedFir(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
    }

    if (params->s32VqeEnable) {
        result = RK_MPI_AI_DisableVqe(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableVqe(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32VqeEnable = 0;
    }

    if (params->s32DataReadEnable) {
        result = RK_MPI_AI_DisableDataRead(params->s32DevId, params->s32ChnIndex);
        if (result != RK_SUCCESS) {
            RK_LOGE("%s: RK_MPI_AI_DisableDataRead(%d,%d) failed with %#x",
                __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
            return result;
        }
        params->s32DataReadEnable = 0;
    }

    result = RK_MPI_AI_DisableChn(params->s32DevId, params->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ai disable channel fail, reason = %X", result);
        return RK_FAILURE;
    }

    result =  RK_MPI_AI_Disable(params->s32DevId);
    if (result != 0) {
        RK_LOGE("ai disable fail, reason = %X", result);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

static AUDIO_BIT_WIDTH_E find_bit_width(RK_S32 bit) {
    AUDIO_BIT_WIDTH_E bitWidth = AUDIO_BIT_WIDTH_BUTT;
    switch (bit) {
      case 8:
        bitWidth = AUDIO_BIT_WIDTH_8;
        break;
      case 16:
        bitWidth = AUDIO_BIT_WIDTH_16;
        break;
      case 24:
        bitWidth = AUDIO_BIT_WIDTH_24;
        break;
      default:
        RK_LOGE("bitwidth(%d) not support", bit);
        return AUDIO_BIT_WIDTH_BUTT;
    }

    return bitWidth;
}

static AUDIO_SOUND_MODE_E find_sound_mode(RK_S32 ch) {
    AUDIO_SOUND_MODE_E channel = AUDIO_SOUND_MODE_BUTT;
    switch (ch) {
      case 1:
        channel = AUDIO_SOUND_MODE_MONO;
        break;
      case 2:
        channel = AUDIO_SOUND_MODE_STEREO;
        break;
      case 4:
        channel = AUDIO_SOUND_MODE_4_CHN;
        break;
      case 6:
        channel = AUDIO_SOUND_MODE_6_CHN;
        break;
      case 8:
        channel = AUDIO_SOUND_MODE_8_CHN;
        break;
      default:
        RK_LOGE("channel = %d not support", ch);
        return AUDIO_SOUND_MODE_BUTT;
    }

    return channel;
}

static void* sendDataThread(void * ptr) {
    TEST_AI_CTX_S *params = (TEST_AI_CTX_S *)(ptr);

    RK_S32 result = 0;
    RK_S32 s32MilliSec = -1;
    RK_S32 bufferLen = 1024;
    RK_S32 frameSize = 0;
    RK_S32 frames = 0;
    AUDIO_FRAME_S sendFrame;
    RK_U8  *srcData = RK_NULL;
    RK_U8 *tmpData = RK_NULL;
    RK_S32 size = 0;
    RK_U64 timeStamp = 0;
    FILE *file = RK_NULL;

    file = fopen(params->srcFilePath, "rb");
    if (file == RK_NULL) {
        RK_LOGE("open input file %s failed because %s.", params->srcFilePath, strerror(errno));
        goto __EXIT;
    }

    if (params->s32AedEnable || params->s32BcdEnable ||
        params->s32BuzEnable || params->s32GbsEnable) {
        frameSize = params->s32DeviceChannel * params->s32BitWidth / 8;
        switch (params->s32DeviceSampleRate) {
            case 8000:
                bufferLen = frameSize * 128;
                break;
            case 16000:
                bufferLen = frameSize * 256;
                break;
            default:
                RK_LOGE("samplerate = %d not support", params->s32DeviceSampleRate);
                return RK_NULL;
        }
    }

    if (params->s32VqeEnable) {
        frameSize = params->s32DeviceChannel * params->s32BitWidth / 8;
        switch (params->s32DeviceSampleRate) {
            case 8000:
                bufferLen = frameSize * 128;
                break;
            case 16000:
                bufferLen = frameSize * 256;
                break;
            case 24000:
                bufferLen = frameSize * 384;
                break;
            case 32000:
                bufferLen = frameSize * 512;
                break;
            case 44100:
                bufferLen = frameSize * 704;
                break;
            case 48000:
                bufferLen = frameSize * 768;
                break;
            default:
                RK_LOGE("samplerate = %d not support", params->s32DeviceSampleRate);
                return RK_NULL;
        }
    }

    srcData = (RK_U8 *)(calloc(bufferLen, sizeof(RK_U8)));
    if (!srcData) {
        RK_LOGE("malloc data falied");
        goto __EXIT;
    }

    while (!gAiExit) {
        size = fread(srcData, 1, bufferLen, file);

        sendFrame.u32Len = size;
        sendFrame.u64TimeStamp = timeStamp++;
        sendFrame.s32SampleRate = params->s32DeviceSampleRate;
        sendFrame.enBitWidth = find_bit_width(params->s32BitWidth);
        sendFrame.enSoundMode = find_sound_mode(params->s32DeviceChannel);
        sendFrame.bBypassMbBlk = RK_FALSE;

        MB_EXT_CONFIG_S extConfig;
        memset(&extConfig, 0, sizeof(extConfig));
        extConfig.pOpaque = srcData;
        extConfig.pu8VirAddr = srcData;
        extConfig.u64Size = size;
        RK_MPI_SYS_CreateMB(&(sendFrame.pMbBlk), &extConfig);
__RETRY:
        result = RK_MPI_AI_SendFrame(params->s32DevId, params->s32ChnIndex, &sendFrame, s32MilliSec);
        if (result < 0) {
            RK_LOGE("send frame fail, result = %X, TimeStamp = %lld, s32MilliSec = %d",
                result, sendFrame.u64TimeStamp, s32MilliSec);
            goto __RETRY;
        }
        RK_MPI_MB_ReleaseMB(sendFrame.pMbBlk);

        if (size <= 0) {
            RK_LOGI("eof");
            break;
        }
    }

    if (gAiExit) {
        sendFrame.u32Len = 0;
        sendFrame.u64TimeStamp = timeStamp++;
        sendFrame.enBitWidth = find_bit_width(params->s32BitWidth);
        sendFrame.enSoundMode = find_sound_mode(params->s32DeviceChannel);
        sendFrame.bBypassMbBlk = RK_FALSE;

        MB_EXT_CONFIG_S extConfig;
        memset(&extConfig, 0, sizeof(extConfig));
        extConfig.pOpaque = srcData;
        extConfig.pu8VirAddr = srcData;
        extConfig.u64Size = 0;
        RK_MPI_SYS_CreateMB(&(sendFrame.pMbBlk), &extConfig);
        RK_LOGI("ai send frame exit");
        result = RK_MPI_AI_SendFrame(params->s32DevId, params->s32ChnIndex, &sendFrame, s32MilliSec);
        if (result < 0) {
            RK_LOGE("send frame fail, result = %X, TimeStamp = %lld, s32MilliSec = %d",
                result, sendFrame.u64TimeStamp, s32MilliSec);
        }

        RK_MPI_MB_ReleaseMB(sendFrame.pMbBlk);
    }

__EXIT:
    if (file) {
        fclose(file);
        file = RK_NULL;
    }

    if (srcData)
        free(srcData);

    if (tmpData)
        free(tmpData);

    return RK_NULL;
}

static void* getDataThread(void * ptr) {
    TEST_AI_CTX_S *params = (TEST_AI_CTX_S *)(ptr);

    RK_S32 result = 0;
    RK_S32 s32MilliSec = -1;
    AUDIO_FRAME_S getFrame;
    FILE *fp_aed, *fp_bcd, *fp_buz, *fp_gbs;
    RK_U8  *srcData = RK_NULL;
    RK_S16 *buf_aed, *buf_bcd, *buf_buz, *buf_gbs;
    RK_S32 s32AiAlgoFrames = 0;
    RK_U32 aed_count = 0, aed_flag = 0;
    RK_U32 bcd_count = 0, bcd_flag = 0;
    RK_U32 buz_count = 0, buz_flag = 0;
    RK_U32 gbs_count = 0, gbs_flag = 0;
    fd_set readfds, writefds;
    struct timeval timeout;
    // mode1: oneshot words
    const char *mode1_wakeup_words[8] = {
        "No wake",
        "Wo Yao Pai Zhao",
        "Guan Bi Re Dian",
        "Da Kai Re Dian",
        "Guan Bi Lu Yin",
        "Da Kai Lu Yin",
        "Guan Bi Ping Mu",
        "Da Kai Ping Mu",
    };
    // mode2: wakeup words
    const char *mode2_wakeup_words[2] = {
        "No wake",
        "Xiao He Xiao He",
    };
    // mode2: command words
    const char *mode2_command_words[19] = {
        "No command",
        "Gei <Ba Ba> Da Dian Hua",
        "Da Dian Hua Gei <Ba Ba>",
        "Hu Jiao <Ba Ba>",
        "Gei <Ma Ma> Da Dian Hua",
        "Da Dian Hua Gei <Ma Ma>",
        "Hu Jiao <Ma Ma>",
        "Gei <Ye Ye> Da Dian Hua",
        "Da Dian Hua Gei <Ye Ye>",
        "Hu Jiao <Ye Ye>",
        "Gei <Nai Nai> Da Dian Hua",
        "Da Dian Hua Gei <Nai Nai>",
        "Hu Jiao <Nai Nai>",
        "Gei <Wai Gong> Da Dian Hua",
        "Da Dian Hua Gei <Wai Gong>",
        "Hu Jiao <Wai Gong>",
        "Gei <Wai Po> Da Dian Hua",
        "Da Dian Hua Gei <Wai Po>",
        "Hu Jiao <Wai Po>",
    };

    if (params->dstFilePath) {
        AUDIO_SAVE_FILE_INFO_S save;
        save.bCfg = RK_TRUE;
        save.u32FileSize = 1024;
        snprintf(save.aFilePath, sizeof(save.aFilePath), "%s", params->dstFilePath);
        snprintf(save.aFileName, sizeof(save.aFileName), "%s", "cap_out.pcm");
        RK_MPI_AI_SaveFile(params->s32DevId, params->s32ChnIndex, &save);
    }

    if (params->s32SampleRate == 16000)
        s32AiAlgoFrames = AI_ALGO_FRAMES;
    else if (params->s32SampleRate == 8000)
        s32AiAlgoFrames = (AI_ALGO_FRAMES >> 1);

    /* Do not dump if s32AiAlgoFrames is invalid */
    if (s32AiAlgoFrames == 0)
        params->s32DumpAlgo = 0;

    if (params->s32DumpAlgo) {
        buf_aed = (RK_S16 *)calloc(s32AiAlgoFrames * 2 * sizeof(RK_S16), 1);
        RK_ASSERT(buf_aed != RK_NULL);
        fp_aed = fopen("/tmp/cap_aed_2ch.pcm", "wb");
        RK_ASSERT(fp_aed != RK_NULL);

        buf_bcd = (RK_S16 *)calloc(s32AiAlgoFrames * 1 * sizeof(RK_S16), 1);
        RK_ASSERT(buf_bcd != RK_NULL);
        fp_bcd = fopen("/tmp/cap_bcd_1ch.pcm", "wb");
        RK_ASSERT(fp_bcd != RK_NULL);

        buf_buz = (RK_S16 *)calloc(s32AiAlgoFrames * 1 * sizeof(RK_S16), 1);
        RK_ASSERT(buf_aed != RK_NULL);
        fp_buz = fopen("/tmp/cap_buz_1ch.pcm", "wb");
        RK_ASSERT(fp_buz != RK_NULL);

        buf_gbs = (RK_S16 *)calloc(s32AiAlgoFrames * 1 * sizeof(RK_S16), 1);
        RK_ASSERT(buf_aed != RK_NULL);
        fp_gbs = fopen("/tmp/cap_gbs_1ch.pcm", "wb");
        RK_ASSERT(fp_gbs != RK_NULL);
    }

    if (params->s32FdGetEnable) {
        RK_LOGI("%s - fd get enable", __func__);
        params->s32DevFd = RK_MPI_AI_GetFd(params->s32DevId, params->s32ChnIndex);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
    }

    while (!gAiExit || params->s32DataReadEnable) {
#if TEST_AI_WITH_FD
        test_ai_poll_event(-1, params->s32DevFd);
#endif
        if (params->s32AedEnable == 2) {
            if ((aed_count + 1) % 50 == 0) {
                result = RK_MPI_AI_DisableAed(params->s32DevId, params->s32ChnIndex);
                if (result != RK_SUCCESS) {
                    RK_LOGE("%s: RK_MPI_AI_DisableAed(%d,%d) failed with %#x",
                        __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
                    return RK_NULL;
                }
                if (aed_flag) {
                    RK_LOGI("%s - aed_count=%ld test_init_ai_aed\n", __func__, aed_count);
                    test_init_ai_aed(params);
                    aed_flag = 0;
                } else {
                    RK_LOGI("%s - aed_count=%ld test_init_ai_aed2\n", __func__, aed_count);
                    test_init_ai_aed2(params);
                    aed_flag = 1;
                }
            }
            aed_count++;
        }

        if (params->s32BcdEnable == 2) {
            if ((bcd_count + 1) % 50 == 0) {
                result = RK_MPI_AI_DisableBcd(params->s32DevId, params->s32ChnIndex);
                if (result != RK_SUCCESS) {
                    RK_LOGE("%s: RK_MPI_AI_DisableBcd(%d,%d) failed with %#x",
                        __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
                    return RK_NULL;
                }
                if (bcd_flag) {
                    RK_LOGI("%s - bcd_count=%ld test_init_ai_bcd\n", __func__, bcd_count);
                    test_init_ai_bcd(params);
                    bcd_flag = 0;
                } else {
                    RK_LOGI("%s - bcd_count=%ld test_init_ai_bcd2\n", __func__, bcd_count);
                    test_init_ai_bcd2(params);
                    bcd_flag = 1;
                }
            }
            bcd_count++;
        }

        if (params->s32BuzEnable == 2) {
            if ((buz_count + 1) % 50 == 0) {
                result = RK_MPI_AI_DisableBuz(params->s32DevId, params->s32ChnIndex);
                if (result != RK_SUCCESS) {
                    RK_LOGE("%s: RK_MPI_AI_DisableBuz(%d,%d) failed with %#x",
                        __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
                    return RK_NULL;
                }
                if (buz_flag) {
                    RK_LOGI("%s - buz_count=%ld test_init_ai_buz\n", __func__, buz_count);
                    test_init_ai_buz(params);
                    buz_flag = 0;
                } else {
                    RK_LOGI("%s - buz_count=%ld test_init_ai_buz2\n", __func__, buz_count);
                    test_init_ai_buz2(params);
                    buz_flag = 1;
                }
            }
            buz_count++;
        }

        if (params->s32GbsEnable == 2) {
            if ((gbs_count + 1) % 50 == 0) {
                result = RK_MPI_AI_DisableGbs(params->s32DevId, params->s32ChnIndex);
                if (result != RK_SUCCESS) {
                    RK_LOGE("%s: RK_MPI_AI_DisableGbs(%d,%d) failed with %#x",
                        __FUNCTION__, params->s32DevId, params->s32ChnIndex, result);
                    return RK_NULL;
                }
                if (gbs_flag) {
                    RK_LOGI("%s - gbs_count=%ld test_init_ai_gbs\n", __func__, gbs_count);
                    test_init_ai_gbs(params);
                    gbs_flag = 0;
                } else {
                    RK_LOGI("%s - gbs_count=%ld test_init_ai_gbs2\n", __func__, gbs_count);
                    test_init_ai_gbs2(params);
                    gbs_flag = 1;
                }
            }
            gbs_count++;
        }

        if (!params->s32FdGetEnable) {
            result = RK_MPI_AI_GetFrame(params->s32DevId, params->s32ChnIndex, &getFrame, RK_NULL, s32MilliSec);
            if (result == 0) {
                void* data = RK_MPI_MB_Handle2VirAddr(getFrame.pMbBlk);
                int   len  = getFrame.u32Len;
                RK_LOGV("data = %p, len = %d", data, getFrame.u32Len);
                RK_MPI_AI_ReleaseFrame(params->s32DevId, params->s32ChnIndex, &getFrame, RK_NULL);
                if (len <= 0) {
                    RK_LOGD("get ai frame end");
                    break;
                }

                if (params->s32QuitCnt > 0) {
                    if(!(--params->s32QuitCnt)) {
                        RK_LOGI("quit cnt is 0");
                        break;
                    }
                }
            } else {
                break;
            }
        } else {
            FD_ZERO(&readfds);
            FD_SET(params->s32DevFd, &readfds);
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            result = select(params->s32DevFd + 1, &readfds, NULL, NULL, &timeout);
            if (result == -1) {
                perror("select");
            } else if (result) {
                if (FD_ISSET(params->s32DevFd, &readfds)) {
                    result = RK_MPI_AI_GetFrame(params->s32DevId, params->s32ChnIndex, &getFrame, RK_NULL, s32MilliSec);
                    if (result == 0) {
                        void* data = RK_MPI_MB_Handle2VirAddr(getFrame.pMbBlk);
                        int   len  = getFrame.u32Len;
                        RK_LOGV("data = %p, len = %d", data, getFrame.u32Len);
                        RK_MPI_AI_ReleaseFrame(params->s32DevId, params->s32ChnIndex, &getFrame, RK_NULL);
                        if (len <= 0) {
                            RK_LOGD("get ai frame end");
                            break;
                        }

                        if (params->s32QuitCnt > 0) {
                            if(!(--params->s32QuitCnt)) {
                                RK_LOGI("quit cnt is 0");
                                break;
                            }
                        }
                    } else {
                        RK_LOGE("get ai frame failed, result = %X", result);
                    }
                }
            } else {
                printf("fd get timeout occurred %d\n", result);
            }
        }

        // dump results of VQE module
        if (params->s32VqeEnable && params->s32VqeResult) {
            AI_VQE_RESULT_S vqe_result;

            memset(&vqe_result, 0, sizeof(vqe_result));
            result = RK_MPI_AI_GetVqeResult(params->s32DevId, params->s32ChnIndex, &vqe_result);
            if (result == 0) {
                if (vqe_result.s32WakeupMode & 0x02) {
                    if (vqe_result.s32WakeupStatus > 0 && vqe_result.s32WakeupStatus <= 1)
                        RK_LOGI("VQE Result: mode2 wakeup word[%d]:\"%s\"\n",
                                vqe_result.s32WakeupStatus, mode2_wakeup_words[vqe_result.s32WakeupStatus]);
                    if (vqe_result.s32WakeupCmdID > 0 && vqe_result.s32WakeupCmdID <= 18) {
                        RK_LOGI("VQE Result: mode2 command word[%d]:\"%s\" score:%f\n",
                                vqe_result.s32WakeupCmdID,
                                mode2_command_words[vqe_result.s32WakeupCmdID],
                                vqe_result.s32WakeupCmdScore);
                    }
                } else if (vqe_result.s32WakeupMode & 0x01) {
                    if (vqe_result.s32WakeupStatus > 0 && vqe_result.s32WakeupStatus <= 7)
                        RK_LOGI("VQE Result: mode1 wakeup word[%d]:\"%s\"\n",
                                vqe_result.s32WakeupStatus, mode1_wakeup_words[vqe_result.s32WakeupStatus]);
                }
            }
        }

        // dump results of SED(AED/BCD) modules
        if (params->s32AedEnable) {
            AI_AED_RESULT_S aed_result;

            memset(&aed_result, 0, sizeof(aed_result));
            result = RK_MPI_AI_GetAedResult(params->s32DevId, params->s32ChnIndex, &aed_result);
            if (result == 0) {
                if (aed_result.bAcousticEventDetected)
                    RK_LOGI("AED Result: AcousticEvent:%d",
                            aed_result.bAcousticEventDetected);
                if (aed_result.bLoudSoundDetected) {
                    params->s32AedLoudCount++;
                    RK_LOGI("AED Result: LoudSound:%d",
                            aed_result.bLoudSoundDetected);
                }

                if (aed_result.bLoudSoundDetected)
                    RK_LOGI("AED Result: LoudSound Volume Result:%f db",
                            aed_result.lsdResult);
            }
            if (params->s32DumpAlgo) {
                for (RK_S32 i = 0; i < s32AiAlgoFrames; i++) {
                    *(buf_aed + 2 * i + 0) = 10000 * aed_result.bAcousticEventDetected;
                    *(buf_aed + 2 * i + 1) = 10000 * aed_result.bLoudSoundDetected;
                }
                fwrite(buf_aed, s32AiAlgoFrames * 2 * sizeof(RK_S16), 1, fp_aed);
            }
        }

        if (params->s32BcdEnable) {
            AI_BCD_RESULT_S bcd_result;

            memset(&bcd_result, 0, sizeof(bcd_result));
            result = RK_MPI_AI_GetBcdResult(params->s32DevId, params->s32ChnIndex, &bcd_result);
            if (result == 0 && bcd_result.bBabyCry) {
                params->s32BcdCount++;
                RK_LOGI("BCD Result: BabyCry:%d", bcd_result.bBabyCry);
            }
            if (params->s32DumpAlgo) {
                for (RK_S32 i = 0; i < s32AiAlgoFrames; i++) {
                    *(buf_bcd + 1 * i) = 10000 * bcd_result.bBabyCry;
                }
                fwrite(buf_bcd, s32AiAlgoFrames * 1 * sizeof(RK_S16), 1, fp_bcd);
            }
        }

        if (params->s32BuzEnable) {
            AI_BUZ_RESULT_S buz_result;

            memset(&buz_result, 0, sizeof(buz_result));
            result = RK_MPI_AI_GetBuzResult(params->s32DevId, params->s32ChnIndex, &buz_result);
            if (result == 0 && buz_result.bBuzz) {
                params->s32BuzCount++;
                RK_LOGI("BUZ Result: Buzz:%d", buz_result.bBuzz);
            }
            if (params->s32DumpAlgo) {
                for (RK_S32 i = 0; i < s32AiAlgoFrames; i++) {
                    *(buf_buz + 1 * i) = 10000 * buz_result.bBuzz;
                }
                fwrite(buf_buz, s32AiAlgoFrames * 1 * sizeof(RK_S16), 1, fp_buz);
            }
        }

        if (params->s32GbsEnable) {
            AI_GBS_RESULT_S gbs_result;

            memset(&gbs_result, 0, sizeof(gbs_result));
            result = RK_MPI_AI_GetGbsResult(params->s32DevId, params->s32ChnIndex, &gbs_result);
            if (result == 0 && gbs_result.bGbs) {
                params->s32GbsCount++;
                RK_LOGI("GBS Result: Gbs:%d", gbs_result.bGbs);
            }
            if (params->s32DumpAlgo) {
                for (RK_S32 i = 0; i < s32AiAlgoFrames; i++) {
                    *(buf_gbs + 1 * i) = 10000 * gbs_result.bGbs;
                }
                fwrite(buf_gbs, s32AiAlgoFrames * 1 * sizeof(RK_S16), 1, fp_gbs);
            }
        }
    }

    if (params->s32DumpAlgo) {
        if (buf_aed)
            free(buf_aed);
        if (buf_bcd)
            free(buf_bcd);
        if (buf_buz)
            free(buf_buz);
        if (buf_gbs)
            free(buf_gbs);

        if (fp_aed)
            fclose(fp_aed);
        if (fp_bcd)
            fclose(fp_bcd);
        if (fp_buz)
            fclose(fp_buz);
        if (fp_gbs)
            fclose(fp_gbs);
    }

    return RK_NULL;
}

static void* commandThread(void * ptr) {
    TEST_AI_CTX_S *params = (TEST_AI_CTX_S *)(ptr);

    AUDIO_FADE_S aFade;
    aFade.bFade = RK_FALSE;
    aFade.enFadeOutRate = (AUDIO_FADE_RATE_E)params->s32SetFadeRate;
    aFade.enFadeInRate = (AUDIO_FADE_RATE_E)params->s32SetFadeRate;
    RK_BOOL mute = (params->s32SetMute == 0) ? RK_FALSE : RK_TRUE;
    RK_LOGI("test info : mute = %d, volume = %d", mute, params->s32SetVolume);
    RK_MPI_AI_SetMute(params->s32DevId, mute, &aFade);
    RK_MPI_AI_SetVolume(params->s32DevId, params->s32SetVolume);
    if (params->s32GetVolume) {
        RK_S32 volume = 0;
        RK_MPI_AI_GetVolume(params->s32DevId, &volume);
        RK_LOGI("test info : get volume = %d", volume);
        params->s32GetVolume = 0;
    }

    if (params->s32GetMute) {
        RK_BOOL mute = RK_FALSE;
        AUDIO_FADE_S fade;
        RK_MPI_AI_GetMute(params->s32DevId, &mute, &fade);
        RK_LOGI("test info : is mute = %d", mute);
        params->s32GetMute = 0;
    }

    if (params->s32GetTrackMode) {
        AUDIO_TRACK_MODE_E trackMode;
        RK_MPI_AI_GetTrackMode(params->s32DevId, &trackMode);
        RK_LOGI("test info : get track mode = %d", trackMode);
        params->s32GetTrackMode = 0;
    }

    return RK_NULL;
}

static RK_S32 test_set_channel_params_ai(TEST_AI_CTX_S *params) {
    AUDIO_DEV aiDevId = params->s32DevId;
    AI_CHN aiChn = params->s32ChnIndex;
    RK_S32 result = 0;
    AI_CHN_PARAM_S pstParams;

    memset(&pstParams, 0, sizeof(AI_CHN_PARAM_S));
    pstParams.enLoopbackMode = (AUDIO_LOOPBACK_MODE_E)params->s32LoopbackMode;
    pstParams.s32UsrFrmDepth = 4;
    pstParams.u32MapPtNumPerFrm = params->s32FrameLength;
    pstParams.enSamplerate = (AUDIO_SAMPLE_RATE_E)params->s32SampleRate;
    pstParams.s32SedQueLen = params->s32SedQueLen;
    result = RK_MPI_AI_SetChnParam(aiDevId, aiChn, &pstParams);
    if (result != RK_SUCCESS) {
        RK_LOGE("ai set channel params failed, aiChn = %d, max chn count %d",
                 aiChn, AI_MAX_CHN_NUM);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 unit_test_mpi_ai(TEST_AI_CTX_S *ctx) {
    RK_S32 i = 0;
    TEST_AI_CTX_S params[AI_MAX_CHN_NUM];
    rkos_task_t *tidSend[AI_MAX_CHN_NUM] = {0};
    rkos_task_t *tidGet[AI_MAX_CHN_NUM] = {0};
    rkos_task_t *tidComand[AI_MAX_CHN_NUM] = {0};
    RK_S32 result = RK_SUCCESS;

    for (i = 0; i < ctx->s32ChnNum; i++)
        memset(&(params[i]), 0, sizeof(TEST_AI_CTX_S));

    if (test_open_device_ai(ctx) != RK_SUCCESS) {
        return RK_FAILURE;
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        memcpy(&(params[i]), ctx, sizeof(TEST_AI_CTX_S));
        params[i].s32ChnIndex = i;
        params[i].s32DevFd = -1;
        result = test_set_channel_params_ai(&params[i]);
        if (result != RK_SUCCESS)
            goto __FAILED;
        result = test_init_mpi_ai(&params[i]);
        if (result != RK_SUCCESS)
            goto __FAILED;

        if (ctx->s32DataReadEnable) {
            tidSend[i] = rkos_kthread_create(sendDataThread, (void *)(&params[i]), "sendDataThread", 40960, 0);
            if (!tidSend[i]) {
                RK_LOGE("send thread create failed");
                goto __FAILED;
            }
        }

		tidGet[i] = rkos_kthread_create(getDataThread, (void *)(&params[i]), "getDataThread", 40960, 0);
		if (!tidGet[i]) {
			RK_LOGE("get thread create failed");
			goto __FAILED;
		}

		tidComand[i] = rkos_kthread_create(commandThread, (void *)(&params[i]), "commandThread", 40960, 0);
		if (!tidComand[i]) {
			RK_LOGE("command thread create failed");
			goto __FAILED;
		}
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        if (tidSend[i]) {
            rkos_kthread_destory(tidSend[i], 1);
            tidSend[i] = NULL;
        }

        if (tidGet[i]) {
            rkos_kthread_destory(tidGet[i], 1);
            tidGet[i] = NULL;
        }

        if (tidComand[i]) {
            rkos_kthread_destory(tidComand[i], 1);
            tidComand[i] = NULL;
        }

        ctx->s32AedLoudCount    = params[i].s32AedLoudCount;
        ctx->s32BcdCount        = params[i].s32BcdCount;
        ctx->s32BuzCount        = params[i].s32BuzCount;
        ctx->s32GbsCount        = params[i].s32GbsCount;
        result = test_deinit_mpi_ai(&params[i]);
        if (result != RK_SUCCESS)
            goto __FAILED;
    }

    return RK_SUCCESS;
__FAILED:
    for (i = 0; i < ctx->s32ChnNum; i++)
        result = test_deinit_mpi_ai(&params[i]);
    return RK_FAILURE;
}

static void mpi_ai_test_show_options(const TEST_AI_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name      : %s\n", ctx->srcFilePath);
    RK_PRINT("output file name      : %s\n", ctx->dstFilePath);
    RK_PRINT("loop count            : %d\n", ctx->s32LoopCount);
    RK_PRINT("channel number        : %d\n", ctx->s32ChnNum);
    RK_PRINT("open sound rate       : %d\n", ctx->s32DeviceSampleRate);
    RK_PRINT("record data rate      : %d\n", ctx->s32SampleRate);
    RK_PRINT("sound card channel    : %d\n", ctx->s32DeviceChannel);
    RK_PRINT("output channel        : %d\n", ctx->s32Channel);
    RK_PRINT("bit_width             : %d\n", ctx->s32BitWidth);
    RK_PRINT("frame_number          : %d\n", ctx->s32FrameNumber);
    RK_PRINT("frame_length          : %d\n", ctx->s32FrameLength);
    RK_PRINT("sound card name       : %s\n", ctx->chCardName);
    RK_PRINT("device id             : %d\n", ctx->s32DevId);
    RK_PRINT("set volume curve      : %d\n", ctx->s32SetVolumeCurve);
    RK_PRINT("set volume            : %d\n", ctx->s32SetVolume);
    RK_PRINT("set mute              : %d\n", ctx->s32SetMute);
    RK_PRINT("set track_mode        : %d\n", ctx->s32SetTrackMode);
    RK_PRINT("get volume            : %d\n", ctx->s32GetVolume);
    RK_PRINT("get mute              : %d\n", ctx->s32GetMute);
    RK_PRINT("get track_mode        : %d\n", ctx->s32GetTrackMode);
    RK_PRINT("data read enable      : %d\n", ctx->s32DataReadEnable);
    RK_PRINT("aed enable            : %d\n", ctx->s32AedEnable);
    RK_PRINT("bcd enable            : %d\n", ctx->s32BcdEnable);
    RK_PRINT("buz enable            : %d\n", ctx->s32BuzEnable);
    RK_PRINT("vqe gap duration (ms) : %d\n", ctx->s32VqeGapMs);
    RK_PRINT("vqe enable            : %d\n", ctx->s32VqeEnable);
    RK_PRINT("get vqe result        : %d\n", ctx->s32VqeResult);
    RK_PRINT("bcd NN model path     : %s\n", ctx->pBcdModelPath);
    RK_PRINT("vqe config file       : %s\n", ctx->pVqeCfgPath);
    RK_PRINT("dump algo pcm data    : %d\n", ctx->s32DumpAlgo);
    RK_PRINT("dev queue len         : %d\n", ctx->s32DevQueLen);
    RK_PRINT("sed queue len         : %d\n", ctx->s32SedQueLen);
    RK_PRINT("fd get enable         : %d\n", ctx->s32FdGetEnable);
}

static const char *const usages[] = {
    "./rk_mpi_ai_test [--device_rate rate] [--device_ch ch] [--out_rate rate] [--out_ch ch] "
                     "[--aed_enable] [--bcd_enable] [--buz_enable] [--vqe_enable] [--vqe_cfg] [--vqe_result]...",
    NULL,
};

int rk_mpi_ai_test_enter(int argc, const char **argv) {
    RK_S32          i;
    RK_S32          s32Ret = 0;
    TEST_AI_CTX_S  *ctx;

    ctx = (TEST_AI_CTX_S *)(malloc(sizeof(TEST_AI_CTX_S)));
    memset(ctx, 0, sizeof(TEST_AI_CTX_S));

    ctx->srcFilePath        = RK_NULL;
    ctx->dstFilePath        = RK_NULL;
    ctx->s32LoopCount       = 1;
    ctx->s32ChnNum          = 1;
    ctx->s32BitWidth        = 16;
    ctx->s32FrameNumber     = 4;
    ctx->s32FrameLength     = 1024;
    ctx->chCardName         = RK_NULL;
    ctx->s32DevId           = 0;
    ctx->s32SetVolumeCurve  = 0;
    ctx->s32SetVolume       = 100;
    ctx->s32SetMute         = 0;
    ctx->s32SetTrackMode    = 0;
    ctx->s32SetFadeRate     = 0;
    ctx->s32GetVolume       = 0;
    ctx->s32GetMute         = 0;
    ctx->s32GetTrackMode    = 0;
    ctx->s32DataReadEnable  = 0;
    ctx->s32AedEnable       = 0;
    ctx->s32BcdEnable       = 0;
    ctx->s32BuzEnable       = 0;
    ctx->s32GbsEnable       = 0;
    ctx->s32VqeGapMs        = 16;
    ctx->s32VqeEnable       = 0;
    ctx->pBcdModelPath      = RK_NULL;
    ctx->pVqeCfgPath        = RK_NULL;
    ctx->s32LoopbackMode    = AUDIO_LOOPBACK_NONE;
    ctx->s32DumpAlgo        = 0;
    ctx->s32DevQueLen       = 0;
    ctx->s32SedQueLen       = 0;
    ctx->s32FdGetEnable     = 1;
    ctx->s32QuitCnt         = -1;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),

        OPT_INTEGER('\0', "device_rate", &(ctx->s32DeviceSampleRate),
                    "the sample rate of open sound card.  <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "device_ch", &(ctx->s32DeviceChannel),
                    "the number of sound card channels. <required>.", NULL, 0, 0),
        OPT_INTEGER('\0', "out_ch", &(ctx->s32Channel),
                    "the channels of out data. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "out_rate", &(ctx->s32SampleRate),
                    "the sample rate of out data. <required>", NULL, 0, 0),
        OPT_STRING('i', "input", &(ctx->srcFilePath),
                    "input file name, e.g.(./ai). default(NULL).", NULL, 0, 0),
        OPT_STRING('o', "output", &(ctx->dstFilePath),
                    "output file name, e.g.(./ai). default(NULL).", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx->s32LoopCount),
                    "loop running count. can be any count. default(1)", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx->s32ChnNum),
                    "the count of adec channel. default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "bit", &(ctx->s32BitWidth),
                    "the bit width of open sound card, range(8, 16, 24), default(16)", NULL, 0, 0),
        OPT_INTEGER('\0', "frame_length", &(ctx->s32FrameLength),
                    "the bytes size of output frame", NULL, 0, 0),
        OPT_INTEGER('\0', "frame_number", &(ctx->s32FrameNumber),
                    "the max frame num in buf", NULL, 0, 0),
        OPT_STRING('\0', "sound_card_name", &(ctx->chCardName),
                    "the sound name for open sound card, default(NULL)", NULL, 0, 0),
        OPT_INTEGER('\0', "set_volume_curve", &(ctx->s32SetVolumeCurve),
                    "set volume curve(builtin linear), 0:unset 1:linear 2:logarithm 3:customize. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "set_volume", &(ctx->s32SetVolume),
                    "set volume test, range(0, 100), default(100)", NULL, 0, 0),
        OPT_INTEGER('\0', "set_mute", &(ctx->s32SetMute),
                    "set mute test, range(0, 1), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "set_fade", &(ctx->s32SetFadeRate),
                    "set fade rate, range(0, 7), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "set_track_mode", &(ctx->s32SetTrackMode),
                    "set track mode test, range(0:normal, 1:both_left, 2:both_right, 3:exchange, 4:mix,"
                    "5:left_mute, 6:right_mute, 7:both_mute, 8: only left, 9: only right, 10:out stereo), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "get_volume", &(ctx->s32GetVolume),
                    "get volume test, range(0, 1), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "get_mute", &(ctx->s32GetMute),
                    "get mute test, range(0, 1), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "get_track_mode", &(ctx->s32GetTrackMode),
                    "get track mode test, range(0, 1), default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "data_read_enable", &(ctx->s32DataReadEnable),
                    "the data read enable, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "aed_enable", &(ctx->s32AedEnable),
                    "the aed enable, 0:disable 1:enable 2:reload test. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "bcd_enable", &(ctx->s32BcdEnable),
                    "the bcd enable, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "buz_enable", &(ctx->s32BuzEnable),
                    "the buz enable, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "gbs_enable", &(ctx->s32GbsEnable),
                    "the gbs enable, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "vqe_gap", &(ctx->s32VqeGapMs),
                    "the vqe gap duration in milliseconds. only supports 16ms or 10ms. default(16).", NULL, 0, 0),
        OPT_INTEGER('\0', "vqe_enable", &(ctx->s32VqeEnable),
                    "the vqe enable, 0:disable 1:enable 2:VqeModEnable. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "vqe_result", &(ctx->s32VqeResult),
                    "get vqe result, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_STRING('\0', "bcd_model", &(ctx->pBcdModelPath),
                    "the bcd config file, default(NULL)", NULL, 0, 0),
        OPT_STRING('\0', "vqe_cfg", &(ctx->pVqeCfgPath),
                    "the vqe config file, default(NULL)", NULL, 0, 0),
        OPT_INTEGER('\0', "loopback_mode", &(ctx->s32LoopbackMode),
                    "configure the loopback mode during ai runtime", NULL, 0, 0),
        OPT_INTEGER('\0', "dump_algo", &(ctx->s32DumpAlgo),
                    "dump algorithm pcm data during ai runtime", NULL, 0, 0),
        OPT_INTEGER('\0', "dev_queue_len", &(ctx->s32DevQueLen),
                    "the dev queue len. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "sed_queue_len", &(ctx->s32SedQueLen),
                    "the sed queue len. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "fd_get_enable", &(ctx->s32FdGetEnable),
                    "the fd get enable, 0:disable 1:enable. default(0).", NULL, 0, 0),
        OPT_INTEGER('q', "quit_cnt", &(ctx->s32QuitCnt),
                    "the quit count. default(-1).", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    if (argc < 0)
        return 0;

    mpi_ai_test_show_options(ctx);

    if (ctx->s32Channel <= 0
        || ctx->s32SampleRate <= 0
        || ctx->s32DeviceSampleRate <= 0
        || ctx->s32DeviceChannel <= 0
        || (ctx->s32DataReadEnable && (ctx->srcFilePath == NULL))) {
        argparse_usage(&argparse);
        s32Ret = -1;
        goto __FAILED;
    }

    // setup the SIGINT to ctrl+c and handing de-init process
    signal(SIGINT, sigterm_handler);

    RK_MPI_SYS_Init();

    for (i = 0; i < ctx->s32LoopCount; i++) {
        ctx->s32AedLoudCount    = 0;
        ctx->s32BcdCount        = 0;
        ctx->s32BuzCount        = 0;
        ctx->s32GbsCount        = 0;
        RK_LOGI("start running loop count  = %d", i);
        s32Ret = unit_test_mpi_ai(ctx);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("unit_test_mpi_ai failed: %d", s32Ret);
            s32Ret = -1;
            goto __FAILED;
        }
        RK_LOGI("end running loop count  = %d", i);
    }

    if (ctx->s32AedEnable && ctx->s32AedLoudCount < 10) {
        RK_LOGE("Aed Loud event count: %d", ctx->s32AedLoudCount);
        s32Ret = -1;
    } else if (ctx->s32AedEnable) {
        RK_LOGD("Aed Loud event count: %d", ctx->s32AedLoudCount);
    }

    if (ctx->s32BcdEnable && ctx->s32BcdCount < 10) {
        RK_LOGE("Bcd event count: %d", ctx->s32BcdCount);
        s32Ret = -1;
    } else if (ctx->s32BcdEnable) {
        RK_LOGD("Bcd event count: %d", ctx->s32BcdCount);
    }

    if (ctx->s32BuzEnable && ctx->s32BuzCount < 10) {
        RK_LOGE("Buz event count: %d", ctx->s32BuzCount);
        s32Ret = -1;
    } else if (ctx->s32BuzEnable) {
        RK_LOGD("Buz event count: %d", ctx->s32BuzCount);
    }

    if (ctx->s32GbsEnable && ctx->s32GbsCount < 10) {
        RK_LOGE("Gbs event count: %d", ctx->s32GbsCount);
        s32Ret = -1;
    } else if (ctx->s32GbsEnable) {
        RK_LOGD("Gbs event count: %d", ctx->s32GbsCount);
    }

__FAILED:
    if (ctx) {
        free(ctx);
        ctx = RK_NULL;
    }

    RK_MPI_SYS_Exit();
    return s32Ret;
}

#ifdef OS_LINUX
int main(int argc, const char **argv) {
    return rk_mpi_ai_test_enter(argc, argv);
}
#endif

#ifdef OS_RTT
#include <finsh.h>
int rk_mpi_ai_test(int argc, char **argv)
{
    argparse_excute_main(argc, argv, rk_mpi_ai_test_enter);
    return 0;
}

MSH_CMD_EXPORT(rk_mpi_ai_test, rockit ai module test);
#endif
