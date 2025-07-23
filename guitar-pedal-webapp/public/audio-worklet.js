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
      
      // Soft limiting
      sample = Math.tanh(sample * 0.8);
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

// Overdrive effect (ported from C++)
class OverdriveEffect extends BaseEffect {
  constructor(id) {
    super(id);
    this.gain = 2.0;
    this.mix = 0.8;
    this.outputLevel = 0.6;
    this.lastSample = 0.0;
    this.lastOutput = 0.0;
  }
  
  setParameter(name, value) {
    switch (name) {
      case 'gain':
        this.gain = Math.max(0.1, Math.min(8.0, value));
        break;
      case 'mix':
        this.mix = Math.max(0.0, Math.min(1.0, value));
        break;
      case 'enabled':
        this.enabled = !!value;
        break;
    }
  }
  
  processSample(inputSample) {
    // Pre-emphasis filter to shape tone
    const preEmphasized = inputSample + 0.3 * (inputSample - this.lastSample);
    this.lastSample = inputSample;
    
    // Tube-style saturation with asymmetric clipping
    const driven = preEmphasized * this.gain;
    let processed;
    
    if (driven > 0.0) {
      processed = Math.tanh(driven * 0.7);
    } else {
      processed = Math.tanh(driven * 0.9);
    }
    
    // Simple low-pass filter to smooth harsh frequencies
    processed = 0.7 * processed + 0.3 * this.lastOutput;
    this.lastOutput = processed;
    
    // Mix with dry signal and apply output level
    return (1.0 - this.mix) * inputSample + this.mix * processed * this.outputLevel;
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
    
    // Soft clip input to prevent distortion
    input = Math.tanh(input * 0.8);
    
    // Early reflections
    let early = 0.0;
    for (let i = 0; i < 6; i++) {
      early += this.earlyReflections[i].process(input * 0.3) * 0.167;
    }
    
    // Diffusion allpass filters with reduced gain
    let diffused = input * 0.4 + early * 0.2;
    for (let i = 0; i < 4; i++) {
      const delayed = this.allpassStates[i];
      this.allpassStates[i] = diffused + delayed * (this.diffusion * 0.5);
      diffused = delayed - diffused * (this.diffusion * 0.5);
    }
    
    // Late reverb with reduced input gain
    let late = 0.0;
    for (let i = 0; i < 4; i++) {
      late += this.lateReverb[i].process(diffused * 0.5) * 0.25;
    }
    
    // Tone control (simple lowpass)
    const alpha = 1.0 - this.tone * 0.6;
    this.lowpassState = alpha * this.lowpassState + (1.0 - alpha) * late;
    late = this.lowpassState;
    
    // Mix wet and dry with increased wet gain for better audibility
    const wet = (early * 2.2 + late * 1.8) * 0.8;
    return Math.max(-1.0, Math.min(1.0, (1.0 - this.mix) * input + this.mix * wet));
  }
}

// Placeholder effect classes (will be implemented next)
class ChorusEffect extends BaseEffect {
  constructor(id) {
    super(id);
    // Placeholder implementation
  }
  
  processSample(sample) {
    return sample; // Pass through for now
  }
}

class DistortionEffect extends BaseEffect {
  constructor(id) {
    super(id);
    // Placeholder implementation
  }
  
  processSample(sample) {
    return sample; // Pass through for now
  }
}

class TremoloEffect extends BaseEffect {
  constructor(id) {
    super(id);
    // Placeholder implementation
  }
  
  processSample(sample) {
    return sample; // Pass through for now
  }
}

registerProcessor('effects-processor', EffectsProcessor);