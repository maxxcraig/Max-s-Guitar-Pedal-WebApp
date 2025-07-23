'use client';

import React from 'react';
import Image from 'next/image';
import { useAuth } from '@/lib/auth-context';

interface HomeScreenProps {
  onNavigateToPedalBoard?: () => void;
  onNavigateToSavedBoards?: () => void;
  onNavigateToAbout?: () => void;
  onNavigateToLogin?: () => void;
}

export default function HomeScreen({
  onNavigateToPedalBoard,
  onNavigateToSavedBoards,
  onNavigateToAbout,
  onNavigateToLogin
}: HomeScreenProps) {
  const { user, signOut } = useAuth();

  const handleSignOut = async () => {
    try {
      await signOut();
    } catch (error) {
      console.error('Sign out error:', error);
    }
  };

  return (
    <div 
      className="min-h-screen flex flex-col items-center justify-center bg-cover bg-center bg-no-repeat relative"
      style={{ backgroundImage: 'url(/images/home-background.png)' }}
    >
      {/* Header with user info */}
      <div className="absolute top-4 right-4 bg-black bg-opacity-80 rounded-lg p-4">
        {user ? (
          <div className="flex items-center space-x-4">
            <div className="text-right">
              <p className="text-white text-sm">Welcome back!</p>
              <p className="text-gray-300 text-xs">{user.email}</p>
            </div>
            <button
              onClick={handleSignOut}
              className="bg-red-600 hover:bg-red-700 text-white text-sm px-3 py-1 rounded transition-colors"
            >
              Sign Out
            </button>
          </div>
        ) : (
          <button
            onClick={onNavigateToLogin}
            className="bg-blue-600 hover:bg-blue-700 text-white text-sm px-4 py-2 rounded transition-colors"
          >
            Sign In
          </button>
        )}
      </div>

      {/* Main content */}
      <div className="bg-black bg-opacity-80 p-12 rounded-lg shadow-lg text-center max-w-2xl">
        <h1 className="text-5xl font-bold text-white mb-4">Guitar Pedal Board</h1>
        <p className="text-xl text-gray-300 mb-12">
          Create amazing guitar sounds with virtual pedals in your browser
        </p>

        {/* Top 3 buttons */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-6">
          <button
            onClick={onNavigateToPedalBoard}
            className="bg-gradient-to-r from-blue-600 to-blue-700 hover:from-blue-700 hover:to-blue-800 text-white p-6 rounded-lg transition-all transform hover:scale-105 shadow-lg"
          >
            <div className="text-2xl font-bold mb-2">🎸 Pedal Board</div>
            <div className="text-sm opacity-90">Start creating your sound</div>
          </button>

          <button
            onClick={onNavigateToSavedBoards}
            className="bg-gradient-to-r from-green-600 to-green-700 hover:from-green-700 hover:to-green-800 text-white p-6 rounded-lg transition-all transform hover:scale-105 shadow-lg"
          >
            <div className="text-2xl font-bold mb-2">💾 Saved Boards</div>
            <div className="text-sm opacity-90">Load your configurations</div>
          </button>

          <button
            onClick={onNavigateToAbout}
            className="bg-gradient-to-r from-purple-600 to-purple-700 hover:from-purple-700 hover:to-purple-800 text-white p-6 rounded-lg transition-all transform hover:scale-105 shadow-lg"
          >
            <div className="text-2xl font-bold mb-2">ℹ️ About</div>
            <div className="text-sm opacity-90">Learn about the app</div>
          </button>
        </div>

        {/* Bottom 2 buttons - centered */}
        <div className="flex justify-center gap-6">
          <button
            className="bg-gradient-to-r from-gray-600 to-gray-700 hover:from-gray-700 hover:to-gray-800 text-white p-6 rounded-lg transition-all transform hover:scale-105 shadow-lg w-64"
            onClick={() => window.open('https://github.com/maxxcraig/Max-s-Guitar-Pedal-WebApp', '_blank')}
          >
            <div className="text-2xl font-bold mb-2">⭐ GitHub</div>
            <div className="text-sm opacity-90">View source code</div>
          </button>

          <button
            className="bg-gradient-to-r from-orange-600 to-orange-700 hover:from-orange-700 hover:to-orange-800 text-white p-6 rounded-lg transition-all transform hover:scale-105 shadow-lg w-64"
            onClick={() => window.open('https://maxcraig.itch.io/maxs-guitar-pedal-app', '_blank')}
          >
            <div className="text-2xl font-bold mb-2">💻 Original App Download</div>
            <div className="text-sm opacity-90">(your OS may block you opening)</div>
          </button>
        </div>

        <div className="mt-12 text-center">
          <p className="text-gray-400 text-sm mb-4">
            Real-time audio processing powered by custom DSP algorithms
          </p>
          <div className="flex justify-center space-x-8 text-xs text-gray-500">
            <span>🎚️ Professional Effects</span>
            <span>🔊 Low Latency</span>
            <span>💾 Cloud Save</span>
            <span>🎯 Easy to Use</span>
          </div>
        </div>
      </div>

      {/* Footer */}
      <div className="absolute bottom-4 left-0 right-0 text-center">
        <p className="text-gray-400 text-sm">
          Guitar Pedal App v2.0 - Web Edition
        </p>
      </div>
    </div>
  );
}