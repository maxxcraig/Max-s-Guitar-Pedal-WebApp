'use client';

import React, { useState, useRef, useCallback, useEffect } from 'react';
import Image from 'next/image';
import PedalComponent, { PedalParameters } from './PedalComponent';
import { AudioEngine, AudioEffect } from '@/lib/audio-engine';
import { useAuth } from '@/lib/auth-context';
import { SupabaseService } from '@/lib/supabase';
import Notification from './Notification';
import ConfirmDialog from './ConfirmDialog';

interface PedalData {
  id: string;
  type: string;
  name: string;
  imagePath: string;
  enabled: boolean;
  parameters: PedalParameters;
  position: { x: number; y: number };
}

interface PedalBoardProps {
  onBackToHome?: () => void;
  boardToLoad?: any;
  onBoardLoaded?: () => void;
}

const AVAILABLE_PEDALS = [
  { type: 'overdrive', name: 'Overdrive', imagePath: '/images/overdrive.png' },
  { type: 'distortion', name: 'Distortion', imagePath: '/images/Distortion.png' },
  { type: 'reverb', name: 'Reverb', imagePath: '/images/Reverb.png' },
  { type: 'chorus', name: 'Chorus', imagePath: '/images/Chorus.png' },
  { type: 'delay', name: 'Delay', imagePath: '/images/Delay.png' },
  { type: 'tremolo', name: 'Tremolo', imagePath: '/images/Tremolo.png' },
  { type: 'bluesdriver', name: 'Blues Driver', imagePath: '/images/BluesDriver.png' },
  { type: 'phaser', name: 'Phaser', imagePath: '/images/Phaser.png' }
];

export default function PedalBoard({ onBackToHome, boardToLoad, onBoardLoaded }: PedalBoardProps) {
  const [pedals, setPedals] = useState<PedalData[]>([]);
  const [audioEngine] = useState(() => new AudioEngine());
  const [isAudioStarted, setIsAudioStarted] = useState(false);
  const [inputGain, setInputGain] = useState(0.3);
  const [masterBypass, setMasterBypass] = useState(false);
  const [supabaseService] = useState(() => new SupabaseService());
  const [showSaveDialog, setShowSaveDialog] = useState(false);
  const [saveBoardName, setSaveBoardName] = useState('');
  const [notification, setNotification] = useState<{message: string; type: 'success' | 'error' | 'info'} | null>(null);
  const { user } = useAuth();
  const boardRef = useRef<HTMLDivElement>(null);

  // Initialize audio engine
  useEffect(() => {
    const initAudio = async () => {
      try {
        await audioEngine.initialize();
        console.log('Audio engine initialized successfully');
      } catch (error) {
        console.error('Failed to initialize audio engine:', error);
      }
    };
    
    initAudio();
    
    // Cleanup on unmount
    return () => {
      audioEngine.destroy();
    };
  }, [audioEngine]);

  // Load board when boardToLoad prop changes
  useEffect(() => {
    if (boardToLoad) {
      loadBoardData(boardToLoad);
      onBoardLoaded?.();
    }
  }, [boardToLoad, onBoardLoaded]);

  const loadBoardData = (boardData: any) => {
    // Clear existing pedals
    pedals.forEach(pedal => audioEngine.removeEffect(pedal.id));
    setPedals([]);

    // Load new pedals
    if (boardData.effects) {
      const newPedals: PedalData[] = boardData.effects.map((effect: any) => ({
        id: effect.id,
        type: effect.type,
        name: AVAILABLE_PEDALS.find(p => p.type === effect.type)?.name || effect.type,
        imagePath: AVAILABLE_PEDALS.find(p => p.type === effect.type)?.imagePath || '/images/overdrive.png',
        enabled: effect.enabled,
        parameters: effect.parameters,
        position: effect.position || { x: 100, y: 200 }
      }));

      setPedals(newPedals);

      // Add effects to audio engine
      newPedals.forEach(pedal => {
        const audioEffect: AudioEffect = {
          id: pedal.id,
          type: pedal.type,
          enabled: pedal.enabled,
          parameters: pedal.parameters
        };
        audioEngine.addEffect(audioEffect);
      });
    }

    // Load other settings
    if (boardData.inputGain !== undefined) {
      setInputGain(boardData.inputGain);
      audioEngine.setInputGain(boardData.inputGain);
    }

    if (boardData.masterBypass !== undefined) {
      setMasterBypass(boardData.masterBypass);
      audioEngine.setMasterBypass(boardData.masterBypass);
    }
  };

  const startAudio = async () => {
    try {
      await audioEngine.startAudio();
      setIsAudioStarted(true);
      setNotification({ message: 'Audio started successfully!', type: 'success' });
      console.log('Audio started');
    } catch (error) {
      console.error('Failed to start audio:', error);
      setNotification({ message: 'Failed to start audio. Please check microphone permissions.', type: 'error' });
    }
  };

  const stopAudio = () => {
    audioEngine.stopAudio();
    setIsAudioStarted(false);
    setNotification({ message: 'Audio stopped', type: 'info' });
  };

  const addPedal = useCallback((pedalType: string) => {
    const pedalInfo = AVAILABLE_PEDALS.find(p => p.type === pedalType);
    if (!pedalInfo) return;

    const id = `${pedalType}-${Date.now()}`;
    const newPedal: PedalData = {
      id,
      type: pedalType,
      name: pedalInfo.name,
      imagePath: pedalInfo.imagePath,
      enabled: false,
      parameters: getDefaultParameters(pedalType),
      position: { x: 100 + pedals.length * 50, y: 200 }
    };

    setPedals(prev => [...prev, newPedal]);

    // Add to audio engine
    const audioEffect: AudioEffect = {
      id,
      type: pedalType,
      enabled: false,
      parameters: newPedal.parameters
    };
    audioEngine.addEffect(audioEffect);
  }, [pedals.length, audioEngine]);

  const getDefaultParameters = (type: string): PedalParameters => {
    switch (type) {
      case 'overdrive':
        return { gain: 4.0, level: 1.0, tone: 5.0 };
      case 'reverb':
        return { mix: 0.5, decay: 0.475, tone: 0.5 };
      case 'distortion':
        return { gain: 5.0, level: 1.0, tone: 5.0 };
      case 'chorus':
        return { rate: 2.5, depth: 0.5, feedback: 0.35, mix: 0.5 };
      case 'delay':
        return { time: 1.0, feedback: 0.475, mix: 0.5 };
      case 'tremolo':
        return { rate: 10.0, depth: 0.5, level: 1.0 };
      case 'phaser':
        return { rate: 2.5, depth: 0.5, feedback: 0.475, mix: 0.5 };
      case 'bluesdriver':
        return { gain: 3.0, level: 1.0, tone: 5.0 };
      default:
        return { level: 0.5 };
    }
  };

  const removePedal = useCallback((id: string) => {
    setPedals(prev => prev.filter(p => p.id !== id));
    audioEngine.removeEffect(id);
  }, [audioEngine]);

  const updatePedalParameter = useCallback((id: string, parameter: string, value: number) => {
    setPedals(prev => prev.map(pedal => 
      pedal.id === id 
        ? { ...pedal, parameters: { ...pedal.parameters, [parameter]: value } }
        : pedal
    ));
    
    audioEngine.updateEffect(id, { [parameter]: value });
  }, [audioEngine]);

  const togglePedal = useCallback((id: string, enabled: boolean) => {
    setPedals(prev => prev.map(pedal => 
      pedal.id === id ? { ...pedal, enabled } : pedal
    ));
    
    audioEngine.setEffectEnabled(id, enabled);
  }, [audioEngine]);

  const updatePedalPosition = useCallback((id: string, position: { x: number; y: number }) => {
    setPedals(prev => prev.map(pedal => 
      pedal.id === id ? { ...pedal, position } : pedal
    ));
  }, []);

  const handleInputGainChange = (gain: number) => {
    setInputGain(gain);
    audioEngine.setInputGain(gain);
  };

  const handleMasterBypass = () => {
    const newBypass = !masterBypass;
    setMasterBypass(newBypass);
    audioEngine.setMasterBypass(newBypass);
  };

  const savePedalBoard = async () => {
    if (user) {
      // Show save dialog for authenticated users
      setShowSaveDialog(true);
    } else {
      // Save to localStorage for guest users
      const boardData = {
        name: `Board-${Date.now()}`,
        effects: pedals.map(pedal => ({
          id: pedal.id,
          type: pedal.type,
          enabled: pedal.enabled,
          parameters: pedal.parameters,
          position: pedal.position
        })),
        inputGain,
        masterBypass
      };
      
      const savedBoards = JSON.parse(localStorage.getItem('pedalBoards') || '[]');
      savedBoards.push(boardData);
      localStorage.setItem('pedalBoards', JSON.stringify(savedBoards));
      
      setNotification({ message: 'Pedal board saved locally! Sign in to save to the cloud.', type: 'success' });
    }
  };

  const handleSaveToCloud = async () => {
    if (!saveBoardName.trim()) {
      setNotification({ message: 'Please enter a board name', type: 'error' });
      return;
    }

    const boardData = {
      effects: pedals.map(pedal => ({
        id: pedal.id,
        type: pedal.type,
        enabled: pedal.enabled,
        parameters: pedal.parameters,
        position: pedal.position
      })),
      inputGain,
      masterBypass
    };

    try {
      const response = await supabaseService.saveBoard(saveBoardName, boardData);
      if (response.success) {
        setNotification({ message: 'Pedal board saved to cloud!', type: 'success' });
        setShowSaveDialog(false);
        setSaveBoardName('');
      } else {
        setNotification({ message: `Failed to save: ${response.errorMessage}`, type: 'error' });
      }
    } catch (error) {
      setNotification({ message: 'Failed to save pedal board', type: 'error' });
      console.error('Save error:', error);
    }
  };

  return (
    <div 
      className="relative w-full h-screen bg-cover bg-center bg-no-repeat overflow-hidden"
      style={{ backgroundImage: 'url(/images/home-background.png)' }}
    >
      {/* Top Control Bar */}
      <div className="absolute top-4 left-4 right-4 bg-black bg-opacity-80 rounded-lg p-4 z-20">
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-4">
            <button
              onClick={onBackToHome}
              className="bg-gray-600 hover:bg-gray-700 text-white px-4 py-2 rounded-lg transition-colors"
            >
              ← Home
            </button>
            
            <div className="flex items-center space-x-2">
              <span className="text-white text-sm">Input Gain:</span>
              <input
                type="range"
                min="0"
                max="1"
                step="0.01"
                value={inputGain}
                onChange={(e) => handleInputGainChange(parseFloat(e.target.value))}
                className="w-20"
              />
              <span className="text-white text-xs w-8">{(inputGain * 100).toFixed(0)}%</span>
            </div>
          </div>
          
          <div className="flex items-center space-x-4">
            <button
              onClick={handleMasterBypass}
              className={`px-4 py-2 rounded-lg font-semibold transition-colors ${
                masterBypass 
                  ? 'bg-red-600 hover:bg-red-700 text-white' 
                  : 'bg-green-600 hover:bg-green-700 text-white'
              }`}
            >
              {masterBypass ? 'BYPASSED' : 'ACTIVE'}
            </button>
            
            <button
              onClick={savePedalBoard}
              className="bg-blue-600 hover:bg-blue-700 text-white px-4 py-2 rounded-lg transition-colors"
            >
              Save Board
            </button>
            
            {!isAudioStarted ? (
              <button
                onClick={startAudio}
                className="bg-green-600 hover:bg-green-700 text-white px-6 py-2 rounded-lg font-semibold transition-colors"
              >
                Start Audio
              </button>
            ) : (
              <button
                onClick={stopAudio}
                className="bg-red-600 hover:bg-red-700 text-white px-6 py-2 rounded-lg font-semibold transition-colors"
              >
                Stop Audio
              </button>
            )}
          </div>
        </div>
      </div>

      {/* Pedal Palette */}
      <div className="absolute left-4 top-24 bottom-4 w-48 bg-black bg-opacity-80 rounded-lg p-4 z-20 overflow-y-auto">
        <h3 className="text-white font-semibold mb-4">Available Pedals</h3>
        <div className="space-y-2">
          {AVAILABLE_PEDALS.map(pedal => (
            <button
              key={pedal.type}
              onClick={() => addPedal(pedal.type)}
              className="w-full bg-gray-700 hover:bg-gray-600 text-white text-sm py-2 px-3 rounded transition-colors text-left"
            >
              + {pedal.name}
            </button>
          ))}
        </div>
        
        {pedals.length > 0 && (
          <div className="mt-6">
            <h4 className="text-white font-semibold mb-2">Active Pedals</h4>
            <div className="space-y-1">
              {pedals.map(pedal => (
                <div key={pedal.id} className="flex items-center justify-between text-xs">
                  <span className={`${pedal.enabled ? 'text-green-400' : 'text-gray-400'}`}>
                    {pedal.name}
                  </span>
                  <button
                    onClick={() => removePedal(pedal.id)}
                    className="text-red-400 hover:text-red-300 text-xs"
                  >
                    ×
                  </button>
                </div>
              ))}
            </div>
          </div>
        )}
      </div>

      {/* Main Pedal Board Area */}
      <div 
        ref={boardRef}
        className="absolute left-52 top-24 right-4 bottom-4 bg-black bg-opacity-40 rounded-lg overflow-hidden"
      >
        {pedals.map(pedal => (
          <PedalComponent
            key={pedal.id}
            id={pedal.id}
            type={pedal.type}
            name={pedal.name}
            imagePath={pedal.imagePath}
            enabled={pedal.enabled}
            parameters={pedal.parameters}
            position={pedal.position}
            onParameterChange={(parameter, value) => updatePedalParameter(pedal.id, parameter, value)}
            onToggle={(enabled) => togglePedal(pedal.id, enabled)}
            onPositionChange={(position) => updatePedalPosition(pedal.id, position)}
          />
        ))}
        
        {pedals.length === 0 && (
          <div className="flex items-center justify-center h-full">
            <div className="text-center text-gray-400">
              <p className="text-lg">Add pedals from the palette to start building your board</p>
              <p className="text-sm mt-2">Click "Start Audio" to begin processing</p>
            </div>
          </div>
        )}
      </div>

      {/* Audio Status Indicator */}
      <div className="absolute bottom-4 right-4 bg-black bg-opacity-80 rounded-lg p-2">
        <div className="flex items-center space-x-2">
          <div className={`w-3 h-3 rounded-full ${isAudioStarted ? 'bg-green-500' : 'bg-red-500'}`} />
          <span className="text-white text-sm">
            {isAudioStarted ? 'Audio Active' : 'Audio Stopped'}
          </span>
        </div>
      </div>

      {/* Save Board Dialog */}
      {showSaveDialog && (
        <div className="fixed inset-0 bg-black bg-opacity-70 flex items-center justify-center z-50">
          <div className="bg-gray-800 p-6 rounded-lg max-w-md w-full mx-4">
            <h2 className="text-white text-xl font-bold mb-4">Save Pedal Board</h2>
            
            <div className="mb-4">
              <label className="block text-gray-300 text-sm mb-2">Board Name:</label>
              <input
                type="text"
                value={saveBoardName}
                onChange={(e) => setSaveBoardName(e.target.value)}
                placeholder="Enter board name..."
                className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded text-white placeholder-gray-400 focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>

            <div className="flex space-x-4">
              <button
                onClick={handleSaveToCloud}
                className="flex-1 bg-blue-600 hover:bg-blue-700 text-white py-2 px-4 rounded transition-colors"
              >
                Save to Cloud
              </button>
              <button
                onClick={() => {
                  setShowSaveDialog(false);
                  setSaveBoardName('');
                }}
                className="flex-1 bg-gray-600 hover:bg-gray-700 text-white py-2 px-4 rounded transition-colors"
              >
                Cancel
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Notification */}
      {notification && (
        <Notification
          message={notification.message}
          type={notification.type}
          onClose={() => setNotification(null)}
        />
      )}
    </div>
  );
}