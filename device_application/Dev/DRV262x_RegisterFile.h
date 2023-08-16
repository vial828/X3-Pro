/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#ifndef DRV262x_REGISTERFILE_H_
#define DRV262x_REGISTERFILE_H_
//#############################################################################
//
//! \file   DRV262x_RegisterFile.h
//!
//! \brief The DRV262x_RegisterFile header file contains an enumeration of the
//!        DRV262x register file, as well as bit field definitions for each
//!        register where appropriate.  This can be thought of as the 
//!        definition of the DRV262x I2C register map.
//
//  Group:          LPAA
//  Target Devices: MSP430FR2xx/4xx/5xx/6xx
//
//  (C) Copyright 2015, Texas Instruments, Inc.
//#############################################################################
// TI Release: TBD
// Release Date: TBD
//#############################################################################

//*****************************************************************************
// Includes
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
// Definitions
//*****************************************************************************

//! \brief A helpful reminder that the bit field is read only.
//!
#define __R

//! \brief A helpful reminder that the bit field is write only.
//!
#define __W

//! \brief A helpful reminder that the bit field is read/write.
//!
#define __RW

//*****************************************************************************
// Type Definitions
//*****************************************************************************

//! \brief An enumeration of the tDRV2605 I2C register file.
//!
typedef enum
{
	DRV262x_reg_chipId = 0x00,
	DRV262x_reg_status,
	DRV262x_reg_intzMask,
	DRV262x_reg_diagZResult,
	DRV262x_reg_vbat,
	DRV262x_reg_lraPeriodMSB,
	DRV262x_reg_lraPeriodLSB,
	DRV262x_reg_ctrl1,//0x07
	DRV262x_reg_ctrl2,//0x08
	DRV262x_reg_vbatMonitor,
	DRV262x_reg_vbatThreshold1,
	DRV262x_reg_vbatThreshold2,
	DRV262x_reg_go,
	DRV262x_reg_libCtrl,
	DRV262x_reg_rtpInput,//0x0E
	DRV262x_reg_waveformSeq1,
	DRV262x_reg_waveformSeq2,
	DRV262x_reg_waveformSeq3,
	DRV262x_reg_waveformSeq4,
	DRV262x_reg_waveformSeq5,
	DRV262x_reg_waveformSeq6,
	DRV262x_reg_waveformSeq7,
	DRV262x_reg_waveformSeq8,
	DRV262x_reg_waveformSeqLoop1,
	DRV262x_reg_waveformSeqLoop2,
	DRV262x_reg_waveSeqMainLoop,
	DRV262x_reg_odt,
	DRV262x_reg_spt,
	DRV262x_reg_snt,
	DRV262x_reg_brt,
	DRV262x_reg_null1,
	DRV262x_reg_ratedVoltage,//0x1F
	DRV262x_reg_overdriveVoltage,//0x20
	DRV262x_reg_aCalCompResult,//0x21
	DRV262x_reg_aCalBEMFResult,//0x22
	DRV262x_reg_feedbackControl,//0x23
	DRV262x_reg_ratedVoltageClamp,
	DRV262x_reg_overdriveVoltLvl1,
	DRV262x_reg_overdriveVoltLvl2,
	DRV262x_reg_ctrl3,
	DRV262x_reg_ctrl4,
	DRV262x_reg_ctrl5,
	DRV262x_reg_ctrl6,//0x2A
	DRV262x_reg_null2,
	DRV262x_reg_ctrl7,
	DRV262x_reg_null3,
	DRV262x_reg_LRAOLPeriodMSB,
	DRV262x_reg_LRAOLPeriodLSB,
	DRV262x_reg_currentCoefficient
} DRV262x_Register_t;

//! \brief CHIP ID REGISTER (0x00) (DRV262x_reg_chipId)
//!
typedef union {
    __R  uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __R  uint8_t bDeviceRevision      :	4;  /* Device Revision*/
      __R  uint8_t bDeviceID            :   4;  /* Device ID */
    } b;
} DRV262x_ChipIdReg_t;

//! \brief Device ID options
//!
typedef enum {
	DRV262x_deviceID_DRV2624,
	DRV262x_deviceID_DRV2625,
} DRV262x_DeviceID_t;

//! \brief STATUS REGISTER (0x01) (DRV262x_reg_status)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __R uint8_t bOverCurrent          :   1;  /* Over Current Flag */
      __R uint8_t bOverTemp             :   1;  /* Over Temperature Flag */
      __R uint8_t bUVLO                 :   1;  /* Under Voltage Flag */
      __R uint8_t bPreocessDone         :   1;  /* Process Done Flag */
      __R uint8_t bPreogramError        :   1;  /* Programming Error Flag */
      __R uint8_t bRESERVED0            :   2;  /* Reserved */
      __R uint8_t bDiagResult           :   1;  /* Diagnostic Results */
    } b;
} DRV262x_StatusReg_t;

//! \brief INTERRUPT MASK REGISTER (0x02) (DRV262x_reg_intzMask)
//!
typedef union {
    __RW uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __RW uint8_t bInterruptMask       :	5;  /* Interrupt Masks */
      __R  uint8_t bRESERVED0           :	3;  /* Reserved */
    } b;
} DRV262x_IntzMaskReg_t;

//! \brief Interrupt Masks options
//!
typedef enum {
    DRV262x_interruptMask_overCurrent,
    DRV262x_interruptMask_overTemp,
    DRV262x_interruptMask_underVoltage,
    DRV262x_interruptMask_preocessDone,
    DRV262x_interruptMask_programError
} DRV262x_InterruptMaskReg_t;

//! \brief DIAGNOSTIC RESULTS REGISTER (0x03) (DRV262x_reg_diagZResult)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __R uint8_t bDiagZResult          :   8;  /* Impedance Measurement of Actuator */
    } b;
} DRV262x_DiagZResultReg_t;

//! \brief BATTERY MONITOR REGISTER (0x04) (DRV262x_reg_vbat)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __R uint8_t bVbat                 :   8;  /* Battery Voltage */
    } b;
} DRV262x_VbatReg_t;

//! \brief LRA Period [8:9] (0x05) (DRV262x_reg_lraPeriodMSB)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bLRAPeriodMSB       :   2;  /* LRA Period[8:9] */
        __R uint8_t bRESERVED0          :   6;  /* Reserved */
    } b;
} DRV262x_LRAPeriodMSBReg_t;

//! \brief LRA Period [7:0] (0x06) (DRV262x_reg_lraPeriodLSB)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bLRAPeriodLSB       :   8;  /* LRA Period[7:0] */
    } b;
} DRV262x_LRAPeriodLSBReg_t;

//! \brief Ctrl 1 (0x07) (DRV262x_reg_ctrl1)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bMode              :   2;  /* Mode */
        __RW uint8_t bTrigPin       :   2;  /* TRIG pin Function */
        __RW uint8_t bLinRegCompSel     :   2;  /* LinReg Compensation Select */
        __RW uint8_t bLRAPeriodAvgDis   :   1;  /* LRA Period Average Disable */
        __RW uint8_t bI2CBroadcastEn    :   1;  /* I2C Broadcast Enable */
    } b;
} DRV262x_Ctrl1Reg_t;

typedef enum {
    DRV262x_linRegCompSel_0,
    DRV262x_linRegCompSel_2,
    DRV262x_linRegCompSel_4,
    DRV262x_linRegCompSel_5,
} DRV262x_LinRegCompSel_t;

typedef enum {
    DRV262x_trigPin_extPulse,
    DRV262x_trigPin_extLevel,
    DRV262x_trigPin_interrupt,
    DRV262x_trigPin_reserved,
} DRV262x_TrigPin_t;

typedef enum {
    DRV262x_mode_RTP,
    DRV262x_mode_waveformSeq,
    DRV262x_mode_diagnostics,
    DRV262x_mode_autocal,
} DRV262x_Mode_t;

//! \brief Ctrl 2 (0x08) (DRV262x_reg_ctrl2)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __R  uint8_t bRESERVED0         :   2;  /* Reserved */
        __RW uint8_t bInpuitSlopeCheck  :   1;  /* Input Slope Check */
        __RW uint8_t bAutoBrkIntoStdby  :   1;  /* Auto-Brake Into Standby */
        __RW uint8_t bAutoBrkOpenLoop   :   1;  /* Auto Loop Brake Enabled for OL */
        __RW uint8_t bHybridLoop        :   1;  /* Hybrid Loop Enabled */
        __RW uint8_t bOLnCL             :   1;  /* OpenLoop = 1 ClosedLoop =0 */
        __RW uint8_t bLRAnERM           :   1;  /* LRA = 1 ERM =0 */
    } b;
} DRV262x_Ctrl2Reg_t;

//! \brief Actuator options
//!
typedef enum
{
    DRV262x_actuator_ERM = 0,
    DRV262x_actuator_LRA
} DRV262x_Actuator;

typedef enum
{
    DRV262x_LoopType_ClosedLoop = 0,
    DRV262x_LoopType_OpenLoop
} DRV262x_LoopType;

//! \brief Battery Monitor and UVLO (0x09) (DRV262x_reg_vbatMonitor)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bUVLOThreshold     :   3;  /* UVLO Threshold */
        __R  uint8_t bRESERVED0         :   3;  /* OpenLoop = 1 ClosedLoop =0 */
        __RW uint8_t bBatLifeExtLvl     :   2;  /* Battery Life Monitor 1 & 2 */
    } b;
} DRV262x_VBatMonitorReg_t;

typedef enum {
    DRV262x_batLifeExtLvl_disbaled,
    DRV262x_batLifeExtLvl_lvl1,
    DRV262x_batLifeExtLvl_lvl2,
    DRV262x_batLifeExtLvl_reserved
} DRV262x_BatLifeExtLvl_t;

typedef enum {
    DRV262x_UVLOThreshold_2p5,
    DRV262x_UVLOThreshold_2p6,
    DRV262x_UVLOThreshold_2p7,
    DRV262x_UVLOThreshold_2p8,
    DRV262x_UVLOThreshold_2p9,
    DRV262x_UVLOThreshold_3p0,
    DRV262x_UVLOThreshold_3p1,
    DRV262x_UVLOThreshold_3p2,
} DRV262x_UVLOThreshold_t;

//! \brief Battery Life Threshold Level 1 (0x0A) (DRV262x_reg_vbatThreshold1)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bBatLifeExtLvl1    :   8;  /* Battery Life Threshold 1 */
    } b;
} DRV262x_VBatThreshold1Reg_t;

//! \brief Battery Life Threshold Level 2 (0x0B) (DRV262x_reg_vbatThreshold2)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bBatLifeExtLvl2    :   8;  /* Battery Life Threshold 2 */
    } b;
} DRV262x_VBatThreshold2Reg_t;

//! \brief Go Bit (0x0C) (DRV262x_reg_go)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bGo                :   1;  /* Go Bit */
        __R  uint8_t bRESERVED0         :   7;  /* Reserved */
    } b;
} DRV262x_GoReg_t;

//! \brief Library Control (0x0D) (DRV262x_reg_libCtrl)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bDigMemGain        :   2;  /* Gain Setting for Memory */
        __R  uint8_t bRESERVED          :   3;  /* Reserved */
        __RW uint8_t bPlaybackInterval  :   1;  /* Playback Interval */
        __RW uint8_t bLibSel            :   1;  /* Library Select (1=ERM) */
        __RW uint8_t bLibEnabled        :   1;  /* Library Enable */
    } b;
} DRV262x_LibCtrlReg_t;

typedef enum {
    DRV262x_digMemGain_100,
    DRV262x_digMemGain_75,
    DRV262x_digMemGain_50,
    DRV262x_digMemGain_25,
} DRV262x_DigMemGain_t;

typedef enum {
    DRV262x_libSel_LRA,
    DRV262x_libSel_ERM,
} DRV262x_LibSel_t;

typedef enum {
    DRV262x_PlaybackInterval_5ms,
    DRV262x_PlaybackInterval_1ms,
} DRV262x_PlaybackInterval_t;

//! \brief Real-Time Playback (0x0E) (DRV262x_reg_rtpInput)
//!
//! Insert RTP Data directly into the data field at this address
//! RTP bit is a signed register
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bRTPInput          :   8;  /* Real-Time Playback Value */
    } b;
} DRV262x_RTPInputReg_t;

//! \brief WAVEFORM SEQUENCE REGISTERS  (0x0F to 0x16) (eWaveformSequenceXReg)
//!
typedef union {
    __RW uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
      __RW uint8_t bParam              :    7;  /* Effect or Delay */
      __RW uint8_t bDelay              :    1;  /* Set as Delay */
    } b;
} DRV262x_WaveformSeqReg_t;

//! \brief Waveform Seq Loop 1-4 (0x17) (DRV262x_reg_waveformSeqLoop1)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bWaveformSeqLoop1  :   2;  /* Waveform Seq Loop 1 */
        __RW uint8_t bWaveformSeqLoop2  :   2;  /* Waveform Seq Loop 2 */
        __RW uint8_t bWaveformSeqLoop3  :   2;  /* Waveform Seq Loop 3 */
        __RW uint8_t bWaveformSeqLoop4  :   2;  /* Waveform Seq Loop 4 */
    } b;
} DRV262x_WaveformSeqLoop1Reg_t;

//! \brief Waveform Seq Loop 5-8 (0x18) (DRV262x_reg_waveformSeqLoop2)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bWaveformSeqLoop5  :   2;  /* Waveform Seq Loop 5 */
        __RW uint8_t bWaveformSeqLoop6  :   2;  /* Waveform Seq Loop 6 */
        __RW uint8_t bWaveformSeqLoop7  :   2;  /* Waveform Seq Loop 7 */
        __RW uint8_t bWaveformSeqLoop8  :   2;  /* Waveform Seq Loop 8 */
    } b;
} DRV262x_WaveformSeqLoop2Reg_t;

//! \brief Main Waveform Seq Loop (0x19) (DRV262x_reg_waveSeqMainLoop)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bWaveSeqMainLoop   :   3;  /* Main Waveform Seq Loop */
        __R  uint8_t bRESERVED          :   5;  /* Reserved */
    } b;
} DRV262x_WaveSeqMainLoopReg_t;

//! \brief Overdrive Time Offest (0x1A) (DRV262x_reg_odt)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bODT               :   8;  /* Overdrive Time Offest */
    } b;
} DRV262x_ODTReg_t;

//! \brief Sustain Positive Time (0x1B) (DRV262x_reg_spt)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bSPT               :   8;  /* Sustain Positive Portion */
    } b;
} DRV262x_SPTReg_t;

//! \brief Sustain Negative Time (0x1C) (DRV262x_reg_snt)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bSNT               :   8;  /* Sustain Positive Portion */
    } b;
} DRV262x_SNTReg_t;

//! \brief Braking Time Offset (0x1D) (DRV262x_reg_brt)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bBRT               :   8;  /* Braking Time Offset */
    } b;
} DRV262x_BRTReg_t;

//! \brief Reserved (0x1E) (DRV262x_reg_null1)
//!
typedef union {
    __R uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bRESERVED           :   8;  /* Reserved */
    } b;
} DRV262x_Null1Reg_t;

//! \brief Rated Voltage (0x1F) (DRV262x_reg_ratedVoltage)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bRatedVoltage      :   8;  /* Rated Voltage */
    } b;
} DRV262x_RatedVoltageReg_t;

//! \brief Overdrive Clamp Voltage (0x20) (DRV262x_reg_overdriveVoltage)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bOverdriveVoltage  :   8;  /* Overdrive Clamp Voltage */
    } b;
} DRV262x_OverdriveVoltageReg_t;

//! \brief Auto-Cal Voltage Compensation (0x21) (DRV262x_reg_aCalCompResult)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bACalCompResult    :   8;  /* Auto-Cal Voltage Compensation */
    } b;
} DRV262x_ACalCompResultReg_t;

//! \brief Auto-Cal BEMF Compensation (0x22) (DRV262x_reg_aCalBEMFResult)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bACalBEMFResult    :   8;  /* Auto-Cal BEMF Compensation */
    } b;
} DRV262x_ACalBEMFResultReg_t;

//! \brief Feedback Control (0x23) (DRV262x_reg_feedbackControl)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bBEMFGain          :   2;  /* BEMF Gain Result */
        __RW uint8_t bLoopGain          :   2;  /* Loop Gain */
        __RW uint8_t bFBBrkFactor       :   3;  /* Feedback Brake Factor */
        __RW uint8_t bNGThreshold       :   1;  /* Noise Gate Threshold */
    } b;
} DRV262x_FeedbackControlReg_t;

//! \brief Feedback Brake Factor options
//!
typedef enum
{
    DRV262x_feedbackBrakeFactor_1x = 0,
    DRV262x_feedbackBrakeFactor_2x,
    DRV262x_feedbackBrakeFactor_3x,
    DRV262x_feedbackBrakeFactor_4x,
    DRV262x_feedbackBrakeFactor_6x,
    DRV262x_feedbackBrakeFactor_8x,
    DRV262x_feedbackBrakeFactor_16x,
    DRV262x_feedbackBrakeFactor_disabled
} DRV262x_FeedbackBrakeFactor_t;

//! \brief Back EMF options
//!
typedef enum
{
    DRV262x_backEMFgain_0 = 0,
    DRV262x_backEMFgain_1,
    DRV262x_backEMFgain_2,
    DRV262x_backEMFgain_3
} DRV262x_BackEMFGain_t;

//! \brief Loop Gain options
//!
typedef enum
{
    DRV262x_loopGain_verySlow = 0,
    DRV262x_loopGain_slow,
    DRV262x_loopGain_fast,
    DRV262x_loopGain_veryFast
} DRV262x_LoopGain_t;

//! \brief Noise Gate Threshold
//!
typedef enum
{
    DRV262x_ngThres_4 = 0,
    DRV262x_ngThres_8
} DRV262x_NGThreshold;

//! \brief Rated Voltage (0x24) (DRV262x_reg_ratedVoltageClamp)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bRatedVoltageClamp :   8;  /* Rated Voltage */
    } b;
} DRV262x_RatedVoltageClampReg_t;

//! \brief Overdrive Clamp Level 1 (0x25) (DRV262x_reg_overdriveVoltLvl1)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bOverdriveVoltageLvl1 :   8;  /* Overdrive Clamp Level 1 */
    } b;
} DRV262x_OverdriveVoltLvl1Reg_t;

//! \brief Overdrive Clamp Level 2 (0x26) (DRV262x_reg_overdriveVoltLvl2)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bOverdriveVoltageLvl2 :   8;  /* Overdrive Clamp Level 2 */
    } b;
} DRV262x_OverdriveVoltLvl2Reg_t;

//! \brief Control 3 (0x27) (DRV262x_reg_ctrl3)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bDriveTime         :   5;  /* Drive Time */
        __R  uint8_t bRESERVED          :   1;  /* Reserved */
        __RW uint8_t bLRAResyncFormat   :   1;  /* Resync Format */
        __RW uint8_t bLRAMinFreqSel     :   1;  /* Min Frequency Support 0=125 1=45 */
    } b;
} DRV262x_Ctrl3Reg_t;

//! \brief Control 4 (0x28) (DRV262x_reg_ctrl4)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bIdissTime         :   4;  /* Time for Inductor Current to Discharge */
        __RW uint8_t bBlankingTime      :   4;  /* Time for BEMF to Settle */
    } b;
} DRV262x_Ctrl4Reg_t;

//! \brief Control 5 (0x29) (DRV262x_reg_ctrl5)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bZCDetTime         :   2;  /* Zero Crossing Detection Time */
        __RW uint8_t bSampleTime        :   2;  /* Sample Time */
        __RW uint8_t bODClampTime       :   2;  /* Overdrive Clamp Time */
        __R  uint8_t bRESERVED          :   2;  /* Reserved */
    } b;
} DRV262x_Ctrl5Reg_t;

//! \brief Sampling Time options
//!
typedef enum
{
    DRV262x_LRASamplingTime_150us = 0,
    DRV262x_LRASamplingTime_200us,
    DRV262x_LRASamplingTime_250us,
    DRV262x_LRASamplingTime_300us
} DRV262x_LRASampleTime_t;

//! \brief Control 6 (0x2A) (DRV262x_reg_ctrl6)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bAutoCalTime       :   2;  /* Auto-Calibration Time */
        __R  uint8_t bRESERVED          :   6;  /* Reserved */
    } b;
} DRV262x_Ctrl6Reg_t;

//! \brief Auto Calibration time options
//!
typedef enum
{
    DRV262x_autoCalTime_250ms = 0,
    DRV262x_autoCalTime_500ms,
    DRV262x_autoCalTime_1000ms,
    DRV262x_autoCalTime_trigger_ctrl
} DRV262x_AutoCalTime_t;

//! \brief Reserved (0x2B) (DRV262x_reg_null2)
//!
typedef union {
    __R uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bRESERVED           :   8;  /* Reserved */
    } b;
} DRV262x_Null2Reg_t;

//! \brief Control 7 (0x2C) (DRV262x_reg_ctrl7)
//!
typedef union {
    __RW uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bLRAWaveShape      :   1;  /* Sine(1) or Square(0) Wave */
        __R  uint8_t bRESERVED          :   4;  /* Reserved */
        __RW uint8_t bAutoOLCount       :   2;  /* Switch to OL after 3,4,5,6 attempts to read BEMF */
        __RW uint8_t bLRAAutoOL         :   1;  /* Enable LRA Auto-OL */
    } b;
} DRV262x_Ctrl7Reg_t;

//! \brief Wave Shape options
//!
typedef enum
{
    DRV262x_LRAWaveShape_Square = 0,
    DRV262x_LRAWaveShape_Sine,
} DRV262x_WaveShape_t;

//! \brief Reserved (0x2D) (DRV262x_reg_null3)
//!
typedef union {
    __R uint8_t r;                            /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bRESERVED           :   8;  /* Reserved */
    } b;
} DRV262x_Null3Reg_t;

//! \brief LRA Open-Loop Period [8:9] (0x2E) (DRV262x_reg_LRAOLPeriodMSB)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bLRAOLPeriodMSB    :   2;  /* LRA Period[8:9] */
        __R uint8_t bRESERVED0          :   6;  /* Reserved */
    } b;
} DRV262x_LRAOLPeriodMSBReg_t;

//! \brief LRA Open-Loop Period [7:0] (0x2F) (DRV262x_reg_LRAOLPeriodLSB)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
        __RW uint8_t bLRAOLPeriodLSB     :   8;  /* LRA Period[7:0] */
    } b;
} DRV262x_LRAOLPeriodLSBReg_t;

//! \brief Current Coefficient (0x30) (DRV262x_reg_currentCoefficient)
//!
typedef union {
    __R uint8_t r;                             /* Complete Register */
    struct {                                    /* Register Bits */
        __R uint8_t bCurrentK           :   8;  /* Current Coefficient */
    } b;
} DRV262x_CurrentCoefficientReg_t;

#endif /* DRV262x_REGISTERFILE_H_ */
