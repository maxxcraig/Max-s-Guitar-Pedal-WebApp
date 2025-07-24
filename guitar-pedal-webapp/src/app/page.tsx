'use client';

import React, { useState } from 'react';
import { AuthProvider } from '@/lib/auth-context';
import HomeScreen from '@/components/HomeScreen';
import LoginScreen from '@/components/LoginScreen';
import SignupScreen from '@/components/SignupScreen';
import PedalBoard from '@/components/PedalBoard';
import SavedBoardsScreen from '@/components/SavedBoardsScreen';
import AboutScreen from '@/components/AboutScreen';

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
          <AboutScreen onBack={() => setCurrentScreen('home')} />
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
