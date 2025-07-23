'use client';

import React, { useState } from 'react';
import { AuthProvider } from '@/lib/auth-context';
import HomeScreen from '@/components/HomeScreen';
import LoginScreen from '@/components/LoginScreen';
import SignupScreen from '@/components/SignupScreen';
import PedalBoard from '@/components/PedalBoard';
import SavedBoardsScreen from '@/components/SavedBoardsScreen';

type Screen = 'home' | 'login' | 'signup' | 'pedalboard' | 'savedboards' | 'about';

export default function GuitarPedalApp() {
  const [currentScreen, setCurrentScreen] = useState<Screen>('home');
  const [boardToLoad, setBoardToLoad] = useState<any>(null);

  const renderScreen = () => {
    switch (currentScreen) {
      case 'login':
        return (
          <LoginScreen
            onSwitchToSignup={() => setCurrentScreen('signup')}
            onLoginSuccess={() => setCurrentScreen('home')}
          />
        );
      
      case 'signup':
        return (
          <SignupScreen
            onSwitchToLogin={() => setCurrentScreen('login')}
            onSignupSuccess={() => setCurrentScreen('home')}
          />
        );
      
      case 'pedalboard':
        return (
          <PedalBoard
            onBackToHome={() => setCurrentScreen('home')}
            boardToLoad={boardToLoad}
            onBoardLoaded={() => setBoardToLoad(null)}
          />
        );
      
      case 'savedboards':
        return (
          <SavedBoardsScreen
            onBackToHome={() => setCurrentScreen('home')}
            onLoadBoard={(boardData) => {
              setBoardToLoad(boardData);
              setCurrentScreen('pedalboard');
            }}
          />
        );
      
      case 'about':
        return (
          <div 
            className="min-h-screen bg-cover bg-center bg-no-repeat flex items-center justify-center"
            style={{ backgroundImage: 'url(/images/about.png)' }}
          >
            <div className="bg-black bg-opacity-80 p-8 rounded-lg max-w-2xl">
              <h1 className="text-white text-3xl font-bold mb-6">About Guitar Pedal App</h1>
              <div className="text-gray-300 space-y-4">
                <p>
                  A web-based guitar effects processor that brings professional-grade pedal effects 
                  to your browser. Built with modern web technologies for real-time audio processing.
                </p>
                
                <h2 className="text-xl font-semibold text-white">Features:</h2>
                <ul className="list-disc list-inside space-y-2">
                  <li>Real-time audio processing with Web Audio API</li>
                  <li>Professional guitar effects (Overdrive, Reverb, Distortion, etc.)</li>
                  <li>Drag-and-drop pedal board interface</li>
                  <li>Save and load custom configurations</li>
                  <li>Cross-platform compatibility</li>
                  <li>Low latency audio processing</li>
                </ul>
                
                <h2 className="text-xl font-semibold text-white">Technology:</h2>
                <ul className="list-disc list-inside space-y-2">
                  <li>React with TypeScript</li>
                  <li>Web Audio API for real-time processing</li>
                  <li>AudioWorklet for low-latency effects</li>
                  <li>Supabase for authentication and storage</li>
                  <li>Tailwind CSS for styling</li>
                </ul>
                
                <p className="text-sm text-gray-400 mt-6">
                  Originally built with JUCE C++, now reimagined for the web.
                </p>
              </div>
              
              <button
                onClick={() => setCurrentScreen('home')}
                className="mt-6 bg-blue-600 hover:bg-blue-700 text-white px-6 py-2 rounded transition-colors"
              >
                Back to Home
              </button>
            </div>
          </div>
        );
      
      default:
        return (
          <HomeScreen
            onNavigateToPedalBoard={() => setCurrentScreen('pedalboard')}
            onNavigateToSavedBoards={() => setCurrentScreen('savedboards')}
            onNavigateToAbout={() => setCurrentScreen('about')}
            onNavigateToLogin={() => setCurrentScreen('login')}
          />
        );
    }
  };

  return (
    <AuthProvider>
      <main className="min-h-screen">
        {renderScreen()}
      </main>
    </AuthProvider>
  );
}
