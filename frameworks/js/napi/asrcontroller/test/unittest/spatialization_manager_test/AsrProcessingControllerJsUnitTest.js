/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import audio from '@ohos.multimedia.audio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

const PARAMETER_MISMATCH = '401';
const PARAMETER_ERROR = '6800101';
const CALLER_ERROR = '6800104';
const ILLEGAL_VALUE = -1;


describe("AsrProcessingControllerJsUnitTest", function () {
//     let audioStreamInfo = {
//       samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_44100,
//       channels: audio.AudioChannel.CHANNEL_2,
//       sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
//       encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
//     }
//     let audioCapturerInfo = {
//       source: audio.SourceType.SOURCE_TYPE_MIC,
//       capturerFlags: 0
//     }
//     let audioCapturerOptions = {
//       streamInfo: audioStreamInfo,
//       capturerInfo: audioCapturerInfo
//     }
    //let capturer = await audio.createAudioCapturer(audioCapturerOptions);
    //let asrProcessingController = audio.createAsrProcessingController();

    beforeAll(async function () {

        // input testsuit setup step，setup invoked before all testcases
        console.info('AsrProcessingControllerJsUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('AsrProcessingControllerJsUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('AsrProcessingControllerJsUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('AsrProcessingControllerJsUnitTest:afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_001
     * @tc.desc:setAsrAecMode by BYPASS value success - AUDIO_SCENE_DEFAULT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_001", 0, function (done) {
     console.info('=====SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_001')
     let asrProcessingController = audio.createAsrProcessingController();
    //  if (asrProcessingController == null || asrProcessingController == undefined) {
    //       console.info('**************')
    //       return;
    //  } 
     console.info('get object success==============')

     try {
          let value = asrProcessingController.setAsrAecMode(1);
          except(value).assertTrue();
     } catch (err) {
          console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_001 SUCCESS: ${err}.`);
          expect(err.code).assertEqual(PARAMETER_MISMATCH);
     }
     done();
    })

    /*
     * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_002
     * @tc.desc:setAsrAecMode by STANDARD value success - AUDIO_SCENE_DEFAULT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
     // it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_002", 0, function (done) {
     //      let asrProcessingController = audio.createAsrProcessingController();

     //     try {
     //          let value = asrProcessingController.setAsrAecMode(audio.AsrAecMode.STANDARD);
     //          except(value).assertFalse();
     //     } catch (err) {
     //          console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_002 SUCCESS: ${err}.`);
     //          expect(err.code).assertEqual(PARAMETER_MISMATCH);
     //     }
     //    done();
     // })


    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_003
     * @tc.desc:setAsrAecMode by illegal value fail - AUDIO_SCENE_DEFAULT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_003", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrAecMode(ILLEGAL_VALUE);
//               except(value).assertFalse();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_AEC_MODE_003 SUCCESS: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_AEC_MODE_001
//      * @tc.desc:getAsrAecMode success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_AEC_MODE_001", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrAecMode(audio.AsrAecMode.BYPASS);
//               except(value).assertTrue();
//               try {
//                    let value = asrProcessingController.getAsrAecMode();
//                    except(value).assertEqual(audio.AsrAecMode.BYPASS);
//               } catch (err) {
//                   console.info(`SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_AEC_MODE_001 ERROR: ${err}.`);
//                   expect(err.code).assertEqual(PARAMETER_MISMATCH);
//               }
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_AEC_MODE_001 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_001
//      * @tc.desc:setAsrNoiseSuppressionMode by BYPASS value success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_001", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(audio.AsrNoiseSuppressionMode.BYPASS);
//               except(value).assertTrue();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_001 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_002
//      * @tc.desc:setAsrNoiseSuppressionMode by STANDARD value success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_002", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(audio.AsrNoiseSuppressionMode.STANDARD);
//               except(value).assertTrue();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_002 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })


//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_003
//      * @tc.desc:setAsrNoiseSuppressionMode by NEAR_FIELD value success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_003", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(audio.AsrNoiseSuppressionMode.NEAR_FIELD);
//               except(value).assertTrue();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_003 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })


//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_004
//      * @tc.desc:setAsrNoiseSuppressionMode by FAR_FIELD value success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_004", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(audio.AsrNoiseSuppressionMode.FAR_FIELD);
//               except(value).assertTrue();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_004 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_003
//      * @tc.desc:setAsrNoiseSuppressionMode by illegal value fail
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_005", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(ILLEGAL_VALUE);
//               except(value).assertFalse();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_SET_ASR_NOISE_SUPPRESSION_MODE_004 ERROR: ${err}.`);
//               expect(err.code).assertEqual(PARAMETER_MISMATCH);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_AEC_MODE_001
//      * @tc.desc:getAsrAecMode with BYPASS value success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_NOISE_SUPPRESSION_MODE_001", 0, function (done) {
//          try {
//               let value = asrProcessingController.setAsrNoiseSuppressionMode(audio.AsrNoiseSuppressionMode.BYPASS);
//               except(value).assertTrue();
//               try {
//                    let value = asrProcessingController.getAsrNoiseSuppressionMode();
//                    except(value).assertEqual(audio.AsrNoiseSuppressionMode.BYPASS);
//               } catch (err) {
//                   console.info(`SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_NOISE_SUPPRESSION_MODE_001 ERROR: ${err}.`);
//                   expect(err.code).assertEqual(PARAMETER_MISMATCH);
//               }
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_GET_ASR_NOISE_SUPPRESSION_MODE_001 ERROR: ${err}.`);
//               expect(err.code).assertEqual(CALLER_ERROR);
//          }
//         done();
//      })

//     /*
//      * @tc.name:SUB_ASR_PROCESSING_CONTROLLER_IS_WHISPERING_001
//      * @tc.desc:isWhisper success - AUDIO_SCENE_DEFAULT
//      * @tc.type: FUNC
//      * @tc.require: I7V04L
//      */
//      it("SUB_ASR_PROCESSING_CONTROLLER_IS_WHISPERING_001", 0, function (done) {
//          try {
//               let value = asrProcessingController.isWhispering();
//          } catch (err) {
//               console.info(`SUB_ASR_PROCESSING_CONTROLLER_IS_WHISPERING_001 ERROR: ${err}.`);
//               expect(err.code).assertEqual(CALLER_ERROR);
//          }
//         done();
//      })
})