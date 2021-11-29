/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import {ErrorCallback, AsyncCallback, Callback} from './basic';
import {VideoPlayer, AudioPlayer} from './@ohos.multimedia.media'
/**
 * @name audio
 * @since 6
 * @sysCap SystemCapability.Multimedia.Audio
 * @import import audio from '@ohos.Multimedia.audio';
 * @permission
 */
declare namespace audio {

  /**
   * Obtains an AudioManager instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function getAudioManager(): AudioManager;

  /**
   * Obtains an AudioCapturer instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function createAudioCapturer(volumeType: AudioVolumeType): AudioCapturer;

  /**
   * Obtains an AudioRenderer instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function createAudioRenderer(volumeType: AudioVolumeType): AudioRenderer;

  /**
   * Enumerates audio stream types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioVolumeType {
	/**
     * Audio streams for ring tones
     */
    RINGTONE = 2,
    /**
     * Audio streams for media purpose
     */
    MEDIA = 3,
    /**
     * Audio streams for voice calls
     */
    VOICE_CALL = 4,
    /**
     * Audio stream for voice assistant
     */
    VOICE_ASSISTANT = 5,
  }

  /**
   * Enumerates audio device flags.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceFlag {
    /**
     * Output devices
     */
    OUTPUT_DEVICES_FLAG = 1,
    /**
     * Input devices
     */
    INPUT_DEVICES_FLAG = 2,
    /**
     * All devices
     */
    ALL_DEVICES_FLAG = 3,
  }
  /**
   * Enumerates device roles.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceRole {
    /**
     * Input role
     */
    INPUT_DEVICE = 1,
    /**
     * Output role
     */
    OUTPUT_DEVICE = 2,
  }
  /**
   * Enumerates device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceType {
    /**
     * Invalid device
     */
    INVALID = 0,
    /**
     * earpiece
     */
    EARPIECE = 1,
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Wired headset
     */
    WIRED_HEADSET = 3,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
    /**
     * Bluetooth device using advanced audio distribution profile (A2DP)
     */
    BLUETOOTH_A2DP = 8,
    /**
     * Microphone
     */
    MIC = 15,
  }
  /**
   * Enumerates Active device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
   enum ActiveDeviceType {
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
  }
  /**
   * Enumerates Audio Ringer modes
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioRingMode {
    /**
     * Silent mode
     */
    RINGER_MODE_SILENT = 0,
    /**
     * Vibration mode
     */
    RINGER_MODE_VIBRATE,
	 /**
     * Normal mode
     */
    RINGER_MODE_NORMAL,
  }

  /**
   * Enumerates the sample format.
   */
  enum AudioSampleFormat {
    SAMPLE_U8 = 1,
    SAMPLE_S16LE = 2,
    SAMPLE_S24LE = 3,
    SAMPLE_S32LE = 4,
    INVALID_WIDTH = -1
  }

  /**
   * Enumerates the audio channel.
   */
  enum AudioChannel {
    MONO = 1,
    STEREO
  }

  /**
   * Enumerates the audio sampling rate.
   */
  enum AudioSamplingRate {
    SAMPLE_RATE_8000 = 8000,
    SAMPLE_RATE_11025 = 11025,
    SAMPLE_RATE_12000 = 12000,
    SAMPLE_RATE_16000 = 16000,
    SAMPLE_RATE_22050 = 22050,
    SAMPLE_RATE_24000 = 24000,
    SAMPLE_RATE_32000 = 32000,
    SAMPLE_RATE_44100 = 44100,
    SAMPLE_RATE_48000 = 48000,
    SAMPLE_RATE_64000 = 64000,
    SAMPLE_RATE_96000 = 96000
  }

  /**
   * Enumerates the audio encoding type.
   */
  enum AudioEncodingType {
    ENCODING_PCM = 0,
    ENCODING_INVALID
  }

  /**
   * Enumerates the audio content type.
   */
  enum ContentType {
    CONTENT_TYPE_UNKNOWN = 0,
    CONTENT_TYPE_SPEECH = 1,
    CONTENT_TYPE_MUSIC = 2,
    CONTENT_TYPE_MOVIE = 3,
    CONTENT_TYPE_SONIFICATION = 4,
  }

  /**
   * Enumerates the stream usage.
   */
  enum StreamUsage {
    STREAM_USAGE_UNKNOWN = 0,
    STREAM_USAGE_MEDIA = 1,
    STREAM_USAGE_VOICE_COMMUNICATION = 2,
    STREAM_USAGE_NOTIFICATION_RINGTONE = 3,
  }

  /**
   * Enumerates the audio render rate.
   */
  enum AudioRendererRate {
    RENDER_RATE_NORMAL = 0,
    RENDER_RATE_DOUBLE = 1,
    RENDER_RATE_HALF = 2,
  }

  enum InterruptType {
    INTERRUPT_TYPE_BEGIN = 1,
    INTERRUPT_TYPE_END = 2,
  }

  enum InterruptHint {
    INTERRUPT_HINT_NONE = 0,
    INTERRUPT_HINT_RESUME,
    INTERRUPT_HINT_PAUSE,
    INTERRUPT_HINT_STOP,
    INTERRUPT_HINT_DUCK,
    INTERRUPT_HINT_UNDUCK
  }

  enum InterruptActionType {
    TYPE_ACTIVATED = 1,
    TYPE_INTERRUPTED = 2,
    TYPE_DEACTIVATED = 3
  }

  /**
   * Manages audio volume and audio device information.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioManager {
    /**
     * Sets volume for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number, callback: AsyncCallback<void>): void;
    /**
     * Sets volume for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number): Promise<void>;
    /**
     * Obtains volume of a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the volume of a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Sets the stream to mute. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    mute(volumeType: AudioVolumeType, mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Sets the stream to mute. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    mute(volumeType: AudioVolumeType, mute: boolean): Promise<void>;
	  /**
     * Checks whether the stream is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType): Promise<boolean>;
    /**
     * Checks whether the stream is active. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is active. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType): Promise<boolean>;
	  /**
     * Mute/Unmutes the microphone. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Mute/Unmutes the microphone. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean): Promise<void>;
    /**
     * Checks whether the microphone is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the microphone is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(): Promise<boolean>;
	  /**
     * Sets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode, callback: AsyncCallback<void>): void;
    /**
     * Sets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode): Promise<void>;
	  /**
     * Gets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(callback: AsyncCallback<AudioRingMode>): void;
    /**
     * Gets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(): Promise<AudioRingMode>;
	  /**
     * Sets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string, callback: AsyncCallback<void>): void;
    /**
     * Sets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string): Promise<void>;
	  /**
     * Gets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string, callback: AsyncCallback<string>): void;
    /**
     * Gets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string): Promise<string>;
    /**
     * Obtains the audio devices of a specified flag. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag, callback: AsyncCallback<AudioDeviceDescriptors>): void;
    /**
     * Obtains the audio devices with a specified flag. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag): Promise<AudioDeviceDescriptors>;
	  /**
     * Activates the device. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean, callback: AsyncCallback<void>): void;
    /**
     * Activates the device. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean): Promise<void>;
    /**
     * Checks whether the device is active. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the device is active. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType): Promise<boolean>;
    /**
     * Activates Audio interrupt
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    activateAudioInterrupt(interrupt: AudioInterrupt): boolean;
    /**
    * Deactivates Audio interrupt
    * @sysCap SystemCapability.Multimedia.Audio
    * @devices
    */
    deactivateAudioInterrupt(interrupt: AudioInterrupt): boolean;
    /**
    * Monitors audio interrupt
    * @sysCap SystemCapability.Multimedia.Audio
    * @devices
    */
    on(type: 'interrupt', volumeType: AudioVolumeType, callback: Callback<InterruptAction>): void;
    /**
     * Stops listening for audio interrupt
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    off(type: 'interrupt', volumeType: AudioVolumeType): void;
  }

  /**
   * Describes an audio device.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioDeviceDescriptor {
    /**
     * Audio device role
     * @devices
     */
    readonly deviceRole: DeviceRole;
    /**
     * Audio device type
     * @devices
     */
    readonly deviceType: DeviceType;
  }

  interface InterruptAction {
    actionType: InterruptActionType;
    interruptType: InterruptType;
    interruptHint: InterruptHint;
  }

  interface AudioInterrupt {
    streamType: AudioVolumeType;
  }
  /**
   * Provides functions for applications for audio playback.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioRenderer {
    /**
     * Sets audio render parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters, callback: AsyncCallback<void>): void;
    /**
     * Sets audio render parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters): Promise<void>;

    /**
     * Obtains audio render parameters. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(callback: AsyncCallback<AudioParameters>): void;
    /**
     * Obtains audio render parameters. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(): Promise<AudioParameters>;

    /**
     * Starts audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(callback: AsyncCallback<boolean>): void;
    /**
     * Starts audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(): Promise<boolean>;

    /**
     * Render audio data. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    write(buffer: ArrayBuffer, callback: AsyncCallback<number>): void;
    /**
     * Render audio data. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    write(buffer: ArrayBuffer): Promise<number>;

    /**
     * Obtains the current timestamp. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(callback: AsyncCallback<number>): void;
    /**
     * Obtains the current timestamp. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(): Promise<number>;

    /**
     * Drain renderer buffer. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    drain(callback: AsyncCallback<boolean>): void;
    /**
     * Drain renderer buffer. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    drain(): Promise<boolean>;

    /**
     * Pauses audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    pause(callback: AsyncCallback<boolean>): void;
    /**
     * Pauses audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    pause(): Promise<boolean>;

    /**
     * Stops audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(callback: AsyncCallback<boolean>): void;
    /**
     * Stops audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(): Promise<boolean>;

    /**
     * Releases resources. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(callback: AsyncCallback<boolean>): void;
    /**
     * Releases resources. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(): Promise<boolean>;

    /**
     * Obtains a reasonable minimum buffer size for renderer, however, the renderer can
     * accept other read sizes as well. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(callback: AsyncCallback<number>): void;
    /**
     * Obtains a reasonable minimum buffer size for renderer, however, the renderer can
     * accept other read sizes as well. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(): Promise<number>;

    /**
     * Set the render rate. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRenderRate(rate: AudioRendererRate, callback: AsyncCallback<number>): void;
    /**
     * Set the render rate. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRenderRate(rate: AudioRendererRate): Promise<number>;

    /**
     * Obtains the current render rate. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRenderRate(callback: AsyncCallback<AudioRendererRate>): void;
    /**
     * Obtains the current render rate. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRenderRate(): Promise<AudioRendererRate>;
  }

  /**
   * Provides functions for applications to manage audio capturing.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioCapturer {
    /**
     * Sets audio capture parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters, callback: AsyncCallback<void>): void;
    /**
     * Sets audio capture parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters): Promise<void>;

    /**
     * Obtains audio capture parameters. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(callback: AsyncCallback<AudioParameters>): void;
    /**
     * Obtains audio capture parameters. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(): Promise<AudioParameters>;

    /**
     * Starts audio capturing. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(callback: AsyncCallback<boolean>): void;
    /**
     * Starts audio capturing. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(): Promise<boolean>;

    /**
     * Capture audio data. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    read(size: number, isBlockingRead: boolean, callback: AsyncCallback<ArrayBuffer>): void;
    /**
     * Capture audio data. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    read(size: number, isBlockingRead: boolean): Promise<ArrayBuffer>;

    /**
     * Obtains the current timestamp. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(callback: AsyncCallback<number>): void;
    /**
     * Obtains the current timestamp. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(): Promise<number>;

    /**
     * Stops audio capturing. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(callback: AsyncCallback<boolean>): void;
    /**
     * Stops audio capturing. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(): Promise<boolean>;

    /**
     * Releases a capture resources. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(callback: AsyncCallback<boolean>): void;
    /**
     * Releases a capture resources. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(): Promise<boolean>;

    /**
     * Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     * accept other read sizes as well. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(callback: AsyncCallback<number>): void;
    /**
     * Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     * accept other read sizes as well. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(): Promise<number>;
  }

  /**
   * Structure for audio parameters
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioParameters {
    /**
     * Audio sample format
     * @devices
     */
    format: AudioSampleFormat;
    /**
     * Audio channels
     * @devices
     */
    channels: AudioChannel;
    /**
     * Audio sampling rate
     * @devices
     */
    samplingRate: AudioSamplingRate;
    /**
     * Audio encoding type
     * @devices
     */
    encoding: AudioEncodingType;
    /**
     * Audio content type
     * @devices
     */
    contentType: ContentType;
    /**
     * Audio stream usage
     * @devices
     */
    usage: StreamUsage;
    /**
     * Audio device role
     * @devices
     */
    deviceRole: DeviceRole;
    /**
     * Audio device type
     * @devices
     */
    deviceType: DeviceType;
  }

  /**
   * A queue of AudioDeviceDescriptor, which is read-only.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  type AudioDeviceDescriptors = Array<Readonly<AudioDeviceDescriptor>>;
}

export default audio;