// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2026 dlgus8648

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <stdlib.h>

#include "gstmacttssink_macos.h"

@interface KbTtsDelegate : NSObject <AVSpeechSynthesizerDelegate>
@property (nonatomic, strong) NSCondition *cond;
@property (nonatomic, assign) BOOL done;
@property (nonatomic, assign) mac_tts_word_cb wordCb;
@property (nonatomic, assign) void *wordCbUser;
@end

@implementation KbTtsDelegate
- (instancetype)init
{
  if ((self = [super init])) {
    _cond = [[NSCondition alloc] init];
    _done = NO;
  }
  return self;
}

- (void)signalDone
{
  [_cond lock];
  _done = YES;
  [_cond signal];
  [_cond unlock];
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
    didFinishSpeechUtterance:(AVSpeechUtterance *)utterance
{
  (void) synthesizer;
  (void) utterance;
  [self signalDone];
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
    didCancelSpeechUtterance:(AVSpeechUtterance *)utterance
{
  (void) synthesizer;
  (void) utterance;
  [self signalDone];
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
    willSpeakRangeOfSpeechString:(NSRange)characterRange
                       utterance:(AVSpeechUtterance *)utterance
{
  (void) synthesizer;
  (void) utterance;
  if (_wordCb) {
    _wordCb (_wordCbUser, (unsigned int) characterRange.location,
        (unsigned int) characterRange.length);
  }
}
@end

@interface KbTtsHandle : NSObject
@property (nonatomic, strong) AVSpeechSynthesizer *synth;
@property (nonatomic, strong) KbTtsDelegate *delegate;
@end

@implementation KbTtsHandle
@end

mac_tts_ctx *
mac_tts_init (void)
{
  @autoreleasepool {
    KbTtsHandle *handle = [[KbTtsHandle alloc] init];
    handle.synth = [[AVSpeechSynthesizer alloc] init];
    handle.delegate = [[KbTtsDelegate alloc] init];
    handle.synth.delegate = handle.delegate;
    /* Transfer ownership to caller; mac_tts_shutdown() reclaims it. */
    return (mac_tts_ctx *) CFBridgingRetain (handle);
  }
}

void
mac_tts_shutdown (mac_tts_ctx *ctx)
{
  if (!ctx) {
    return;
  }
  @autoreleasepool {
    KbTtsHandle *handle = (__bridge_transfer KbTtsHandle *) ctx;
    if (handle.synth.isSpeaking) {
      /* 일시정지 상태에서 stop은 콜백(didCancel)이 안 오는 알려진 quirk가
       * 있어, 먼저 continueSpeaking으로 재개한 뒤 즉시 정지한다. */
      if (handle.synth.isPaused) {
        [handle.synth continueSpeaking];
      }
      [handle.synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
    }
    handle.synth.delegate = nil;
    handle.synth = nil;
    handle.delegate = nil;
  }
}

void
mac_tts_pause (mac_tts_ctx *ctx)
{
  if (!ctx) {
    return;
  }
  @autoreleasepool {
    KbTtsHandle *handle = (__bridge KbTtsHandle *) ctx;
    if (handle.synth.isSpeaking && !handle.synth.isPaused) {
      [handle.synth pauseSpeakingAtBoundary:AVSpeechBoundaryWord];
    }
  }
}

void
mac_tts_resume (mac_tts_ctx *ctx)
{
  if (!ctx) {
    return;
  }
  @autoreleasepool {
    KbTtsHandle *handle = (__bridge KbTtsHandle *) ctx;
    if (handle.synth.isPaused) {
      [handle.synth continueSpeaking];
    }
  }
}

void
mac_tts_cancel (mac_tts_ctx *ctx)
{
  if (!ctx) {
    return;
  }
  @autoreleasepool {
    KbTtsHandle *handle = (__bridge KbTtsHandle *) ctx;
    if (handle.synth.isSpeaking) {
      /* 일시정지 상태면 stop 콜백이 안 오는 quirk 대비 먼저 재개 */
      if (handle.synth.isPaused) {
        [handle.synth continueSpeaking];
      }
      [handle.synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
    }
    /* 중요: AVSpeech의 didCancel 콜백은 메인 스레드로 전달되는데, 정지(state
     * 변경)는 메인 스레드를 블록한다 → 콜백이 안 와서 render()의 cond 대기가
     * 영영 안 깨지는 데드락. 그래서 콜백에 의존하지 않고 대기 중인 render()를
     * 여기서 직접 깨운다. (뒤늦게 didCancel이 와도 done은 이미 YES라 무해.) */
    [handle.delegate.cond lock];
    handle.delegate.done = YES;
    [handle.delegate.cond signal];
    [handle.delegate.cond unlock];
  }
}

int
mac_tts_speak (mac_tts_ctx *ctx, const char *utf8_text,
               const mac_tts_options *opts,
               mac_tts_word_cb word_cb, void *word_cb_user)
{
  if (!ctx || !utf8_text) {
    return -1;
  }
  @autoreleasepool {
    KbTtsHandle *handle = (__bridge KbTtsHandle *) ctx;
    NSString *text = [NSString stringWithUTF8String:utf8_text];
    if (!text || text.length == 0) {
      return -1;
    }

    /* 이번 utterance 동안만 단어 콜백 활성화 */
    handle.delegate.wordCb = word_cb;
    handle.delegate.wordCbUser = word_cb_user;

    AVSpeechUtterance *utterance = [AVSpeechUtterance speechUtteranceWithString:text];

    /* AVSpeech의 didFinishSpeechUtterance: 콜백이 audio buffer flush보다
     * 먼저 호출되는 경우가 있어 마지막 음절(특히 한국어 받침)이 잘림.
     * postUtteranceDelay로 콜백을 늦춰 audio가 완전히 끝날 시간을 확보. */
    utterance.postUtteranceDelay = 0.25;

    if (opts) {
      utterance.rate            = opts->rate;
      utterance.pitchMultiplier = opts->pitch;
      utterance.volume          = opts->volume;

      if (opts->voice_id && opts->voice_id[0] != '\0') {
        NSString *vid = [NSString stringWithUTF8String:opts->voice_id];
        if (vid) {
          /* identifier (dotted form) 우선, 실패 시 language code로 fallback */
          AVSpeechSynthesisVoice *voice =
              [AVSpeechSynthesisVoice voiceWithIdentifier:vid];
          if (!voice) {
            voice = [AVSpeechSynthesisVoice voiceWithLanguage:vid];
          }
          if (voice) {
            utterance.voice = voice;
          }
        }
      }
    }

    [handle.delegate.cond lock];
    handle.delegate.done = NO;
    [handle.delegate.cond unlock];

    [handle.synth speakUtterance:utterance];

    [handle.delegate.cond lock];
    while (!handle.delegate.done) {
      [handle.delegate.cond wait];
    }
    [handle.delegate.cond unlock];

    handle.delegate.wordCb = NULL;
    handle.delegate.wordCbUser = NULL;
    return 0;
  }
}
