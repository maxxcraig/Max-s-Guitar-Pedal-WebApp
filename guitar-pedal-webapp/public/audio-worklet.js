// Audio effects processor worklet
class EffectsProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.effects = [];
    this.enabled = false;
    this.inputGain = 0.3;
    this.masterBypass = false;
    
    // Listen for effect updates from main thread
    this.port.onmessage = (event) => {
      const { type, data } = event.data;
      
      switch (type) {
        case 'addEffect':
          this.addEffect(data);
          break;
        case 'removeEffect':
          this.removeEffect(data.id);
          break;
        case 'updateEffect':
          this.updateEffect(data.id, data.params);
          break;
        case 'setEnabled':
          this.enabled = data.enabled;
          break;
        case 'setInputGain':
          this.inputGain = data.gain;
          break;
        case 'setMasterBypass':
          this.masterBypass = data.bypass;
          break;
        case 'reorderEffects':
          this.reorderEffects(data.order);
          break;
      }
    };
  }
  
  addEffect(effectData) {
    let effect;
    
    switch (effectData.type) {
      case 'overdrive':
        effect = new OverdriveEffect(effectData.id);
        break;
      case 'reverb':
        effect = new ReverbEffect(effectData.id);
        break;
      case 'chorus':
        effect = new ChorusEffect(effectData.id);
        break;
      case 'distortion':
        effect = new DistortionEffect(effectData.id);
        break;
      case 'tremolo':
        effect = new TremoloEffect(effectData.id);
        break;
      case 'delay':
        effect = new DelayEffect(effectData.id);
        break;
      case 'phaser':
        effect = new PhaserEffect(effectData.id);
        break;
      case 'bluesdriver':
        effect = new BluesDriverEffect(effectData.id);
        break;
      default:
        return;
    }
    
    if (effectData.params) {
      Object.keys(effectData.params).forEach(key => {
        effect.setParameter(key, effectData.params[key]);
      });
    }
    
    this.effects.push(effect);
  }
  
  removeEffect(id) {
    this.effects = this.effects.filter(effect => effect.id !== id);
  }
  
  updateEffect(id, params) {
    const effect = this.effects.find(e => e.id === id);
    if (effect) {
      Object.keys(params).forEach(key => {
        effect.setParameter(key, params[key]);
      });
    }
  }
  
  reorderEffects(order) {
    const orderedEffects = [];
    order.forEach(id => {
      const effect = this.effects.find(e => e.id === id);
      if (effect) orderedEffects.push(effect);
    });
    this.effects = orderedEffects;
  }
  
  process(inputs, outputs, parameters) {
    const input = inputs[0];
    const output = outputs[0];
    
    if (!input || !input[0] || !this.enabled || this.masterBypass) {
      // Pass through if no input or disabled
      if (output && output[0] && input && input[0]) {
        output[0].set(input[0]);
      }
      return true;
    }
    
    const inputChannel = input[0];
    const outputChannel = output[0];
    
    for (let i = 0; i < inputChannel.length; i++) {
      let sample = inputChannel[i] * this.inputGain;
      
      // Process through effect chain
      for (const effect of this.effects) {
        if (effect.enabled) {
          sample = effect.processSample(sample);
        }
      }
      
      // Soft limiting to prevent clipping
      sample = Math.tanh(sample * 0.7);
      outputChannel[i] = Math.max(-1, Math.min(1, sample));
    }
    
    return true;
  }
}

// Base effect class
class BaseEffect {
  constructor(id) {
    this.id = id;
    this.enabled = false;
  }
  
  processSample(sample) {
    return sample;
  }
  
  setParameter(name, value) {
    // Override in subclasses
  }
  
  setSampleRate(rate) {
    this.sampleRate = rate;
  }
}

// Warm tube-style overdrive effect
class OverdriveEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.gain = 4.0;
    this.level = 1.0;
    this.tone = 5.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'gain':
        this.gain = Math.max(0.1, Math.min(8.0, value));
        break;
      case 'level':
        this.level = Math.max(0.0, Math.min(2.0, value));
        break;
      case 'tone':
        this.tone = Math.max(0.0, Math.min(10.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // Simple overdrive - similar to other working pedals
    let processed = sample * this.gain * 0.5;
    
    // Soft clipping for overdrive character
    processed = Math.tanh(processed * 0.8);
    
    // Add second harmonic for tube warmth
    const harmonic = Math.sin(processed * Math.PI * 2) * 0.08;
    processed += harmonic;
    
    // Simple tone control
    const toneAmount = this.tone / 10.0;
    const filtered = processed * 0.8; // Simulate filtering
    processed = filtered * (1 - toneAmount) + processed * toneAmount;
    
    // Output level
    processed *= this.level * 0.9;
    
    return Math.max(-1.0, Math.min(1.0, processed));
  }
}

// Delay line helper class for reverb
class DelayLine {
  constructor() {
    this.buffer = [];
    this.delaySamples = 0;
    this.feedback = 0.0;
    this.writeIndex = 0;
  }
  
  process(input) {
    if (this.buffer.length === 0) return 0;
    
    const readIndex = (this.writeIndex - this.delaySamples + this.buffer.length) % this.buffer.length;
    const output = this.buffer[readIndex];
    
    this.buffer[this.writeIndex] = input + output * this.feedback;
    this.writeIndex = (this.writeIndex + 1) % this.buffer.length;
    
    return output;
  }
}

// Reverb effect (ported from C++)
class ReverbEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.sampleRate = 44100;
    this.mix = 0.3;
    this.decay = 0.6;
    this.tone = 0.5;
    this.diffusion = 0.625;
    
    this.earlyReflections = Array(6).fill().map(() => new DelayLine());
    this.lateReverb = Array(4).fill().map(() => new DelayLine());
    this.lowpassState = 0.0;
    this.allpassStates = Array(4).fill(0.0);
    
    // Add modulation for subtle wavy effect
    this.modPhase1 = 0;
    this.modPhase2 = Math.PI / 4; // Different phase
    this.modRate1 = 3.2; // Hz - much faster
    this.modRate2 = 4.7; // Hz - much faster
    
    this.initializeDelayLines();
  }
  
  setSampleRate(rate) {
    this.sampleRate = rate;
    this.initializeDelayLines();
  }
  
  initializeDelayLines() {
    // Early reflection delay times (ms converted to samples)
    const earlyDelays = [19.1, 22.6, 28.9, 35.8, 41.2, 47.3];
    const earlyGains = [0.3, 0.25, 0.22, 0.18, 0.15, 0.12];
    
    for (let i = 0; i < 6; i++) {
      const samples = Math.floor(earlyDelays[i] * this.sampleRate / 1000.0);
      this.earlyReflections[i].buffer = new Array(samples).fill(0.0);
      this.earlyReflections[i].delaySamples = samples;
      this.earlyReflections[i].feedback = earlyGains[i] * this.decay * 0.15;
      this.earlyReflections[i].writeIndex = 0;
    }
    
    // Late reverb delay times
    const lateDelays = [89.6, 99.8, 111.3, 125.0];
    for (let i = 0; i < 4; i++) {
      const samples = Math.floor(lateDelays[i] * this.sampleRate / 1000.0);
      this.lateReverb[i].buffer = new Array(samples).fill(0.0);
      this.lateReverb[i].delaySamples = samples;
      this.lateReverb[i].feedback = this.decay * 0.35;
      this.lateReverb[i].writeIndex = 0;
    }
    
    this.lowpassState = 0.0;
    this.allpassStates.fill(0.0);
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'mix':
        this.mix = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'decay':
        this.decay = Math.max(0.1, Math.min(0.85, value));
        this.updateFeedback();
        break;
      case 'tone':
        this.tone = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  updateFeedback() {
    for (let i = 0; i < 6; i++) {
      this.earlyReflections[i].feedback = (0.3 - i * 0.03) * this.decay * 0.15;
    }
    for (let i = 0; i < 4; i++) {
      this.lateReverb[i].feedback = this.decay * 0.35;
    }
  }
  
  processSample(input) {
    if (this.earlyReflections[0].buffer.length === 0) return input;
    
    // Update modulation LFOs for wavy effect
    this.modPhase1 += (2 * Math.PI * this.modRate1) / this.sampleRate;
    this.modPhase2 += (2 * Math.PI * this.modRate2) / this.sampleRate;
    
    if (this.modPhase1 > 2 * Math.PI) this.modPhase1 -= 2 * Math.PI;
    if (this.modPhase2 > 2 * Math.PI) this.modPhase2 -= 2 * Math.PI;
    
    // Create modulation values
    const mod1 = Math.sin(this.modPhase1);
    const mod2 = Math.sin(this.modPhase2);
    
    // Less input clipping to preserve signal strength
    input = Math.tanh(input * 0.9);
    
    // Much stronger early reflections with subtle modulation
    let early = 0.0;
    for (let i = 0; i < 6; i++) {
      // Add very subtle modulation to early reflections
      const modAmount = (mod1 + mod2) * 0.03 * (i / 6); // Much smaller modulation
      early += this.earlyReflections[i].process(input * (0.8 + modAmount)) * 0.167;
    }
    
    // Enhanced diffusion with subtle modulated signal
    let diffused = input * 0.5 + early * 0.8;
    for (let i = 0; i < 4; i++) {
      const delayed = this.allpassStates[i];
      // Very subtle modulation of diffusion
      const modDiffusion = this.diffusion * (0.8 + mod1 * 0.05);
      this.allpassStates[i] = diffused + delayed * modDiffusion;
      diffused = delayed - diffused * modDiffusion;
    }
    
    // Much louder late reverb with subtle modulation
    let late = 0.0;
    for (let i = 0; i < 4; i++) {
      // Very subtle modulation of late reverb input
      const modInput = diffused * (1.0 + mod2 * 0.08);
      late += this.lateReverb[i].process(modInput) * 0.25;
    }
    
    // More open tone control with subtle modulation
    const toneAmount = this.tone;
    const modAlpha = 0.2 + toneAmount * 0.3 + mod1 * 0.02; // Subtle filtering modulation
    this.lowpassState = modAlpha * this.lowpassState + (1.0 - modAlpha) * late;
    
    // Shimmer with subtle wavy modulation
    const shimmer = late - this.lowpassState;
    const modShimmer = shimmer * toneAmount * (1.5 + mod2 * 0.15);
    late = this.lowpassState + modShimmer;
    
    // Create loud ethereal atmosphere with consistent mixing
    const wet = (early * 3.8 + late * 3.5) * 1.7;
    
    // Consistent wet signal level
    const wetLevel = this.mix * 2.2;
    const output = (1.0 - wetLevel) * input + wetLevel * wet;
    
    // Consistent output level
    return Math.max(-1.0, Math.min(1.0, output * 1.15));
  }
}

// Classic chorus effect (like Boss CE-2)
class ChorusEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.sampleRate = 44100;
    this.rate = 0.8; // Hz
    this.depth = 0.4;
    this.feedback = 0.15;
    this.mix = 0.5;
    
    // Two delay lines for stereo-like chorus effect
    this.delayBuffer1 = new Array(Math.floor(this.sampleRate * 0.03)).fill(0); // 30ms max
    this.delayBuffer2 = new Array(Math.floor(this.sampleRate * 0.03)).fill(0);
    this.writeIndex = 0;
    this.lfoPhase1 = 0;
    this.lfoPhase2 = Math.PI / 2; // 90 degree phase offset
    
    // High-pass filter for classic chorus brightness
    this.hpState = 0.0;
    this.lastInput = 0.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'rate':
        this.rate = Math.max(0.1, Math.min(5.0, value));
        break;
      case 'depth':
        this.depth = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'feedback':
        this.feedback = Math.max(0.0, Math.min(0.7, value));
        break;
      case 'mix':
        this.mix = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // Stronger high-pass filter for more brightness
    const hpAlpha = 0.998;
    this.hpState = hpAlpha * this.hpState + hpAlpha * (sample - this.lastInput);
    this.lastInput = sample;
    let processed = sample + this.hpState * 0.6; // More high-freq boost
    
    // Update both LFOs
    this.lfoPhase1 += (2 * Math.PI * this.rate) / this.sampleRate;
    this.lfoPhase2 += (2 * Math.PI * this.rate) / this.sampleRate;
    
    if (this.lfoPhase1 > 2 * Math.PI) this.lfoPhase1 -= 2 * Math.PI;
    if (this.lfoPhase2 > 2 * Math.PI) this.lfoPhase2 -= 2 * Math.PI;
    
    // Much more pronounced LFO modulation
    const lfo1 = Math.sin(this.lfoPhase1);
    const lfo2 = Math.sin(this.lfoPhase2 + Math.PI/3) * 0.9; // Different phase and amplitude
    
    // Extremely wide delay modulation range for very obvious effect
    const baseDelay = 15; // Even higher base delay
    const modRange = this.depth * 25; // Much wider range (15-40ms)
    const delay1Samples = baseDelay + modRange * (1 + lfo1) / 2;
    const delay2Samples = baseDelay + modRange * (1 + lfo2) / 2;
    
    // Interpolated reads from both delay lines
    const readIndex1 = this.writeIndex - delay1Samples;
    const readIndex2 = this.writeIndex - delay2Samples;
    
    const getDelayedSample = (buffer, readIndex) => {
      const intIndex = Math.floor(readIndex);
      const fracIndex = readIndex - intIndex;
      const index1 = (intIndex + buffer.length) % buffer.length;
      const index2 = (intIndex + 1 + buffer.length) % buffer.length;
      return buffer[index1] * (1 - fracIndex) + buffer[index2] * fracIndex;
    };
    
    const delayed1 = getDelayedSample(this.delayBuffer1, readIndex1);
    const delayed2 = getDelayedSample(this.delayBuffer2, readIndex2);
    
    // Write to delay buffers with more feedback for richness
    this.delayBuffer1[this.writeIndex] = processed + delayed1 * this.feedback;
    this.delayBuffer2[this.writeIndex] = processed + delayed2 * this.feedback;
    this.writeIndex = (this.writeIndex + 1) % this.delayBuffer1.length;
    
    // Create extremely prominent wet signal
    const wetSignal = (delayed1 * 1.5 + delayed2 * 1.3) * 1.0;
    
    // Add much more pitch modulation for stronger chorus character
    const pitchMod = (lfo1 + lfo2) * 0.005;
    const modulatedWet = wetSignal * (1 + pitchMod);
    
    // Add vibrato-like effect for more obvious modulation
    const vibrato = Math.sin(this.lfoPhase1 * 2) * this.depth * 0.003;
    const vibratoWet = modulatedWet * (1 + vibrato);
    
    // Very prominent chorus mix - wet signal dominates
    const output = (1 - this.mix) * sample + this.mix * vibratoWet * 2.2;
    
    // Boost overall output significantly
    return output * 1.2;
  }
}

// DS-1 style distortion (Kurt Cobain's sound)
class DistortionEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.gain = 6.0;
    this.level = 1.0;
    this.tone = 7.0;
    
    // Filter states for DS-1 characteristic EQ
    this.highShelfState = 0.0;
    this.lowpassState1 = 0.0;
    this.lowpassState2 = 0.0;
    this.lastInput = 0.0;
    
    // Pre-distortion filter state
    this.preFilterState = 0.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'gain':
        this.gain = Math.max(0.1, Math.min(10.0, value));
        break;
      case 'level':
        this.level = Math.max(0.0, Math.min(2.0, value));
        break;
      case 'tone':
        this.tone = Math.max(0.0, Math.min(10.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // Pre-distortion high-pass to tighten low end (like DS-1)
    const hp_alpha = 0.98;
    this.preFilterState = hp_alpha * this.preFilterState + hp_alpha * (sample - this.lastInput);
    this.lastInput = sample;
    let processed = this.preFilterState;
    
    // Slightly less aggressive pre-gain boost
    processed *= this.gain * 1.3;
    
    // First stage - hard clipping but less extreme
    const firstThreshold = 0.45;
    if (processed > firstThreshold) {
      processed = firstThreshold + (processed - firstThreshold) * 0.03;
    } else if (processed < -firstThreshold * 0.85) {
      processed = -firstThreshold * 0.85 + (processed + firstThreshold * 0.85) * 0.04;
    }
    
    // Reduced harmonics for drier sound
    const harmonics = Math.sin(processed * 5) * 0.1 + Math.sin(processed * 10) * 0.05;
    processed += harmonics;
    
    // Second stage - still aggressive but drier
    const secondThreshold = 0.55;
    if (Math.abs(processed) > secondThreshold) {
      const sign = Math.sign(processed);
      const excess = Math.abs(processed) - secondThreshold;
      // Hard limiting but less crushed
      processed = sign * (secondThreshold + excess * 0.02);
    }
    
    // Less bit-crushing for drier sound
    const bitDepth = 48; // Higher = less crushed
    processed = Math.round(processed * bitDepth) / bitDepth;
    
    // Reduced saturation for tighter sound
    processed = Math.tanh(processed * 1.5) * 1.1;
    
    // Post-distortion tone shaping
    const toneAmount = this.tone / 10.0;
    
    // High shelf boost/cut (DS-1's tone control)
    const shelfFreq = 1500;
    const shelfAlpha = Math.exp(-2.0 * Math.PI * shelfFreq / 44100);
    this.highShelfState = shelfAlpha * this.highShelfState + (1 - shelfAlpha) * processed;
    const highShelf = processed - this.highShelfState;
    
    // More aggressive tone control
    const shelfGain = -2.0 + toneAmount * 4.0;
    processed = processed + highShelf * shelfGain * 0.4;
    
    // Aggressive low-pass to control the worst of the dirt
    const lpFreq = 3500 + toneAmount * 2500;
    const lpAlpha = Math.exp(-2.0 * Math.PI * lpFreq / 44100);
    
    this.lowpassState1 = lpAlpha * this.lowpassState1 + (1 - lpAlpha) * processed;
    this.lowpassState2 = lpAlpha * this.lowpassState2 + (1 - lpAlpha) * this.lowpassState1;
    
    processed = this.lowpassState2;
    
    // Enhanced mid-range boost for more aggressive character
    const midBoost = 1.0 + 0.6 * (1.0 - Math.abs(toneAmount - 0.6) * 2.0);
    processed *= midBoost;
    
    // Final output with heavy compression to control the dirt
    processed *= this.level * 0.8;
    processed = Math.tanh(processed * 0.7) * 1.1;
    
    return processed;
  }
}

// Classic amp-style tremolo effect
class TremoloEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.sampleRate = 44100;
    this.rate = 5.0; // Hz
    this.depth = 0.7;
    this.level = 1.2;
    this.lfoPhase = 0;
    
    // Smoothing filter to reduce clicks
    this.smoothingState = 1.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'rate':
        this.rate = Math.max(0.1, Math.min(20.0, value));
        break;
      case 'depth':
        this.depth = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'level':
        this.level = Math.max(0.0, Math.min(2.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // Update LFO
    this.lfoPhase += (2 * Math.PI * this.rate) / this.sampleRate;
    if (this.lfoPhase > 2 * Math.PI) this.lfoPhase -= 2 * Math.PI;
    
    // Classic amp tremolo uses a more complex waveform
    // Combine sine wave with slight triangle wave for more authentic sound
    const sineWave = Math.sin(this.lfoPhase);
    const triangleWave = (2 / Math.PI) * Math.asin(Math.sin(this.lfoPhase));
    const lfoValue = sineWave * 0.85 + triangleWave * 0.15;
    
    // Much more dramatic tremolo modulation for obvious effect
    const modulation = 1.0 - this.depth * 0.95 * (1 + lfoValue) / 2;
    
    // Less smoothing for more pronounced effect
    const smoothingAlpha = 0.995;
    this.smoothingState = smoothingAlpha * this.smoothingState + (1 - smoothingAlpha) * modulation;
    
    // Apply tremolo with level control - make it much more noticeable
    const processed = sample * this.smoothingState * this.level;
    
    // Add slight even harmonics for tube-like character
    const harmonics = Math.sin(sample * Math.PI * 3) * 0.05 * this.depth;
    
    return processed + harmonics;
  }
}

// Classic analog delay effect
class DelayEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.sampleRate = 44100;
    this.time = 0.38; // seconds (classic 380ms)
    this.feedback = 0.35;
    this.mix = 0.4;
    
    this.maxDelay = Math.floor(this.sampleRate * 2); // 2 second max delay
    this.delayBuffer = new Array(this.maxDelay).fill(0);
    this.writeIndex = 0;
    
    // Analog-style filtering in feedback loop
    this.feedbackLowpass = 0.0;
    this.feedbackHighpass = 0.0;
    this.lastFeedback = 0.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'time':
        this.time = Math.max(0.01, Math.min(2.0, value));
        break;
      case 'feedback':
        this.feedback = Math.max(0.0, Math.min(0.95, value));
        break;
      case 'mix':
        this.mix = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    const delaySamples = Math.floor(this.time * this.sampleRate);
    
    // Get three taps at different delay times
    const tap1Index = (this.writeIndex - delaySamples + this.maxDelay) % this.maxDelay;
    const tap2Index = (this.writeIndex - delaySamples * 2 + this.maxDelay) % this.maxDelay;
    const tap3Index = (this.writeIndex - delaySamples * 3 + this.maxDelay) % this.maxDelay;
    
    let tap1 = this.delayBuffer[tap1Index];
    let tap2 = this.delayBuffer[tap2Index];
    let tap3 = this.delayBuffer[tap3Index];
    
    // Apply analog-style filtering to first tap for feedback
    const hpAlpha = 0.998;
    this.feedbackHighpass = hpAlpha * this.feedbackHighpass + hpAlpha * (tap1 - this.lastFeedback);
    this.lastFeedback = tap1;
    
    // Low-pass to simulate tape/analog degradation
    const lpAlpha = 0.3 + this.feedback * 0.4;
    this.feedbackLowpass = lpAlpha * this.feedbackLowpass + (1 - lpAlpha) * this.feedbackHighpass;
    
    // Write to delay buffer with filtered feedback from first tap only
    this.delayBuffer[this.writeIndex] = sample + this.feedbackLowpass * this.feedback;
    this.writeIndex = (this.writeIndex + 1) % this.maxDelay;
    
    // Add slight modulation to each tap for analog character
    const mod1 = Math.sin(this.writeIndex * 0.0001) * 0.002;
    const mod2 = Math.sin(this.writeIndex * 0.00015) * 0.0015;
    const mod3 = Math.sin(this.writeIndex * 0.0002) * 0.001;
    
    tap1 *= (1 + mod1);
    tap2 *= (1 + mod2);
    tap3 *= (1 + mod3);
    
    // Create multiple taps with decreasing volume (each 60% of previous)
    const delayedSignal = tap1 * 1.0 + tap2 * 0.6 + tap3 * 0.36;
    
    // Balanced delay mix
    return (1 - this.mix) * sample + this.mix * delayedSignal * 1.2;
  }
}

// Classic 70s phaser effect (like MXR Phase 90)
class PhaserEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.sampleRate = 44100;
    this.rate = 0.5; // Hz
    this.depth = 0.8;
    this.feedback = 0.3;
    this.mix = 0.7; // More wet for classic phaser sound
    
    this.lfoPhase = 0;
    this.allpassStates = Array(4).fill(0); // 4-stage like classic phasers
    
    // Add some analog character
    this.dcBlocker = 0.0;
    this.lastInput = 0.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'rate':
        this.rate = Math.max(0.1, Math.min(5.0, value));
        break;
      case 'depth':
        this.depth = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'feedback':
        this.feedback = Math.max(0.0, Math.min(0.95, value));
        break;
      case 'mix':
        this.mix = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // DC blocking for cleaner sound
    const dcAlpha = 0.995;
    this.dcBlocker = dcAlpha * this.dcBlocker + dcAlpha * (sample - this.lastInput);
    this.lastInput = sample;
    let processed = this.dcBlocker;
    
    // Update LFO with triangle wave for more authentic sweep
    this.lfoPhase += (2 * Math.PI * this.rate) / this.sampleRate;
    if (this.lfoPhase > 2 * Math.PI) this.lfoPhase -= 2 * Math.PI;
    
    // Triangle wave LFO (more linear sweep than sine)
    const triangleLfo = (2 / Math.PI) * Math.asin(Math.sin(this.lfoPhase));
    
    // Classic phaser frequency range (wider sweep for more obvious effect)
    const baseFreq = 100;
    const freqRange = 4000;
    const freq = baseFreq + this.depth * freqRange * (1 + triangleLfo) / 2;
    
    // Calculate allpass coefficient with safety bounds
    const omega = Math.min(2 * Math.PI * freq / this.sampleRate, Math.PI * 0.99);
    const coefficient = (Math.tan(omega / 2) - 1) / (Math.tan(omega / 2) + 1);
    
    // Apply 4-stage allpass filter cascade
    for (let i = 0; i < 4; i++) {
      const input = processed;
      processed = coefficient * input + this.allpassStates[i];
      this.allpassStates[i] = input - coefficient * processed;
    }
    
    // Add feedback for resonance
    const feedbackSignal = processed * this.feedback;
    processed = processed + feedbackSignal;
    
    // Classic phaser mixing - combine with original for notch effect
    const wetSignal = processed;
    const drySignal = sample;
    
    // Mix with phase inversion for classic phaser sweep
    const output = drySignal + this.mix * (wetSignal - drySignal) * 1.2;
    
    return Math.max(-1.0, Math.min(1.0, output));
  }
}

// Classic Tube Screamer style blues driver
class BluesDriverEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.gain = 3.0;
    this.level = 1.0;
    this.tone = 5.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'gain':
        this.gain = Math.max(0.1, Math.min(6.0, value));
        break;
      case 'level':
        this.level = Math.max(0.0, Math.min(2.0, value));
        break;
      case 'tone':
        this.tone = Math.max(0.0, Math.min(10.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(sample) {
    // Simple blues driver - based on tube screamer characteristics
    let processed = sample * this.gain * 2.0;
    
    // Soft clipping for tube-like distortion
    processed = Math.tanh(processed * 1.2);
    
    // Add second harmonic for twang
    const harmonic = Math.sin(processed * Math.PI * 2) * 0.1;
    processed += harmonic;
    
    // Simple tone control
    const toneAmount = this.tone / 10.0;
    const filtered = processed * 0.7; // Simulate low-pass
    processed = filtered * (1 - toneAmount) + processed * toneAmount;
    
    // Output level
    processed *= this.level * 1.5;
    
    return Math.max(-1.0, Math.min(1.0, processed));
  }
}

registerProcessor('effects-processor', EffectsProcessor);