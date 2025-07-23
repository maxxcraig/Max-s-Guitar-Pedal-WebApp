'use client';

import React, { useState, useEffect } from 'react';
import { useAuth } from '@/lib/auth-context';
import { SupabaseService, BoardData } from '@/lib/supabase';
import Notification from './Notification';
import ConfirmDialog from './ConfirmDialog';

interface SavedBoardsScreenProps {
  onBackToHome?: () => void;
  onLoadBoard?: (boardData: any) => void;
}

export default function SavedBoardsScreen({ onBackToHome, onLoadBoard }: SavedBoardsScreenProps) {
  const [cloudBoards, setCloudBoards] = useState<BoardData[]>([]);
  const [localBoards, setLocalBoards] = useState<any[]>([]);
  const [loading, setLoading] = useState(false);
  const [supabaseService] = useState(() => new SupabaseService());
  const [notification, setNotification] = useState<{message: string; type: 'success' | 'error' | 'info'} | null>(null);
  const [confirmDialog, setConfirmDialog] = useState<{isOpen: boolean; title: string; message: string; onConfirm: () => void} | null>(null);
  const { user } = useAuth();

  useEffect(() => {
    loadBoards();
  }, [user]);

  const loadBoards = async () => {
    setLoading(true);

    // Load local boards from localStorage
    const localData = JSON.parse(localStorage.getItem('pedalBoards') || '[]');
    setLocalBoards(localData);

    // Load cloud boards if user is authenticated
    if (user) {
      try {
        const response = await supabaseService.loadBoards();
        if (response.success && response.boards) {
          setCloudBoards(response.boards);
        } else {
          console.error('Failed to load cloud boards:', response.errorMessage);
        }
      } catch (error) {
        console.error('Error loading cloud boards:', error);
      }
    }

    setLoading(false);
  };

  const deleteCloudBoard = async (boardId: string) => {
    setConfirmDialog({
      isOpen: true,
      title: 'Delete Board',
      message: 'Are you sure you want to delete this board? This action cannot be undone.',
      onConfirm: async () => {
        try {
          const response = await supabaseService.deleteBoard(boardId);
          if (response.success) {
            setCloudBoards(prev => prev.filter(board => board.id !== boardId));
            setNotification({ message: 'Board deleted successfully!', type: 'success' });
          } else {
            setNotification({ message: `Failed to delete board: ${response.errorMessage}`, type: 'error' });
          }
        } catch (error) {
          setNotification({ message: 'Failed to delete board', type: 'error' });
          console.error('Delete error:', error);
        }
        setConfirmDialog(null);
      }
    });
  };

  const deleteLocalBoard = (index: number) => {
    setConfirmDialog({
      isOpen: true,
      title: 'Delete Local Board',
      message: 'Are you sure you want to delete this local board? This action cannot be undone.',
      onConfirm: () => {
        const updatedBoards = localBoards.filter((_, i) => i !== index);
        setLocalBoards(updatedBoards);
        localStorage.setItem('pedalBoards', JSON.stringify(updatedBoards));
        setNotification({ message: 'Local board deleted!', type: 'success' });
        setConfirmDialog(null);
      }
    });
  };

  const loadCloudBoard = (board: BoardData) => {
    if (onLoadBoard) {
      onLoadBoard({
        effects: board.board_data.effects || [],
        inputGain: board.board_data.inputGain || 0.3,
        masterBypass: board.board_data.masterBypass || false
      });
    }
  };

  const loadLocalBoard = (board: any) => {
    if (onLoadBoard) {
      onLoadBoard({
        effects: board.effects || [],
        inputGain: board.inputGain || 0.3,
        masterBypass: board.masterBypass || false
      });
    }
  };

  return (
    <div 
      className="min-h-screen bg-cover bg-center bg-no-repeat"
      style={{ backgroundImage: 'url(/images/home-background.png)' }}
    >
      <div className="min-h-screen bg-black bg-opacity-60 p-8">
        <div className="max-w-6xl mx-auto">
          {/* Header */}
          <div className="flex items-center justify-between mb-8">
            <h1 className="text-4xl font-bold text-white">Saved Pedal Boards</h1>
            <button
              onClick={onBackToHome}
              className="bg-gray-600 hover:bg-gray-700 text-white px-6 py-2 rounded-lg transition-colors"
            >
              ← Back to Home
            </button>
          </div>

          {loading && (
            <div className="text-center text-white text-lg">Loading boards...</div>
          )}

          <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
            {/* Cloud Boards */}
            {user && (
              <div className="bg-black bg-opacity-80 rounded-lg p-6">
                <h2 className="text-2xl font-semibold text-white mb-4 flex items-center">
                  ☁️ Cloud Boards
                  <span className="ml-2 text-sm text-gray-400">({cloudBoards.length})</span>
                </h2>
                
                {cloudBoards.length === 0 ? (
                  <p className="text-gray-400">No cloud boards saved yet. Create and save a board to see it here!</p>
                ) : (
                  <div className="space-y-4">
                    {cloudBoards.map((board) => (
                      <div key={board.id} className="bg-gray-700 rounded-lg p-4">
                        <div className="flex items-center justify-between mb-2">
                          <h3 className="text-lg font-semibold text-white">{board.name}</h3>
                          <div className="flex space-x-2">
                            <button
                              onClick={() => loadCloudBoard(board)}
                              className="bg-blue-600 hover:bg-blue-700 text-white px-3 py-1 rounded text-sm transition-colors"
                            >
                              Load
                            </button>
                            <button
                              onClick={() => deleteCloudBoard(board.id)}
                              className="bg-red-600 hover:bg-red-700 text-white px-3 py-1 rounded text-sm transition-colors"
                            >
                              Delete
                            </button>
                          </div>
                        </div>
                        
                        <div className="text-sm text-gray-300">
                          <p>Effects: {board.board_data?.effects?.length || 0} pedals</p>
                          <p>Created: {new Date(board.created_at).toLocaleDateString()}</p>
                          <p>Updated: {new Date(board.updated_at).toLocaleDateString()}</p>
                        </div>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            )}

            {/* Local Boards */}
            <div className="bg-black bg-opacity-80 rounded-lg p-6">
              <h2 className="text-2xl font-semibold text-white mb-4 flex items-center">
                💾 Local Boards
                <span className="ml-2 text-sm text-gray-400">({localBoards.length})</span>
              </h2>
              
              {localBoards.length === 0 ? (
                <p className="text-gray-400">No local boards saved. Boards are saved locally when not signed in.</p>
              ) : (
                <div className="space-y-4">
                  {localBoards.map((board, index) => (
                    <div key={index} className="bg-gray-700 rounded-lg p-4">
                      <div className="flex items-center justify-between mb-2">
                        <h3 className="text-lg font-semibold text-white">{board.name}</h3>
                        <div className="flex space-x-2">
                          <button
                            onClick={() => loadLocalBoard(board)}
                            className="bg-blue-600 hover:bg-blue-700 text-white px-3 py-1 rounded text-sm transition-colors"
                          >
                            Load
                          </button>
                          <button
                            onClick={() => deleteLocalBoard(index)}
                            className="bg-red-600 hover:bg-red-700 text-white px-3 py-1 rounded text-sm transition-colors"
                          >
                            Delete
                          </button>
                        </div>
                      </div>
                      
                      <div className="text-sm text-gray-300">
                        <p>Effects: {board.effects?.length || 0} pedals</p>
                        <p>Saved locally on this device</p>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>

          {/* Sign in prompt for guests */}
          {!user && (
            <div className="mt-8 bg-blue-600 bg-opacity-20 border border-blue-600 rounded-lg p-6 text-center">
              <h3 className="text-xl font-semibold text-white mb-2">Save to the Cloud</h3>
              <p className="text-blue-200 mb-4">
                Sign in to save your pedal boards to the cloud and access them from any device!
              </p>
              <button
                onClick={onBackToHome}
                className="bg-blue-600 hover:bg-blue-700 text-white px-6 py-2 rounded-lg transition-colors"
              >
                Sign In
              </button>
            </div>
          )}
        </div>
      </div>

      {/* Notification */}
      {notification && (
        <Notification
          message={notification.message}
          type={notification.type}
          onClose={() => setNotification(null)}
        />
      )}

      {/* Confirm Dialog */}
      {confirmDialog && (
        <ConfirmDialog
          isOpen={confirmDialog.isOpen}
          title={confirmDialog.title}
          message={confirmDialog.message}
          onConfirm={confirmDialog.onConfirm}
          onCancel={() => setConfirmDialog(null)}
          confirmText="Delete"
          confirmButtonClass="bg-red-600 hover:bg-red-700"
        />
      )}
    </div>
  );
}