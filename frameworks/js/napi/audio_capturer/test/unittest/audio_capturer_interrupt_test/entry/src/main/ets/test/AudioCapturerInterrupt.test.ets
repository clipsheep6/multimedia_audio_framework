/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import audio from '@ohos.multimedia.audio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from "deccjsunit/index";
import abilityAccessCtrl from "@ohos.abilityAccessCtrl";
import bundle from "@ohos.bundle";
import uitest from "@ohos.UiTest";

export default function audioCapturerInterruptTest(abilityContext) {
  describe('audioCapturerInterruptTest', function () {
    function sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    beforeAll(async function () {
        console.info("beforeAll case");
        let atManager = abilityAccessCtrl.createAtManager();
        let permissions = ["ohos.permission.MICROPHONE"];

        try {
            atManager.requestPermissionsFromUser(abilityContext, permissions, (err, data) => {
                console.info(`getPermission requestPermissionsFromUser ${JSON.stringify(data)}`);
            });
        } catch (err) {
            console.log(`get permission catch err -> ${JSON.stringify(err)}`);
        }
        await sleep(1000);

        let driver = uitest.Driver.create();
        await sleep(2000);

        let button = await driver.findComponent(uitest.ON.text("允许"));
        console.info(`button is ${JSON.stringify(button)}`);
        await button.click();
        await sleep(2000);
        let appInfo = await bundle.getApplicationInfo("ohos.audiocapture.bundle.stage.test", 0, 100);
        let tokenID = appInfo.accessTokenId;

        let isGranted = await atManager.verifyAccessToken(tokenID, "ohos.permission.MICROPHONE");
        if (isGranted != 0) {
            console.info("getpermission failed");
        }
    });

    beforeEach(function () {
        // input testcase setup step, setup invoked before each testcases
        console.info('beforeEach called')
    })

    afterEach(function () {
        // input testcase teardown step, teardown invoked after each testcases
        console.info('afterEach called')
    })

    afterAll(function () {
        // input testsuit teardown step, teardown invoked after all testcases
        console.info('afterAll called')
    })

    let capturerInfo = {
        "MIC": {
            source: audio.SourceType.SOURCE_TYPE_MIC,
            capturerFlags: 0,
        },
        'VOICE_RECOGNITION': {
            source: audio.SourceType.SOURCE_TYPE_VOICE_RECOGNITION,
            capturerFlags: 0,
        },
        'VOICE_COMMUNICATION': {
            source: audio.SourceType.SOURCE_TYPE_VOICE_COMMUNICATION,
            capturerFlags: 0,
        }
    }

    let renderInfo = {
        'MUSIC': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0,
        },
        'VOICE_CALL': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
            rendererFlags: 0
        },
        'RINGTONE': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_NOTIFICATION_RINGTONE,
            rendererFlags: 0,
        },
        'VOICE_ASSISTANT': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_ASSISTANT,
            rendererFlags: 0
        },
        'ULTRASONIC': {
            content: audio.ContentType.CONTENT_TYPE_ULTRASONIC,
            usage: audio.StreamUsage.STREAM_USAGE_SYSTEM,
            rendererFlags: 0
        },
        'ALARM': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_ALARM,
            rendererFlags: 0
        },
        'ACCESSIBILITY': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_ACCESSIBILITY,
            rendererFlags: 0
        },
        'SPEECH': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0
        },
        'MOVIE': {
            content: audio.ContentType.CONTENT_TYPE_MOVIE,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0
        },
        'UNKNOW': {
            content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
            usage: audio.StreamUsage.STREAM_USAGE_UNKNOWN,
            rendererFlags: 0
        },
    }

    let streamInfo = {
        '16000': {
            samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_16000,
            channels: audio.AudioChannel.CHANNEL_2,
            sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
            encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
        },
        '44100': {
            samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_44100,
            channels: audio.AudioChannel.CHANNEL_2,
            sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
            encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
        },
        '48000' : {
            samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
            channels: audio.AudioChannel.CHANNEL_2,
            sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S32LE,
            encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
        },
    }

    async function createAudioRenderer(AudioRendererInfo, AudioStreamInfo, done) {
        let render = null;

        var AudioRendererOptions = {
            streamInfo: AudioStreamInfo,
            rendererInfo: AudioRendererInfo
        }
        try {
            render = await audio.createAudioRenderer(AudioRendererOptions)
            console.log("createAudioRenderer success.")
        } catch (err) {
            console.log("createAudioRenderer err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
        return render
    }

    async function renderStart(render, done) {
        try {
            await render.start()
            console.log("renderStart success.")
        } catch (err) {
            await renderRelease(render,done)
            console.log("renderStart err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    async function renderRelease(render, done) {
        if (render && render.state == audio.AudioState.STATE_RELEASED) {
            console.log("renderRelease render state: " + render.state)
            return
        }
        try {
            await render.release()
            console.log("renderRelease success.")
        } catch (err) {
            console.log("renderRelease err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    async function createAudioCapturer(AudioCapturerInfo, AudioStreamInfo, done) {
        let capturer = null;

        var AudioCapturerOptions = {
            streamInfo: AudioStreamInfo,
            capturerInfo: AudioCapturerInfo
        }
        try {
            capturer = await audio.createAudioCapturer(AudioCapturerOptions)
            console.log("createAudioCapturer success.")
        } catch (err) {
            console.log("createAudioCapturer err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
        return capturer
    }

    async function capturerStart(capturer, done) {
        try {
            await capturer.start()
            console.log("capturerStart success.")
        } catch (err) {
            console.log("capturerStart err:" + JSON.stringify(err))
            await capturerRelease(capturer, done)
            expect(false).assertEqual(true)
            done()
        }
    }

    async function capturerStartFail(capturer, done, capturer1) {
        try {
            await capturer.start()
            console.log("capturerStartFail success.")
        } catch (err) {
            console.log("capturerStartFail err:" + JSON.stringify(err))
            await capturerRelease(capturer, done)
            await capturerRelease(capturer1, done)
            expect(true).assertEqual(true)
            done()
        }
    }

    async function capturerRelease(capturer, done) {
        if (capturer && capturer.state == audio.AudioState.STATE_RELEASED) {
            console.log("capturerRelease current state: " + capturer.state)
            return
        }
        try {
            await capturer.release()
            console.log("capturerRelease success.")
        } catch (err) {
            console.log("capturerRelease err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_001', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("01 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_002', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("02 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_003', 0, async function (done) {

        let capture1 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        capture1.on("audioInterrupt", async (eventAction) => {
            console.log("03 capture1.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture2, done)
        await sleep(500)
        await capturerRelease(capture2, done)
        await capturerRelease(capture1, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_004', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_005', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_006', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['RING'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_007', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_008', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_009', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_010', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_011', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_012', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_013', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("013 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_014', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("14 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_015', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        capture1.on("audioInterrupt", async (eventAction) => {
            console.log("15 capture1.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture2, done)
        await sleep(500)
        await capturerRelease(capture2, done)
        await capturerRelease(capture1, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_016', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_017', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_018', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['RING'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_019', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_020', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_021', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_022', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_023', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_024', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_025', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['MIC'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("025 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_026', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_RECOGNITION'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("26 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_027', 0, async function (done) {
        let capture1 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture1, done)

        let capture2 = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        capture2.on("audioInterrupt", async (eventAction) => {
            console.log("27 capture2.eventAction:" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await capturerStartFail(capture2, done, capture1)
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_028', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_029', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_030', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['RING'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_031', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_032', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_033', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_0034', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_035', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })

    it('SUB_AUDIO_CAPTURER_INTERRUPT_TEST_036', 0, async function (done) {
        let capture = await createAudioCapturer(capturerInfo['VOICE_COMMUNICATION'], streamInfo['16000'], done)
        await capturerStart(capture, done)

        let render = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'], done)
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await renderStart(render, done)

        await sleep(500)
        expect(audio.AudioState.STATE_RUNNING == capture.state).assertTrue();
        expect(audio.AudioState.STATE_RUNNING == render.state).assertTrue()
        await capturerRelease(capture, done)
        await renderRelease(render, done)
        done()
    })
  })
}