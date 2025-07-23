export interface AudioEffect {
  id: string;
  type: string;
  enabled: boolean;
  parameters: Record<string, number>;
}

export interface PedalBoard {
  name: string;
  effects: AudioEffect[];
  inputGain: number;
  masterBypass: boolean;
}

export class AudioEngine {
  private audioContext: AudioContext | null = null;
  private workletNode: AudioWorkletNode | null = null;
  private inputNode: MediaStreamAudioSourceNode | null = null;
  private gainNode: GainNode | null = null;
  private enabled = false;
  private effects: AudioEffect[] = [];

  async initialize(): Promise<void> {
    try {
      // Create audio context
      this.audioContext = new AudioContext();
      
      // Load audio worklet
      await this.audioContext.audioWorklet.addModule('/audio-worklet.js');
      
      // Create worklet node
      this.workletNode = new AudioWorkletNode(this.audioContext, 'effects-processor');
      
      // Create gain node for output volume
      this.gainNode = this.audioContext.createGain();
      this.gainNode.gain.value = 0.8;
      
      // Connect nodes: worklet -> gain -> destination
      this.workletNode.connect(this.gainNode);
      this.gainNode.connect(this.audioContext.destination);
      
      console.log('Audio engine initialized');
    } catch (error) {
      console.error('Failed to initialize audio engine:', error);
      throw error;
    }
  }

  async startAudio(): Promise<void> {
    if (!this.audioContext || !this.workletNode) {
      throw new Error('Audio engine not initialized');
    }

    try {
      // Request microphone access
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: false,
          noiseSuppression: false,
          autoGainControl: false,
          sampleRate: 44100
        }
      });

      // Create input source
      this.inputNode = this.audioContext.createMediaStreamSource(stream);
      
      // Connect input to worklet
      this.inputNode.connect(this.workletNode);
      
      // Resume audio context if suspended
      if (this.audioContext.state === 'suspended') {
        await this.audioContext.resume();
      }
      
      this.enabled = true;
      this.sendWorkletMessage({ type: 'setEnabled', data: { enabled: true } });
      
      console.log('Audio started');
    } catch (error) {
      console.error('Failed to start audio:', error);
      throw error;
    }
  }

  stopAudio(): void {
    if (this.inputNode) {
      this.inputNode.disconnect();
      this.inputNode = null;
    }
    
    this.enabled = false;
    this.sendWorkletMessage({ type: 'setEnabled', data: { enabled: false } });
    
    console.log('Audio stopped');
  }

  addEffect(effect: AudioEffect): void {
    this.effects.push(effect);
    this.sendWorkletMessage({
      type: 'addEffect',
      data: {
        id: effect.id,
        type: effect.type,
        params: effect.parameters
      }
    });
  }

  removeEffect(effectId: string): void {
    this.effects = this.effects.filter(e => e.id !== effectId);
    this.sendWorkletMessage({
      type: 'removeEffect',
      data: { id: effectId }
    });
  }

  updateEffect(effectId: string, parameters: Record<string, number>): void {
    const effect = this.effects.find(e => e.id === effectId);
    if (effect) {
      effect.parameters = { ...effect.parameters, ...parameters };
      this.sendWorkletMessage({
        type: 'updateEffect',
        data: { id: effectId, params: parameters }
      });
    }
  }

  setEffectEnabled(effectId: string, enabled: boolean): void {
    const effect = this.effects.find(e => e.id === effectId);
    if (effect) {
      effect.enabled = enabled;
      this.updateEffect(effectId, { enabled: enabled ? 1 : 0 });
    }
  }

  setInputGain(gain: number): void {
    this.sendWorkletMessage({
      type: 'setInputGain',
      data: { gain }
    });
  }

  setMasterBypass(bypass: boolean): void {
    this.sendWorkletMessage({
      type: 'setMasterBypass',
      data: { bypass }
    });
  }

  reorderEffects(effectIds: string[]): void {
    // Reorder effects array to match new order
    const orderedEffects: AudioEffect[] = [];
    effectIds.forEach(id => {
      const effect = this.effects.find(e => e.id === id);
      if (effect) orderedEffects.push(effect);
    });
    this.effects = orderedEffects;
    
    this.sendWorkletMessage({
      type: 'reorderEffects',
      data: { order: effectIds }
    });
  }

  loadPedalBoard(board: PedalBoard): void {
    // Clear existing effects
    this.effects.forEach(effect => this.removeEffect(effect.id));
    this.effects = [];
    
    // Add new effects
    board.effects.forEach(effect => this.addEffect(effect));
    
    // Set other parameters
    this.setInputGain(board.inputGain);
    this.setMasterBypass(board.masterBypass);
  }

  getCurrentBoard(): PedalBoard {
    return {
      name: 'Current Board',
      effects: [...this.effects],
      inputGain: 0.3, // TODO: track this properly
      masterBypass: false // TODO: track this properly
    };
  }

  isEnabled(): boolean {
    return this.enabled;
  }

  getEffects(): AudioEffect[] {
    return [...this.effects];
  }

  private sendWorkletMessage(message: any): void {
    if (this.workletNode) {
      this.workletNode.port.postMessage(message);
    }
  }

  destroy(): void {
    this.stopAudio();
    
    if (this.workletNode) {
      this.workletNode.disconnect();
      this.workletNode = null;
    }
    
    if (this.gainNode) {
      this.gainNode.disconnect();
      this.gainNode = null;
    }
    
    if (this.audioContext) {
      this.audioContext.close();
      this.audioContext = null;
    }
  }
}