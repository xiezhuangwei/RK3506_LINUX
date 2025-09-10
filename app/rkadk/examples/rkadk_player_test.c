/*
 * Copyright (c) 2021 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "rkadk_common.h"
#include "rkadk_media_comm.h"
#include "rkadk_log.h"
#include "rkadk_param.h"
#include "rkadk_player.h"
#include "rkadk_demuxer.h"
#ifndef OS_RTT
#include "rkdemuxer.h"
#else
#include "../../common_algorithm/misc/include/rkdemuxer.h"
#endif
#include <math.h>

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

extern int optind;
extern char *optarg;
static bool is_quit = false;
static bool is_destroy = false;
static RKADK_CHAR optstr[] = "i:x:y:W:H:r:a:s:P:I:t:F:T:l:c:d:O:S:V:R:w:C:A:M:N:mfvbDh";

static RKADK_VOID *mDemuxerCfg = NULL;
static void print_usage(const RKADK_CHAR *name) {
  printf("usage example:\n");
  printf("\t%s [-i xxx.mp4] [-x 180] [-y 320] [-W 360] [-H 640] [-r 90] "
         "[-m] [-f] [-v] [-b]\n",
         name);
  printf("\t-i: input url, Default: /etc/bsa_file/8k8bpsMono.wav\n");
  printf("\t-x: display x coordinate, Default: 0\n");
  printf("\t-y: display y coordinate, Default: 0\n");
  printf("\t-W: display width, Default: Physical screen width\n");
  printf("\t-H: display height, Default: Physical screen height\n");
  printf("\t-r: rotation, option: 0(0), 1(90), 2(180), 3(270), Default: 0\n");
  printf("\t-m: mirror enable, Default: disable\n");
  printf("\t-f: flip enable, Default: disable\n");
  printf("\t-a: set audio enable/disable, option: 0, 1; Default: enable\n");
  printf("\t-v: video enable, Default: disable\n");
  printf("\t-s: vo layer splice mode, option: 0(RGA), 1(GPU), 2(ByPass); Default: 0\n");
  printf("\t-P: display pixel type, option: 0(RGB888), 1(NV12), 2(RGB565), 3(RGB444); Default: 0\n");
  printf("\t-I: Type of a VO interface, option: 0(DEFAILT), 1(MIPI), 2(LCD); Default: 1106: 0, other chip: 1\n");
  printf("\t-F: vo display framerete, Default: 30\n");
  printf("\t-t: rtsp transport protocol, option: 0(udp), 1(tcp); Default: udp\n");
  printf("\t-b: Black Backgound enable, Default: disable\n");
  printf("\t-T: rtsp socket I/O timeout(millisecond), option: block\n");
  printf("\t-l: vo layer id, Default: 0\n");
  printf("\t-c: loop play count, Default: 0\n");
  printf("\t-d: loop play once duration(second), Default: file duration\n");
  printf("\t-O: Vdec output buffer count, Default: 3\n");
  printf("\t-S: Vdec stream(input) buffer count, Default: 3\n");
  printf("\t-w: Vdec waterline(frames), Default: 0\n");
  printf("\t-D: enable third-party demuxer, Default: disable\n");
  printf("\t-A: adec buffer len, Default: 4\n");
  printf("\t-M: vdec decode mode, option: 0(frame mode), 1(stream mode), 2(send frame decode slice), 3(send slice decode slice), Default: 0\n");
  printf("\t-C: Ao sound card name, Default: RV1106/RV1103/RK3506 = hw:0,0, other chip = default\n");
  printf("\t-V: ao volume, Default: 70\n");
  printf("\t-A: ao sound card samplerate, Default: 16000\n");
  printf("\t-N: ao sound card channel index, Default: 0\n");
  printf("\t-h: help\n");
}

static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  is_quit = true;
}

static RKADK_VOID PlayerEventFnTest(RKADK_MW_PTR pPlayer,
                                    RKADK_PLAYER_EVENT_E enEvent,
                                    RKADK_VOID *pData) {
  switch (enEvent) {
  case RKADK_PLAYER_EVENT_STATE_CHANGED:
    printf("+++++ RKADK_PLAYER_EVENT_STATE_CHANGED +++++\n");
    break;
  case RKADK_PLAYER_EVENT_EOF:
    printf("+++++ RKADK_PLAYER_EVENT_EOF +++++\n");
#ifdef OS_RTT
    is_quit = true;
#endif
    break;
  case RKADK_PLAYER_EVENT_SOF:
    printf("+++++ RKADK_PLAYER_EVENT_SOF +++++\n");
    break;
  case RKADK_PLAYER_EVENT_SEEK_END:
    printf("+++++ RKADK_PLAYER_EVENT_SEEK_END +++++\n");
    break;
  case RKADK_PLAYER_EVENT_ERROR:
    printf("+++++ RKADK_PLAYER_EVENT_ERROR +++++\n");
    break;
  case RKADK_PLAYER_EVENT_PREPARED:
    printf("+++++ RKADK_PLAYER_EVENT_PREPARED +++++\n");
    break;
  case RKADK_PLAYER_EVENT_PLAY:
    printf("+++++ RKADK_PLAYER_EVENT_PLAY +++++\n");
    break;
  case RKADK_PLAYER_EVENT_PAUSED:
    printf("+++++ RKADK_PLAYER_EVENT_PAUSED +++++\n");
    break;
  case RKADK_PLAYER_EVENT_STOPPED:
    printf("+++++ RKADK_PLAYER_EVENT_STOPPED +++++\n");
    break;
  default:
    printf("+++++ Unknown event(%d) +++++\n", enEvent);
    break;
  }
}

void param_init(RKADK_PLAYER_FRAME_INFO_S *pstFrmInfo) {
  RKADK_CHECK_POINTER_N(pstFrmInfo);

  memset(pstFrmInfo, 0, sizeof(RKADK_PLAYER_FRAME_INFO_S));
#ifndef OS_RTT
#ifdef RK3576
  pstFrmInfo->u32DispWidth = 1080;
  pstFrmInfo->u32DispHeight = 1920;
#else
  pstFrmInfo->u32DispWidth = 720;
  pstFrmInfo->u32DispHeight = 1280;
#endif
  pstFrmInfo->u32ImgWidth = pstFrmInfo->u32DispWidth;
  pstFrmInfo->u32ImgHeight = pstFrmInfo->u32DispHeight;
  pstFrmInfo->u32VoFormat = VO_FORMAT_RGB888;
#if defined(RV1106_1103) || defined(RV1103B)
  pstFrmInfo->u32EnIntfType = DISPLAY_TYPE_DEFAULT;
#else
  pstFrmInfo->u32EnIntfType = DISPLAY_TYPE_MIPI;
#endif
  pstFrmInfo->enIntfSync = RKADK_VO_OUTPUT_DEFAULT;
  pstFrmInfo->u32BorderColor = 0x0000FA;
  pstFrmInfo->bMirror = RKADK_FALSE;
  pstFrmInfo->bFlip = RKADK_FALSE;
  pstFrmInfo->u32Rotation = 1;
  pstFrmInfo->stSyncInfo.bIdv = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIhs = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIvs = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bSynm = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIop = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.u16FrameRate = 30;
  pstFrmInfo->stSyncInfo.u16PixClock = 65000;
  pstFrmInfo->stSyncInfo.u16Hact = 1200;
  pstFrmInfo->stSyncInfo.u16Hbb = 24;
  pstFrmInfo->stSyncInfo.u16Hfb = 240;
  pstFrmInfo->stSyncInfo.u16Hpw = 136;
  pstFrmInfo->stSyncInfo.u16Hmid = 0;
  pstFrmInfo->stSyncInfo.u16Vact = 1200;
  pstFrmInfo->stSyncInfo.u16Vbb = 200;
  pstFrmInfo->stSyncInfo.u16Vfb = 194;
  pstFrmInfo->stSyncInfo.u16Vpw = 6;
  pstFrmInfo->enVoSpliceMode = SPLICE_MODE_RGA;
#ifdef RK3576
  pstFrmInfo->u32VoDev = 1;
  pstFrmInfo->u32VoLay = 5;
#endif
#else
  pstFrmInfo->u32DispWidth = 720;
  pstFrmInfo->u32DispHeight = 1280;
  pstFrmInfo->u32ImgWidth = pstFrmInfo->u32DispWidth;
  pstFrmInfo->u32ImgHeight = pstFrmInfo->u32DispHeight;
  pstFrmInfo->u32VoFormat = VO_FORMAT_RGB888;
#if defined(RV1106_1103) || defined(RV1103B)
  pstFrmInfo->u32EnIntfType = DISPLAY_TYPE_DEFAULT;
#else
  pstFrmInfo->u32EnIntfType = DISPLAY_TYPE_MIPI;
#endif
  pstFrmInfo->enIntfSync = RKADK_VO_OUTPUT_480P60;
  pstFrmInfo->u32BorderColor = 0x0000FA;
  pstFrmInfo->bMirror = RKADK_FALSE;
  pstFrmInfo->bFlip = RKADK_FALSE;
  pstFrmInfo->u32Rotation = 1;
  pstFrmInfo->stSyncInfo.bIdv = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIhs = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIvs = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bSynm = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.bIop = RKADK_TRUE;
  pstFrmInfo->stSyncInfo.u16FrameRate = 25;
  pstFrmInfo->stSyncInfo.u16PixClock = 65000;
  pstFrmInfo->stSyncInfo.u16Hact = 1200;
  pstFrmInfo->stSyncInfo.u16Hbb = 24;
  pstFrmInfo->stSyncInfo.u16Hfb = 240;
  pstFrmInfo->stSyncInfo.u16Hpw = 136;
  pstFrmInfo->stSyncInfo.u16Hmid = 0;
  pstFrmInfo->stSyncInfo.u16Vact = 1200;
  pstFrmInfo->stSyncInfo.u16Vbb = 200;
  pstFrmInfo->stSyncInfo.u16Vfb = 194;
  pstFrmInfo->stSyncInfo.u16Vpw = 6;
  pstFrmInfo->enVoSpliceMode = SPLICE_MODE_RGA;
#endif

  return;
}

RKADK_VOID *GetPosition(RKADK_VOID *arg) {
    RKADK_S64 position = 0;
    while (!is_quit && !is_destroy) {
      position = RKADK_PLAYER_GetCurrentPosition(arg);
      printf("position = %lld\n", position);

#ifndef OS_RTT
      usleep(1000000);
#else
      rkos_msleep(1000);
#endif
    }

  return NULL;
}

static void SnapshotDataRecv(RKADK_PLAYER_SNAPSHOT_S *pstData) {
  static RKADK_U32 snapshotId = 0;
  char jpegPath[128];
  FILE *file = NULL;

  if (!pstData || !pstData->pu8DataBuf) {
    RKADK_LOGE("Invalid photo data");
    return;
  }

  memset(jpegPath, 0, 128);
  sprintf(jpegPath, "/tmp/Snapshot_%d.jpeg", snapshotId);
  file = fopen(jpegPath, "w");
  if (!file) {
    RKADK_LOGE("Create jpeg file(%s) failed", jpegPath);
    return;
  }

  RKADK_LOGP("save snapshot[%d, %d] jpeg to %s", pstData->u32Width, pstData->u32Height, jpegPath);

  fwrite(pstData->pu8DataBuf, 1, pstData->u32DataLen, file);
  fclose(file);

  snapshotId++;
  if (snapshotId > 10)
    snapshotId = 0;
}

static RKADK_S32 DemuxerBufferFree(RKADK_VOID *opaque) {
  if (opaque) {
    free(opaque);
    opaque = NULL;
  }

  return 0;
}

static RKADK_VOID DemuxerReadVideoPacket(RKADK_VOID* pPacket) {
  int ret;
  RKADK_PLAYER_PACKET stPacket;
  DemuxerPacket *pstDemuxerPacket = (DemuxerPacket *)pPacket;

  memset(&stPacket, 0, sizeof(RKADK_PLAYER_PACKET));
  stPacket.bBypass = false;
  stPacket.pFreeCB = DemuxerBufferFree;
  stPacket.bEofFlag = pstDemuxerPacket->s8EofFlag ? true : false;
  stPacket.s32PacketSize = pstDemuxerPacket->s32PacketSize;
  stPacket.s8PacketData = (RKADK_S8 *)pstDemuxerPacket->s8PacketData;
  stPacket.s64Pts = pstDemuxerPacket->s64Pts;
  stPacket.u32Seq = pstDemuxerPacket->s32Series;

  ret = RKADK_PLAYER_SendVideoPacket(pstDemuxerPacket->ptr, &stPacket);
  if (ret)
    RKADK_LOGE("SendVideoPacket failed");
}

static RKADK_VOID DemuxerReadAudioPacket(RKADK_VOID* pPacket) {
  int ret;
  RKADK_PLAYER_PACKET stPacket;
  DemuxerPacket *pstDemuxerPacket = (DemuxerPacket *)pPacket;

  memset(&stPacket, 0, sizeof(RKADK_PLAYER_PACKET));
  stPacket.bBypass = false;
  stPacket.pFreeCB = DemuxerBufferFree;
  stPacket.bEofFlag = pstDemuxerPacket->s8EofFlag ? true : false;
  stPacket.s32PacketSize = pstDemuxerPacket->s32PacketSize;
  stPacket.s8PacketData = (RKADK_S8 *)pstDemuxerPacket->s8PacketData;
  stPacket.s64Pts = pstDemuxerPacket->s64Pts;
  stPacket.u32Seq = pstDemuxerPacket->s32Series;

  ret = RKADK_PLAYER_SendAudioPacket(pstDemuxerPacket->ptr, &stPacket);
  if (ret)
    RKADK_LOGE("SendAudioPacket failed");
}

static int DemuxerSetDataParam(RKADK_MW_PTR pPlayer, char *file,
                                          RKADK_PLAYER_CFG_S stPlayCfg) {
  int ret;
  RKADK_DEMUXER_INPUT_S stDemuxerInput;
  RKADK_DEMUXER_PARAM_S stDemuxerParam;
  RKADK_PLAYER_DATA_PARAM_S stDataParam;

  memset(&stDemuxerInput, 0, sizeof(RKADK_DEMUXER_INPUT_S));
  memset(&stDemuxerParam, 0, sizeof(RKADK_DEMUXER_PARAM_S));
  memset(&stDataParam, 0, sizeof(RKADK_PLAYER_DATA_PARAM_S));

  stDemuxerInput.ptr = pPlayer;
  stDemuxerInput.readModeFlag = DEMUXER_TYPE_PASSIVE;
  stDemuxerInput.videoEnableFlag = stPlayCfg.bEnableVideo;
  stDemuxerInput.audioEnableFlag = stPlayCfg.bEnableAudio;
  stDemuxerInput.transport = stPlayCfg.stNetStreamCfg.transport;
  stDemuxerInput.u32IoTimeout = stPlayCfg.stNetStreamCfg.u32IoTimeout;

  if (RKADK_DEMUXER_Create(&mDemuxerCfg, &stDemuxerInput)) {
    RKADK_LOGE("RKADK_DEMUXER_Create failed");
    return -1;
  }

  stDemuxerParam.pstReadPacketCallback.pfnReadVideoPacketCallback = DemuxerReadVideoPacket;
  stDemuxerParam.pstReadPacketCallback.pfnReadAudioPacketCallback = DemuxerReadAudioPacket;
  if (RKADK_DEMUXER_GetParam(mDemuxerCfg, file, &stDemuxerParam)) {
    RKADK_LOGE("RKADK_DEMUXER_GetParam failed");
    return -1;
  }

  stDataParam.pFilePath = file;
  stDataParam.bVideoExist = stPlayCfg.bEnableVideo;
  stDataParam.bAudioExist = stPlayCfg.bEnableAudio;
  if (strstr(file, "rtsp://"))
    stDataParam.bIsNetRTStream = RKADK_TRUE;

  if (stDataParam.bVideoExist) {
    if (stDemuxerParam.pVideoCodec != NULL) {
      if (!strcmp(stDemuxerParam.pVideoCodec, "h264")) {
        stDataParam.enVCodecType = RKADK_CODEC_TYPE_H264;
      } else if (!strcmp(stDemuxerParam.pVideoCodec, "h265")) {
        stDataParam.enVCodecType = RKADK_CODEC_TYPE_H265;
      } else {
        RKADK_LOGE("Unsupported audio format[%s]", stDemuxerParam.pVideoCodec);
        return -1;
      }
    }

    if (stDemuxerParam.VideoFormat == DEMUXER_VIDEO_YUV420SP_10BIT)
      stDataParam.enPixFmt = RK_FMT_YUV420SP_10BIT;
    else
      stDataParam.enPixFmt = RK_FMT_YUV420SP;

    stDataParam.u32Width = stDemuxerParam.videoWidth;
    stDataParam.u32Height = stDemuxerParam.videoHeigh;
    stDataParam.u32FrameRate = stDemuxerParam.videoAvgFrameRate;
  }

  if (stDataParam.bAudioExist) {
    if (stDemuxerParam.pAudioCodec != NULL) {
      if (!strcmp(stDemuxerParam.pAudioCodec, "mp3"))
        stDataParam.enACodecType = RKADK_CODEC_TYPE_MP3;
      else if (!strcmp(stDemuxerParam.pAudioCodec, "wav"))
        stDataParam.enACodecType = RKADK_CODEC_TYPE_PCM;
      else if (!strcmp(stDemuxerParam.pAudioCodec, "pcm_alaw"))
        stDataParam.enACodecType = RKADK_CODEC_TYPE_G711A;
      else {
        RKADK_LOGE("Unsupported audio format[%s]", stDemuxerParam.pAudioCodec);
        return -1;
      }
    }

    if (stDemuxerParam.audioFormat == 0)
      stDataParam.u32BitWidth = 8;
    else if (stDemuxerParam.audioFormat == 1)
      stDataParam.u32BitWidth = 16;
    else {
      RKADK_LOGE("Unsupported audioFormat = %d", stDemuxerParam.audioFormat);
      return -1;
    }

    stDataParam.u32SampleRate = stDemuxerParam.audioSampleRate;
    stDataParam.u32Channel = stDemuxerParam.audioChannels;
  }

  ret = RKADK_PLAYER_SetDataParam(pPlayer, &stDataParam);
  if (ret) {
    RKADK_LOGE("SetDataParam failed, ret = %d", ret);
    return -1;
  }

  return 0;
}

#ifdef OS_LINUX
int main(int argc, char *argv[]) {
  int c, ret, transport = 0;
  char *file = "/userdata/16000_2.mp3";
  RKADK_BOOL bVideoEnable = false;
  RKADK_BOOL bAudioEnable = true;
  RKADK_BOOL bBlackBackgroundEnable = false;
  RKADK_MW_PTR pPlayer = NULL;
  RKADK_S64 seekTimeInMs = 0, maxSeekTimeInMs = (RKADK_S64)pow(2, 63) / 1000;
  pthread_t getPosition = 0;
  RKADK_U32 duration = 0;
  int u32VoFormat = -1, u32SpliceMode = -1, u32IntfType = -1;
  RKADK_PLAYER_CFG_S stPlayCfg;
  int loop_count = -1, loop_duration = 0;
  RKADK_PLAYER_STATE_E enState = RKADK_PLAYER_STATE_BUTT;
  RKADK_U32 u32Waterline = 0;
  RKADK_U32 u32DecodeMode = 0;
  RKADK_U32 u32SeekTime = 3000;

  memset(&stPlayCfg, 0, sizeof(RKADK_PLAYER_CFG_S));
  param_init(&stPlayCfg.stFrmInfo);

#if defined(RK3506) || defined(RV1106_1103) || defined(RV1103B)
  stPlayCfg.stAudioCfg.pSoundCard = "hw:0,0";
#else
  stPlayCfg.stAudioCfg.pSoundCard = "default";
#endif

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
    case 'i':
      file = optarg;
      break;
    case 'x':
      stPlayCfg.stFrmInfo.u32FrmInfoX = atoi(optarg);
      break;
    case 'y':
      stPlayCfg.stFrmInfo.u32FrmInfoY = atoi(optarg);
      break;
    case 'W':
      stPlayCfg.stFrmInfo.u32DispWidth = atoi(optarg);
      break;
    case 'H':
      stPlayCfg.stFrmInfo.u32DispHeight = atoi(optarg);
      break;
    case 'r':
      stPlayCfg.stFrmInfo.u32Rotation = atoi(optarg);
      break;
    case 'F':
      stPlayCfg.stFrmInfo.stSyncInfo.u16FrameRate = atoi(optarg);
      break;
    case 'm':
      stPlayCfg.stFrmInfo.bMirror = true;
      break;
    case 'f':
      stPlayCfg.stFrmInfo.bFlip = true;
      break;
    case 'T':
      stPlayCfg.stNetStreamCfg.u32IoTimeout = atoi(optarg) * 1000;
      break;
    case 'l':
      stPlayCfg.stFrmInfo.u32VoLay = atoi(optarg);
      break;
    case 'O':
      stPlayCfg.stVdecCfg.u32FrameBufCnt = atoi(optarg);
      break;
    case 'S':
      stPlayCfg.stVdecCfg.u32StreamBufCnt = atoi(optarg);
      break;
    case 'w':
      u32Waterline = atoi(optarg);
      break;
    case 'v':
      bVideoEnable = true;
      break;
    case 'b':
      bBlackBackgroundEnable = true;
      break;
    case 'a':
      bAudioEnable = atoi(optarg);
      break;
    case 's':
      u32SpliceMode = atoi(optarg);
      break;
    case 'P':
      u32VoFormat = atoi(optarg);
      break;
    case 'I':
      u32IntfType = atoi(optarg);
      break;
    case 't':
      transport = atoi(optarg);
      break;
    case 'c':
      loop_count = atoi(optarg);
      break;
    case 'd':
      loop_duration = atoi(optarg);
      break;
    case 'D':
      stPlayCfg.bEnableThirdDemuxer = true;
      RKADK_LOGP("Enable third-party demuxer");
      break;
    case 'A':
      stPlayCfg.stAudioCfg.u32AdecBufCnt= atoi(optarg);
      break;
    case 'M':
      u32DecodeMode= atoi(optarg);
      break;
    case 'C':
      stPlayCfg.stAudioCfg.pSoundCard = optarg;
      break;
    case 'V':
      stPlayCfg.stAudioCfg.u32SpeakerVolume = atoi(optarg);
      break;
    case 'R':
      stPlayCfg.stAudioCfg.u32AoSampleRate = atoi(optarg);
      break;
    case 'N':
      stPlayCfg.stAudioCfg.u32AoChnId = atoi(optarg);
      break;
    case 'h':
    default:
      print_usage(argv[0]);
      optind = 0;
      return 0;
    }
  }
  optind = 0;

  RKADK_LOGP("#play file: %s, bVideoEnable: %d, bAudioEnable: %d",file, bVideoEnable, bAudioEnable);
  RKADK_LOGP("#video display rect[%d, %d, %d, %d], u32SpliceMode: %d, u32VoFormat: %d, fps: %d",
              stPlayCfg.stFrmInfo.u32FrmInfoX, stPlayCfg.stFrmInfo.u32FrmInfoY,
              stPlayCfg.stFrmInfo.u32DispWidth, stPlayCfg.stFrmInfo.u32DispHeight,
              u32SpliceMode, u32VoFormat, stPlayCfg.stFrmInfo.stSyncInfo.u16FrameRate);
  RKADK_LOGP("u32Waterline: %d, u32FrameBufCnt: %d, u32StreamBufCnt: %d",
              u32Waterline, stPlayCfg.stVdecCfg.u32FrameBufCnt, stPlayCfg.stVdecCfg.u32StreamBufCnt);
  RKADK_LOGP("transport: %d, u32IoTimeout: %d(us), pSoundCard: %s",
              transport, stPlayCfg.stNetStreamCfg.u32IoTimeout, stPlayCfg.stAudioCfg.pSoundCard);

  if (u32SpliceMode == 1)
    stPlayCfg.stFrmInfo.enVoSpliceMode = SPLICE_MODE_GPU;
  else if (u32SpliceMode == 2)
    stPlayCfg.stFrmInfo.enVoSpliceMode = SPLICE_MODE_BYPASS;

  if (u32VoFormat == 1)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_NV12;
  else if (u32VoFormat == 2)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_RGB565;
  else if (u32VoFormat == 3)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_RGB444;

  if (u32IntfType == 1)
    stPlayCfg.stFrmInfo.u32EnIntfType = DISPLAY_TYPE_MIPI;
  else if (u32IntfType == 2)
    stPlayCfg.stFrmInfo.u32EnIntfType = DISPLAY_TYPE_LCD;

  if (u32DecodeMode == 1)
    stPlayCfg.stVdecCfg.u32DecodeMode = RKADK_DECODE_MODE_STREAM;
  else if (u32DecodeMode == 2)
    stPlayCfg.stVdecCfg.u32DecodeMode = RKADK_DECODE_MODE_FRAME_SLICE;
  else if (u32DecodeMode == 3)
    stPlayCfg.stVdecCfg.u32DecodeMode = RKADK_DECODE_MODE_SLICE;
  else if (u32DecodeMode == 4)
    stPlayCfg.stVdecCfg.u32DecodeMode = RKADK_DECODE_MODE_COMPAT;

  signal(SIGINT, sigterm_handler);

  RKADK_MPI_SYS_Init();

  if (bAudioEnable)
    stPlayCfg.bEnableAudio = true;
  if (bVideoEnable)
    stPlayCfg.bEnableVideo = true;
  if (bBlackBackgroundEnable)
    stPlayCfg.bEnableBlackBackground = true;

  stPlayCfg.pfnPlayerCallback = PlayerEventFnTest;

  //for rtsp
  if (transport == 1)
    stPlayCfg.stNetStreamCfg.transport = "tcp";
  else
    stPlayCfg.stNetStreamCfg.transport = "udp";

  stPlayCfg.stSnapshotCfg.u32VencChn = 15;
  stPlayCfg.stSnapshotCfg.pfnDataCallback = SnapshotDataRecv;
  if (RKADK_PARAM_GetCommParam(RKADK_PARAM_TYPE_VOLUME, &stPlayCfg.stAudioCfg.u32SpeakerVolume))
    stPlayCfg.stAudioCfg.u32SpeakerVolume = 70;

  if (RKADK_PLAYER_Create(&pPlayer, &stPlayCfg)) {
    RKADK_LOGE("RKADK_PLAYER_Create failed");
    return -1;
  }

  if (u32Waterline > 0)
    RKADK_PLAYER_SetVdecWaterline(pPlayer, u32Waterline);

  if (stPlayCfg.bEnableThirdDemuxer) {
    ret = DemuxerSetDataParam(pPlayer, file, stPlayCfg);
    if (ret) {
      RKADK_LOGE("DemuxerSetDataParam failed");
      goto __EXIT;
    }
  } else {
    ret = RKADK_PLAYER_SetDataSource(pPlayer, file);
    if (ret) {
      RKADK_LOGE("SetDataSource failed, ret = %d", ret);
      goto __EXIT;
    }
  }

  ret = RKADK_PLAYER_Prepare(pPlayer);
  if (ret) {
    RKADK_LOGE("Prepare failed, ret = %d", ret);
    goto __EXIT;
  }

  RKADK_PLAYER_GetDuration(pPlayer, &duration);
  RKADK_LOGP("file duration: %d ms", duration);

  if (duration > 0) {
    if (loop_duration <= 0 || loop_duration > duration / 1000)
      loop_duration = duration / 1000 + 1; //ms to m
  }

  RKADK_LOGP("loop_count: %d, duration:%d(ms), loop_duration: %d(s)", loop_count, duration, loop_duration);

  if (stPlayCfg.bEnableThirdDemuxer) {
    ret = RKADK_DEMUXER_ReadPacketStart(mDemuxerCfg, 0);
    if (ret != 0) {
      RKADK_LOGE("RKADK_DEMUXER_ReadPacketStart failed");
      goto __EXIT;
    }
  }

  ret = RKADK_PLAYER_Play(pPlayer);
  if (ret) {
    RKADK_LOGE("Play failed, ret = %d", ret);
    return -1;
  }
  loop_count--;

  pthread_create(&getPosition, 0, GetPosition, pPlayer);

  char cmd[64];
  printf("\n#Usage: input 'quit' to exit programe!\n"
         "peress any other key to capture one picture to file\n");
  while (!is_quit) {
    if (loop_count >= 0 && !stPlayCfg.bEnableThirdDemuxer) {
      RKADK_PLAYER_GetPlayStatus(pPlayer, &enState);
      if (loop_duration > 0) {
        sleep(loop_duration);
      } else if (enState != RKADK_PLAYER_STATE_EOF) {
        sleep(1);
        continue;
      }

      RKADK_LOGP("seek + replay test, loop_count: %d", loop_count);
      if (loop_count == 0) {
        RKADK_LOGP("loop play end!");
        is_quit = true;
        goto __EXIT;
      }

      ret = RKADK_PLAYER_Stop(pPlayer);
      if (ret) {
        RKADK_LOGE("Stop failed");
      }

      ret = RKADK_PLAYER_Destroy(pPlayer);
      if (ret) {
        RKADK_LOGE("Destroy failed");
        goto __EXIT;
      }

      if (getPosition) {
        is_destroy = true;
        pthread_join(getPosition, RKADK_NULL);
        getPosition = 0;
      }

      pPlayer = NULL;
      ret = RKADK_PLAYER_Create(&pPlayer, &stPlayCfg);
      if (ret) {
        RKADK_LOGE("Create failed");
        goto __EXIT;
      }

      ret = RKADK_PLAYER_SetDataSource(pPlayer, file);
      if (ret) {
        RKADK_LOGE("SetDataSource failed, ret = %d", ret);
        break;
      }

      ret = RKADK_PLAYER_Prepare(pPlayer);
      if (ret) {
        RKADK_LOGE("Prepare failed, ret = %d", ret);
        break;
      }
      RKADK_PLAYER_GetDuration(pPlayer, &duration);

      ret = RKADK_PLAYER_Play(pPlayer);
      if (ret) {
        RKADK_LOGE("Play failed, ret = %d", ret);
        break;
      }

      is_destroy = false;
      pthread_create(&getPosition, 0, GetPosition, pPlayer);
      if ((loop_count % 3) == 0) {
        u32SeekTime = 9200;
      } else if ((loop_count % 2) == 0) {
        u32SeekTime = 15800;
        usleep(5 * 1000);
      } else {
        u32SeekTime = 3000;
      }

      RKADK_PLAYER_Seek(pPlayer, u32SeekTime);
      if ((loop_count % 2) == 0)
        sleep(3);

      loop_count--;
    } else {
      fgets(cmd, sizeof(cmd), stdin);
      RKADK_LOGP("#Input cmd: %s", cmd);
      if (strstr(cmd, "quit") || is_quit) {
        RKADK_LOGP("#Get 'quit' cmd!");
        if (!is_quit)
          is_quit = true;

        break;
      } else if (strstr(cmd, "pause")) {
        ret = RKADK_PLAYER_Pause(pPlayer);
        if (ret) {
          RKADK_LOGE("Pause failed, ret = %d", ret);
          break;
        }
      } else if (strstr(cmd, "resume")) {
        ret = RKADK_PLAYER_Play(pPlayer);
        if (ret) {
          RKADK_LOGE("Play failed, ret = %d", ret);
          break;
        }
      } else if (strstr(cmd, "replay")) {
        if (stPlayCfg.bEnableThirdDemuxer) {
          //for exit ao pause
          RKADK_PLAYER_GetPlayStatus(pPlayer, &enState);
          if (enState == RKADK_PLAYER_STATE_PAUSE)
            RKADK_PLAYER_Play(pPlayer);

          RKADK_DEMUXER_ReadPacketStop(mDemuxerCfg);
        }

        RKADK_PLAYER_Stop(pPlayer);

        if (stPlayCfg.bEnableThirdDemuxer) {
          ret = DemuxerSetDataParam(pPlayer, file, stPlayCfg);
          if (ret) {
            RKADK_LOGE("DemuxerSetDataParam failed");
            break;
          }
        } else {
          ret = RKADK_PLAYER_SetDataSource(pPlayer, file);
          if (ret) {
            RKADK_LOGE("SetDataSource failed, ret = %d", ret);
            break;
          }
        }

        ret = RKADK_PLAYER_Prepare(pPlayer);
        if (ret) {
          RKADK_LOGE("Prepare failed, ret = %d", ret);
          break;
        }

        if (stPlayCfg.bEnableThirdDemuxer) {
          ret = RKADK_DEMUXER_ReadPacketStart(mDemuxerCfg, 0);
          if (ret != 0) {
            RKADK_LOGE("RKADK_DEMUXER_ReadPacketStart failed");
            break;
          }
        }

        ret = RKADK_PLAYER_Play(pPlayer);
        if (ret) {
          RKADK_LOGE("Play failed, ret = %d", ret);
          break;
        }

        RKADK_PLAYER_GetDuration(pPlayer, &duration);
      } else if (strstr(cmd, "seek")) {
        if (stPlayCfg.bEnableThirdDemuxer)
          break;

        fgets(cmd, sizeof(cmd), stdin);
        seekTimeInMs = atoi(cmd);
        if ((seekTimeInMs < 0) || (seekTimeInMs > maxSeekTimeInMs)) {
          RKADK_LOGE("seekTimeInMs(%lld) is out of range", seekTimeInMs);
          break;
        }

        RKADK_PLAYER_Seek(pPlayer, seekTimeInMs);
      } else if (strstr(cmd, "snap")) {
        RKADK_PLAYER_Snapshot(pPlayer);
      }

      usleep(500000);
    }
  }

__EXIT:
  if (stPlayCfg.bEnableThirdDemuxer) {
    //for exit ao pause
    RKADK_PLAYER_GetPlayStatus(pPlayer, &enState);
    if (enState == RKADK_PLAYER_STATE_PAUSE)
      RKADK_PLAYER_Play(pPlayer);

    RKADK_DEMUXER_ReadPacketStop(mDemuxerCfg);

    if (mDemuxerCfg)
      RKADK_DEMUXER_Destroy(&mDemuxerCfg);
  }

  RKADK_PLAYER_Destroy(pPlayer);

  if (getPosition)
    pthread_join(getPosition, RKADK_NULL);

  pPlayer = NULL;
  RKADK_MPI_SYS_Exit();
  return 0;
}
#elif defined(OS_RTT)
#include <finsh.h>

typedef struct {
  int argc;
  const char **argv;
} RKADK_ARG;

static void playerMain(void *arg) {
  int c, ret, transport = 0;
  char *file = "16000_2.mp3";
  RKADK_BOOL bVideoEnable = false;
  RKADK_BOOL bAudioEnable = true;
  RKADK_BOOL bBlackBackgroundEnable = false;
  RKADK_ARG *argCfg = (RKADK_ARG *)arg;
  RKADK_MW_PTR pPlayer = NULL;
  RKADK_S64 seekTimeInMs = 0, maxSeekTimeInMs = (RKADK_S64)pow(2, 63) / 1000;
  pthread_t getPosition = 0;
  RKADK_U32 duration = 0;
  int u32VoFormat = -1, u32SpliceMode = -1, u32IntfType = -1;
  RKADK_PLAYER_CFG_S stPlayCfg;
  int loop_count = 1, loop_duration = 0;
  RKADK_PLAYER_STATE_E enState = RKADK_PLAYER_STATE_BUTT;
  RKADK_U32 u32Waterline = 0;
  rt_thread_t tid = RT_NULL;

  memset(&stPlayCfg, 0, sizeof(RKADK_PLAYER_CFG_S));
  param_init(&stPlayCfg.stFrmInfo);
  if (RKADK_PARAM_GetCommParam(RKADK_PARAM_TYPE_VOLUME, &stPlayCfg.stAudioCfg.u32SpeakerVolume))
    stPlayCfg.stAudioCfg.u32SpeakerVolume = 70;

#if defined(RK3506)
  stPlayCfg.stAudioCfg.pSoundCard = "es8388p";
#else
  stPlayCfg.stAudioCfg.pSoundCard = "acodecp";
#endif

  while ((c = getopt(argCfg->argc, argCfg->argv, optstr)) != -1) {
    switch (c) {
    case 'i':
      file = optarg;
      break;
    case 'x':
      stPlayCfg.stFrmInfo.u32FrmInfoX = atoi(optarg);
      break;
    case 'y':
      stPlayCfg.stFrmInfo.u32FrmInfoY = atoi(optarg);
      break;
    case 'W':
      stPlayCfg.stFrmInfo.u32DispWidth = atoi(optarg);
      break;
    case 'H':
      stPlayCfg.stFrmInfo.u32DispHeight = atoi(optarg);
      break;
    case 'r':
      stPlayCfg.stFrmInfo.u32Rotation = atoi(optarg);
      break;
    case 'F':
      stPlayCfg.stFrmInfo.stSyncInfo.u16FrameRate = atoi(optarg);
      break;
    case 'm':
      stPlayCfg.stFrmInfo.bMirror = true;
      break;
    case 'f':
      stPlayCfg.stFrmInfo.bFlip = true;
      break;
    case 'T':
      stPlayCfg.stNetStreamCfg.u32IoTimeout = atoi(optarg) * 1000;
      break;
    case 'l':
      stPlayCfg.stFrmInfo.u32VoLay = atoi(optarg);
      break;
    case 'O':
      stPlayCfg.stVdecCfg.u32FrameBufCnt = atoi(optarg);
      break;
    case 'S':
      stPlayCfg.stVdecCfg.u32StreamBufCnt = atoi(optarg);
      break;
    case 'w':
      u32Waterline = atoi(optarg);
      break;
    case 'v':
      bVideoEnable = true;
      break;
    case 'b':
      bBlackBackgroundEnable = true;
      break;
    case 'a':
      bAudioEnable = atoi(optarg);
      break;
    case 's':
      u32SpliceMode = atoi(optarg);
      break;
    case 'P':
      u32VoFormat = atoi(optarg);
      break;
    case 'I':
      u32IntfType = atoi(optarg);
      break;
    case 't':
      transport = atoi(optarg);
      break;
    case 'c':
      loop_count = atoi(optarg);
      break;
    case 'd':
      loop_duration = atoi(optarg);
      break;
    case 'D':
      stPlayCfg.bEnableThirdDemuxer = true;
      RKADK_LOGD("Enable third-party demuxer");
      break;
    case 'A':
      stPlayCfg.stAudioCfg.u32AdecBufCnt= atoi(optarg);
      break;
    case 'C':
      stPlayCfg.stAudioCfg.pSoundCard = optarg;
      break;
    case 'V':
      stPlayCfg.stAudioCfg.u32SpeakerVolume = atoi(optarg);
      break;
    case 'R':
      stPlayCfg.stAudioCfg.u32AoSampleRate = atoi(optarg);
      break;
    case 'h':
    default:
      print_usage(argCfg->argv[0]);
      optind = 0;
      return 0;
    }
  }
  optind = 0;

  RKADK_LOGI("#play file: %s, bVideoEnable: %d, bAudioEnable: %d",file, bVideoEnable, bAudioEnable);
  RKADK_LOGI("#video display rect[%d, %d, %d, %d], u32SpliceMode: %d, u32VoFormat: %d, fps: %d",
              stPlayCfg.stFrmInfo.u32FrmInfoX, stPlayCfg.stFrmInfo.u32FrmInfoY,
              stPlayCfg.stFrmInfo.u32DispWidth, stPlayCfg.stFrmInfo.u32DispHeight,
              u32SpliceMode, u32VoFormat, stPlayCfg.stFrmInfo.stSyncInfo.u16FrameRate);
  RKADK_LOGI("u32Waterline: %d, u32FrameBufCnt: %d, u32StreamBufCnt: %d",
              u32Waterline, stPlayCfg.stVdecCfg.u32FrameBufCnt, stPlayCfg.stVdecCfg.u32StreamBufCnt);
  RKADK_LOGI("transport: %d, u32IoTimeout: %d(us), pSoundCard: %s",
              transport, stPlayCfg.stNetStreamCfg.u32IoTimeout, stPlayCfg.stAudioCfg.pSoundCard);
  RKADK_LOGI("hact: %d, vact: %d, speakerVolume: %d",
              stPlayCfg.stFrmInfo.stSyncInfo.u16Hact, stPlayCfg.stFrmInfo.stSyncInfo.u16Vact, stPlayCfg.stAudioCfg.u32SpeakerVolume);

  if (u32SpliceMode == 1)
    stPlayCfg.stFrmInfo.enVoSpliceMode = SPLICE_MODE_GPU;
  else if (u32SpliceMode == 2)
    stPlayCfg.stFrmInfo.enVoSpliceMode = SPLICE_MODE_BYPASS;

  if (u32VoFormat == 1)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_NV12;
  else if (u32VoFormat == 2)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_RGB565;
  else if (u32VoFormat == 3)
    stPlayCfg.stFrmInfo.u32VoFormat = VO_FORMAT_RGB444;

  if (u32IntfType == 1)
    stPlayCfg.stFrmInfo.u32EnIntfType = DISPLAY_TYPE_MIPI;
  else if (u32IntfType == 2)
    stPlayCfg.stFrmInfo.u32EnIntfType = DISPLAY_TYPE_LCD;

  signal(SIGINT, sigterm_handler);

  RKADK_MPI_SYS_Init();

  if (bAudioEnable)
    stPlayCfg.bEnableAudio = true;
  if (bVideoEnable)
    stPlayCfg.bEnableVideo = true;
  if (bBlackBackgroundEnable)
    stPlayCfg.bEnableBlackBackground = true;

  stPlayCfg.pfnPlayerCallback = PlayerEventFnTest;

  //for rtsp
  if (transport == 1)
    stPlayCfg.stNetStreamCfg.transport = "tcp";
  else
    stPlayCfg.stNetStreamCfg.transport = "udp";

  stPlayCfg.stSnapshotCfg.u32VencChn = 15;
  stPlayCfg.stSnapshotCfg.pfnDataCallback = SnapshotDataRecv;

  while (loop_count) {
    if (RKADK_PLAYER_Create(&pPlayer, &stPlayCfg)) {
      RKADK_LOGE("RKADK_PLAYER_Create failed");
      goto __EXIT;
    }

    if (u32Waterline > 0)
      RKADK_PLAYER_SetVdecWaterline(pPlayer, u32Waterline);

    if (stPlayCfg.bEnableThirdDemuxer) {
      ret = DemuxerSetDataParam(pPlayer, file, stPlayCfg);
      if (ret) {
        RKADK_LOGE("DemuxerSetDataParam failed");
        goto __EXIT;
      }
    } else {
      ret = RKADK_PLAYER_SetDataSource(pPlayer, file);
      if (ret) {
        RKADK_LOGE("SetDataSource failed, ret = %d", ret);
        goto __EXIT;
      }
    }

    RKADK_PLAYER_GetDuration(pPlayer, &duration);
    RKADK_LOGD("file duration: %d ms", duration);

    if (duration > 0) {
      if (loop_duration <= 0 || loop_duration > duration / 1000)
        loop_duration = duration / 1000 + 1; //ms to m
    }

    RKADK_LOGE("loop_count: %d, duration:%d(ms), loop_duration: %d(s)", loop_count, duration, loop_duration);

    ret = RKADK_PLAYER_Prepare(pPlayer);
    if (ret) {
      RKADK_LOGE("Prepare failed, ret = %d", ret);
      goto __EXIT;
    }

    if (stPlayCfg.bEnableThirdDemuxer) {
      ret = RKADK_DEMUXER_ReadPacketStart(mDemuxerCfg, 0);
      if (ret != 0) {
        RKADK_LOGE("RKADK_DEMUXER_ReadPacketStart failed");
        goto __EXIT;
      }
    }

    ret = RKADK_PLAYER_Play(pPlayer);
    if (ret) {
      RKADK_LOGE("Play failed, ret = %d", ret);
      return -1;
    }
    loop_count--;

    is_quit = false;
    RKADK_ARG argCfg;
    tid = rt_thread_create("GetPosition", GetPosition, pPlayer, 2048, 16, 10);
    if (tid)
        rt_thread_startup(tid);

    char cmd[64];
    printf("\n#Usage: input 'quit' to exit programe!\n"
          "peress any other key to capture one picture to file\n");

    while (!is_quit) {
      rkos_msleep(1000);
    }

    RKADK_PLAYER_Destroy(pPlayer);
    pPlayer = NULL;
  }

__EXIT:
  if (stPlayCfg.bEnableThirdDemuxer) {
    //for exit ao pause
    RKADK_PLAYER_GetPlayStatus(pPlayer, &enState);
    if (enState == RKADK_PLAYER_STATE_PAUSE)
      RKADK_PLAYER_Play(pPlayer);

    RKADK_DEMUXER_ReadPacketStop(mDemuxerCfg);

    if (mDemuxerCfg)
      RKADK_DEMUXER_Destroy(&mDemuxerCfg);
  }

  is_quit = true;
  if (pPlayer) {
    RKADK_PLAYER_Destroy(pPlayer);
    pPlayer = NULL;
  }

  RKADK_MPI_SYS_Exit();
  return;
}

int rkadk_player_test_enter(int argc, char *argv[]) {
  rt_thread_t tid = RT_NULL;
  RKADK_ARG argCfg;
  argCfg.argc = argc;
  argCfg.argv = argv;
  tid = rt_thread_create("playerMain", playerMain, &argCfg, 90112, 16, 10);

  if (tid)
      rt_thread_startup(tid);

  while (tid->stat != RT_THREAD_CLOSE) {
    rkos_msleep(1000);
  }

  rkos_msleep(10);
  is_quit = false;
  return 0;
}

struct _main_param
{
  int argc;
  char **argv;
  int (* pmain)(int argc, char **argv);
};

static int thread_excute_main(void *arg)
{
    struct _main_param *p;
    p = (struct _main_param *)arg;
    rkos_kthread_set_name(p->argv[0]);
    p->pmain(p->argc, p->argv);
    for (int i = 1; i < p->argc; i++)
        rkos_vfree(p->argv[i]);
    rkos_vfree(p->argv);
    rkos_vfree(p);
    return 0;
}

static int argparse_excute_main(int argc, char **argv, int (* pMain)(int argc, char **argv))
{
  if (argv[argc - 1][0] != '&')
  {
    if (pMain)
      return pMain(argc, argv);
  }
  else
  {
    char **argv_tmp = rkos_vcalloc(1, sizeof(char *) * argc);
    for (int i = 0; i < (argc - 1); i++) {
      argv_tmp[i] = rkos_vcalloc(1, strlen(argv[i]));
      memcpy(argv_tmp[i], argv[i], strlen(argv[i]));
      argv_tmp[i][strlen(argv[i])] = 0;
    }

    struct _main_param *p = rkos_vcalloc(1, sizeof(struct _main_param));
    p->argc = argc;
    p->argv = argv_tmp;
    p->pmain = pMain;
    rkos_kthread_create(thread_excute_main, p, argv_tmp[0], 4096, 0);
  }
  return 0;
}

int rkadk_player_test(int argc, char **argv)
{
  argparse_excute_main(argc, argv, rkadk_player_test_enter);
  return 0;
}

MSH_CMD_EXPORT(rkadk_player_test, rkadk player module test);
#endif
