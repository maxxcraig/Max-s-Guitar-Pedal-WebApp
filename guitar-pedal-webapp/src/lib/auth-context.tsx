'use client';

import React, { createContext, useContext, useEffect, useState } from 'react';
import { SupabaseService, AuthResponse } from './supabase';

interface User {
  id: string;
  email: string;
  accessToken: string;
}

interface AuthContextType {
  user: User | null;
  loading: boolean;
  signIn: (email: string, password: string) => Promise<AuthResponse>;
  signUp: (email: string, password: string) => Promise<AuthResponse>;
  signOut: () => Promise<void>;
}

const AuthContext = createContext<AuthContextType | undefined>(undefined);

export function useAuth() {
  const context = useContext(AuthContext);
  if (context === undefined) {
    throw new Error('useAuth must be used within an AuthProvider');
  }
  return context;
}

interface AuthProviderProps {
  children: React.ReactNode;
}

export function AuthProvider({ children }: AuthProviderProps) {
  const [user, setUser] = useState<User | null>(null);
  const [loading, setLoading] = useState(true);
  const [supabaseService] = useState(() => new SupabaseService());

  useEffect(() => {
    // Check for existing session
    const checkSession = async () => {
      try {
        const currentUser = await supabaseService.getCurrentUser();
        if (currentUser) {
          setUser({
            id: currentUser.id,
            email: currentUser.email || '',
            accessToken: '' // Will be refreshed as needed
          });
        }
      } catch (error) {
        console.error('Error checking session:', error);
      } finally {
        setLoading(false);
      }
    };

    checkSession();

    // Listen for auth state changes
    const { data: { subscription } } = supabaseService.onAuthStateChange(
      async (event, session) => {
        if (event === 'SIGNED_IN' && session) {
          setUser({
            id: session.user.id,
            email: session.user.email || '',
            accessToken: session.access_token
          });
        } else if (event === 'SIGNED_OUT') {
          setUser(null);
        }
        setLoading(false);
      }
    );

    return () => {
      subscription.unsubscribe();
    };
  }, [supabaseService]);

  const signIn = async (email: string, password: string): Promise<AuthResponse> => {
    setLoading(true);
    try {
      const response = await supabaseService.signIn(email, password);
      if (response.success && response.userId && response.email && response.accessToken) {
        setUser({
          id: response.userId,
          email: response.email,
          accessToken: response.accessToken
        });
      }
      return response;
    } finally {
      setLoading(false);
    }
  };

  const signUp = async (email: string, password: string): Promise<AuthResponse> => {
    setLoading(true);
    try {
      const response = await supabaseService.signUp(email, password);
      if (response.success && response.userId && response.email && response.accessToken) {
        setUser({
          id: response.userId,
          email: response.email,
          accessToken: response.accessToken
        });
      }
      return response;
    } finally {
      setLoading(false);
    }
  };

  const signOut = async (): Promise<void> => {
    setLoading(true);
    try {
      await supabaseService.signOut();
      setUser(null);
    } finally {
      setLoading(false);
    }
  };

  const value: AuthContextType = {
    user,
    loading,
    signIn,
    signUp,
    signOut
  };

  return (
    <AuthContext.Provider value={value}>
      {children}
    </AuthContext.Provider>
  );
}