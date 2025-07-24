'use client';

import React, { useState, useRef, useCallback } from 'react';
import Image from 'next/image';
import Knob from './Knob';

export interface PedalParameters {
  [key: string]: number;
}

export interface PedalComponentProps {
  id: string;
  type: string;
  name: string;
  imagePath: string;
  enabled: boolean;
  parameters: PedalParameters;
  position: { x: number; y: number };
  onParameterChange: (parameter: string, value: number) => void;
  onToggle: (enabled: boolean) => void;
  onPositionChange: (position: { x: number; y: number }) => void;
  className?: string;
  draggable?: boolean;
}

export default function PedalComponent({
  id,
  type,
  name,
  imagePath,
  enabled,
  parameters,
  position,
  onParameterChange,
  onToggle,
  onPositionChange,
  className = '',
  draggable = true
}: PedalComponentProps) {
  const [isDragging, setIsDragging] = useState(false);
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });
  const pedalRef = useRef<HTMLDivElement>(null);

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    if (!draggable) return;
    
    // Don't start dragging if clicking on interactive elements
    const target = e.target as HTMLElement;
    if (target.closest('.knob-component') || target.closest('button')) {
      return;
    }
    
    const rect = pedalRef.current?.getBoundingClientRect();
    if (rect) {
      setDragOffset({
        x: e.clientX - rect.left,
        y: e.clientY - rect.top
      });
      setIsDragging(true);
    }
    
    e.preventDefault();
  }, [draggable]);

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (!isDragging || !draggable) return;
    
    const newPosition = {
      x: e.clientX - dragOffset.x,
      y: e.clientY - dragOffset.y
    };
    
    onPositionChange(newPosition);
  }, [isDragging, dragOffset, onPositionChange, draggable]);

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  React.useEffect(() => {
    if (isDragging) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
      
      return () => {
        document.removeEventListener('mousemove', handleMouseMove);
        document.removeEventListener('mouseup', handleMouseUp);
      };
    }
  }, [isDragging, handleMouseMove, handleMouseUp]);

  // Render knobs based on pedal type
  const renderKnobs = () => {
    switch (type) {
      case 'overdrive':
      case 'distortion':
      case 'bluesdriver':
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.gain !== undefined ? parameters.gain : 4.05}
                min={0.1}
                max={type === 'distortion' ? 10.0 : type === 'bluesdriver' ? 6.0 : 8.0}
                step={0.1}
                size={45}
                onChange={(value) => onParameterChange('gain', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 211, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.level !== undefined ? parameters.level : 1.0}
                min={0.0}
                max={2.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('level', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 180, top: 42, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.tone !== undefined ? parameters.tone : 5.0}
                min={0.0}
                max={10.0}
                step={0.1}
                size={37}
                onChange={(value) => onParameterChange('tone', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );
      
      case 'reverb':
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.mix !== undefined ? parameters.mix : 0.5}
                min={0.0}
                max={1.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('mix', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 211, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.decay !== undefined ? parameters.decay : 0.475}
                min={0.1}
                max={0.85}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('decay', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 180, top: 42, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.tone !== undefined ? parameters.tone : 0.5}
                min={0.0}
                max={1.0}
                step={0.01}
                size={37}
                onChange={(value) => onParameterChange('tone', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );

      case 'chorus':
      case 'phaser':
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.rate !== undefined ? parameters.rate : 0.5}
                min={0.1}
                max={5.0}
                step={0.1}
                size={45}
                onChange={(value) => onParameterChange('rate', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 211, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.depth !== undefined ? parameters.depth : 0.3}
                min={0.0}
                max={1.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('depth', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 180, top: 42, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.feedback !== undefined ? parameters.feedback : 0.2}
                min={0.0}
                max={type === 'phaser' ? 0.95 : 0.7}
                step={0.01}
                size={37}
                onChange={(value) => onParameterChange('feedback', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );

      case 'delay':
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.time !== undefined ? parameters.time : 0.3}
                min={0.01}
                max={2.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('time', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 211, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.feedback !== undefined ? parameters.feedback : 0.4}
                min={0.0}
                max={0.95}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('feedback', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 180, top: 42, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.mix !== undefined ? parameters.mix : 0.3}
                min={0.0}
                max={1.0}
                step={0.01}
                size={37}
                onChange={(value) => onParameterChange('mix', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );

      case 'tremolo':
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.rate !== undefined ? parameters.rate : 4.0}
                min={0.1}
                max={20.0}
                step={0.1}
                size={45}
                onChange={(value) => onParameterChange('rate', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 211, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.depth !== undefined ? parameters.depth : 0.5}
                min={0.0}
                max={1.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('depth', value)}
                disabled={!enabled}
              />
            </div>
            <div className="absolute knob-component" style={{ left: 180, top: 42, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.level !== undefined ? parameters.level : 1.0}
                min={0.0}
                max={2.0}
                step={0.01}
                size={37}
                onChange={(value) => onParameterChange('level', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );
      
      default:
        // Fallback for unknown pedal types
        return (
          <>
            <div className="absolute knob-component" style={{ left: 140, top: 11, pointerEvents: 'auto' }}>
              <Knob
                value={parameters.level !== undefined ? parameters.level : 0.5}
                min={0.0}
                max={1.0}
                step={0.01}
                size={45}
                onChange={(value) => onParameterChange('level', value)}
                disabled={!enabled}
              />
            </div>
          </>
        );
    }
  };

  return (
    <div
      ref={pedalRef}
      className={`absolute ${className} ${isDragging ? 'z-50' : 'z-10'}`}
      style={{
        left: position.x,
        top: position.y,
        width: '396px',
        height: '264px',
        pointerEvents: 'none'
      }}
    >
      {/* Pedal background image with precise hitbox */}
      <Image
        src={imagePath}
        alt={name}
        width={396}
        height={264}
        className="absolute top-0 left-0 w-full h-full object-contain"
        draggable={false}
        onMouseDown={handleMouseDown}
        style={{ 
          cursor: draggable ? 'move' : 'default',
          pointerEvents: 'auto',
          filter: enabled ? 'none' : 'grayscale(1) brightness(0.7)'
        }}
      />
        
      {/* Knobs overlay */}
      {renderKnobs()}
      
      {/* Toggle button */}
      <button
        className={`absolute w-[132px] h-[85px] border-2 rounded-lg font-bold text-sm transition-all ${
          enabled
            ? 'bg-green-500 border-green-600 text-white shadow-lg'
            : 'bg-gray-400 border-gray-500 text-gray-700'
        } hover:opacity-80 active:scale-95`}
        style={{ 
          left: 132, 
          bottom: 16,
          pointerEvents: 'auto'
        }}
        onClick={(e) => {
          e.stopPropagation();
          onToggle(!enabled);
        }}
      >
        {enabled ? 'ON' : 'OFF'}
      </button>
      
      {/* Drag indicator when dragging */}
      {isDragging && (
        <div className="absolute inset-0 border-2 border-dashed border-blue-400 rounded-lg pointer-events-none" />
      )}
    </div>
  );
}