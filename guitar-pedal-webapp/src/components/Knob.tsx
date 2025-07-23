'use client';

import React, { useState, useRef, useCallback } from 'react';

interface KnobProps {
  value: number;
  min: number;
  max: number;
  step?: number;
  size?: number;
  onChange: (value: number) => void;
  className?: string;
  disabled?: boolean;
}

export default function Knob({
  value,
  min,
  max,
  step = 0.01,
  size = 40,
  onChange,
  className = '',
  disabled = false
}: KnobProps) {
  const [isDragging, setIsDragging] = useState(false);
  const [dragStartY, setDragStartY] = useState(0);
  const [dragStartX, setDragStartX] = useState(0);
  const [dragStartValue, setDragStartValue] = useState(0);
  const knobRef = useRef<HTMLDivElement>(null);

  // Convert value to rotation angle (240° range, default position straight up)
  const getRotationAngle = (val: number) => {
    // Clamp the value to prevent wrapping issues
    const clampedVal = Math.max(min, Math.min(max, val));
    const normalized = (clampedVal - min) / (max - min);
    const angle = -120 + (normalized * 240); // -120° to +120°
    // Ensure angle stays within bounds to prevent wrapping
    return Math.max(-120, Math.min(120, angle));
  };

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    if (disabled) return;
    
    setIsDragging(true);
    setDragStartY(e.clientY);
    setDragStartX(e.clientX);
    setDragStartValue(value);
    
    e.preventDefault();
    e.stopPropagation();
  }, [value, disabled]);

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (!isDragging) return;
    
    const deltaX = e.clientX - dragStartX;
    const deltaY = dragStartY - e.clientY; // Up movement
    const totalDelta = deltaX + deltaY; // Combined horizontal and vertical movement
    
    const sensitivity = 0.005; // Reduced sensitivity for smoother control
    const range = max - min;
    const delta = totalDelta * sensitivity * range;
    
    let newValue = dragStartValue + delta;
    // Strictly clamp the value to prevent any wrapping
    newValue = Math.max(min, Math.min(max, newValue));
    
    // Apply step if specified and ensure it doesn't go out of bounds
    if (step > 0) {
      newValue = Math.round(newValue / step) * step;
      newValue = Math.max(min, Math.min(max, newValue)); // Clamp again after step
    }
    
    // Only update if the value actually changed and is within bounds
    if (newValue !== value && newValue >= min && newValue <= max) {
      onChange(newValue);
    }
  }, [isDragging, dragStartX, dragStartY, dragStartValue, min, max, step, onChange, value]);

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  // Add/remove mouse event listeners
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

  const rotationAngle = getRotationAngle(value);

  return (
    <div
      ref={knobRef}
      className={`relative cursor-pointer select-none ${className}`}
      style={{ width: size, height: size }}
      onMouseDown={handleMouseDown}
    >
      {/* Knob background circle */}
      <div
        className={`w-full h-full rounded-full border-2 transition-colors ${
          disabled 
            ? 'bg-gray-400 border-gray-500' 
            : isDragging 
              ? 'bg-gray-600 border-gray-400' 
              : 'bg-gray-700 border-gray-500 hover:bg-gray-600'
        }`}
        style={{
          background: disabled 
            ? '#9CA3AF' 
            : `conic-gradient(from 225deg, #374151 0deg, #4B5563 135deg, #6B7280 270deg, #374151 360deg)`
        }}
      >
        {/* White indicator that rotates - extends from rim to center */}
        <div
          className="absolute"
          style={{
            width: '2px',
            height: `${size / 2 - 2}px`, // From rim to center, accounting for border
            backgroundColor: 'white',
            borderRadius: '1px',
            top: '2px',
            left: '50%',
            transformOrigin: `1px ${size / 2 - 2}px`, // Rotate around the center point
            transform: `translateX(-1px) rotate(${rotationAngle}deg)`,
            opacity: disabled ? 0.5 : 1
          }}
        />
        
        {/* Center dot */}
        <div
          className={`absolute w-2 h-2 rounded-full ${
            disabled ? 'bg-gray-600' : 'bg-gray-800'
          }`}
          style={{
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)'
          }}
        />
      </div>
      
      {/* Visual feedback ring when dragging */}
      {isDragging && !disabled && (
        <div
          className="absolute inset-0 rounded-full border-2 border-blue-400 pointer-events-none"
          style={{ 
            width: size + 4, 
            height: size + 4, 
            left: -2, 
            top: -2 
          }}
        />
      )}
      
    </div>
  );
}