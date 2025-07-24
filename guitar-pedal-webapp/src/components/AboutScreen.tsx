'use client';

import React from 'react';
import Image from 'next/image';

interface AboutScreenProps {
  onBack?: () => void;
}

export default function AboutScreen({ onBack }: AboutScreenProps) {
  return (
    <div 
      className="min-h-screen flex flex-col items-center justify-center bg-cover bg-center bg-no-repeat relative"
      style={{ backgroundImage: 'url(/images/home-background.png)' }}
    >
      {/* Back button */}
      <div className="absolute top-4 left-4">
        <button
          onClick={onBack}
          className="bg-black bg-opacity-80 hover:bg-opacity-100 text-white px-4 py-2 rounded-lg transition-all transform hover:scale-105 shadow-lg"
        >
          ← Back to Home
        </button>
      </div>

      {/* Main content */}
      <div className="bg-black bg-opacity-80 p-12 rounded-lg shadow-lg text-center max-w-4xl mx-4">
        <h1 className="text-5xl font-bold text-white mb-8">About</h1>
        
        {/* App description */}
        <div className="text-left space-y-6 mb-12">
          <div className="bg-gray-800 bg-opacity-50 p-6 rounded-lg">
            <h2 className="text-2xl font-bold text-white mb-4">My Guitar</h2>
            <p className="text-gray-300 leading-relaxed">
              Max's guitar pedal board is a real-time audio processing application that brings the experience 
              of classic guitar pedals to your browser. Built with modern web technologies, it features 
              professional-grade DSP algorithms for authentic sound processing.
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div className="bg-blue-800 bg-opacity-50 p-6 rounded-lg">
              <h3 className="text-xl font-bold text-white mb-3">Features</h3>
              <ul className="text-gray-300 space-y-2 text-sm">
                <li>• Real-time audio processing</li>
                <li>• Multiple guitar effects</li>
                <li>• Drag & drop pedal arrangement</li>
                <li>• Save & load configurations</li>
                <li>• Low-latency performance</li>
              </ul>
            </div>

            <div className="bg-green-800 bg-opacity-50 p-6 rounded-lg">
              <h3 className="text-xl font-bold text-white mb-3">Available Effects</h3>
              <ul className="text-gray-300 space-y-2 text-sm">
                <li>• Overdrive & Distortion</li>
                <li>• Reverb & Delay</li>
                <li>• Chorus & Phaser</li>
                <li>• Tremolo & Blues Driver</li>
                <li>• Custom parameter controls</li>
              </ul>
            </div>
          </div>

          <div className="bg-purple-800 bg-opacity-50 p-6 rounded-lg">
            <h3 className="text-xl font-bold text-white mb-3">Technology Stack</h3>
            <p className="text-gray-300 text-sm leading-relaxed">
              Built with React, Next.js, TypeScript, and Web Audio API. The application uses custom DSP 
              algorithms for authentic guitar effect processing, with Supabase for user authentication 
              and cloud storage of pedal board configurations.
            </p>
          </div>
        </div>

        {/* Developer section with image */}
        <div className="mb-8">
          <div className="flex flex-col items-center">
            <div className="relative mb-4">
              <Image
                src="/images/max.png"
                alt="Max"
                width={600}
                height={600}
                className="rounded-full shadow-lg border-4 border-white"
                priority
              />
            </div>
            <p className="text-2xl font-semibold text-white mb-2">Me!</p>
          </div>
        </div>

      </div>

    </div>
  );
}