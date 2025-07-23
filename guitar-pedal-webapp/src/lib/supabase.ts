import { createClient } from '@supabase/supabase-js';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL || '';
const supabaseKey = process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY || '';

if (!supabaseUrl || !supabaseKey) {
  console.warn('Supabase credentials not found. Authentication features will be disabled.');
}

export const supabase = createClient(supabaseUrl, supabaseKey);

export interface AuthResponse {
  success: boolean;
  errorMessage?: string;
  userId?: string;
  accessToken?: string;
  email?: string;
}

export interface BoardData {
  id: string;
  name: string;
  board_data: any;
  created_at: string;
  updated_at: string;
  user_id: string;
}

export interface BoardResponse {
  success: boolean;
  errorMessage?: string;
  boards?: BoardData[];
}

export class SupabaseService {
  async signUp(email: string, password: string): Promise<AuthResponse> {
    try {
      const { data, error } = await supabase.auth.signUp({
        email,
        password
      });

      if (error) {
        return {
          success: false,
          errorMessage: error.message
        };
      }

      if (data.user && data.session) {
        return {
          success: true,
          userId: data.user.id,
          accessToken: data.session.access_token,
          email: data.user.email || ''
        };
      }

      return {
        success: false,
        errorMessage: 'Signup failed - no user data returned'
      };
    } catch (error) {
      return {
        success: false,
        errorMessage: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  async signIn(email: string, password: string): Promise<AuthResponse> {
    try {
      const { data, error } = await supabase.auth.signInWithPassword({
        email,
        password
      });

      if (error) {
        return {
          success: false,
          errorMessage: error.message
        };
      }

      if (data.user && data.session) {
        return {
          success: true,
          userId: data.user.id,
          accessToken: data.session.access_token,
          email: data.user.email || ''
        };
      }

      return {
        success: false,
        errorMessage: 'Login failed - no user data returned'
      };
    } catch (error) {
      return {
        success: false,
        errorMessage: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  async signOut(): Promise<void> {
    await supabase.auth.signOut();
  }

  async getCurrentUser() {
    const { data: { user } } = await supabase.auth.getUser();
    return user;
  }

  async saveBoard(boardName: string, boardData: any): Promise<BoardResponse> {
    try {
      const user = await this.getCurrentUser();
      if (!user) {
        return {
          success: false,
          errorMessage: 'User not authenticated'
        };
      }

      // Check if board already exists
      const { data: existing } = await supabase
        .from('user_boards')
        .select('id')
        .eq('user_id', user.id)
        .eq('name', boardName)
        .single();

      let result;
      if (existing) {
        // Update existing board
        result = await supabase
          .from('user_boards')
          .update({
            board_data: boardData,
            updated_at: new Date().toISOString()
          })
          .eq('id', existing.id)
          .select();
      } else {
        // Insert new board
        result = await supabase
          .from('user_boards')
          .insert({
            user_id: user.id,
            name: boardName,
            board_data: boardData,
            created_at: new Date().toISOString(),
            updated_at: new Date().toISOString()
          })
          .select();
      }

      if (result.error) {
        return {
          success: false,
          errorMessage: result.error.message
        };
      }

      return {
        success: true,
        boards: result.data
      };
    } catch (error) {
      return {
        success: false,
        errorMessage: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  async loadBoards(): Promise<BoardResponse> {
    try {
      const user = await this.getCurrentUser();
      if (!user) {
        return {
          success: false,
          errorMessage: 'User not authenticated'
        };
      }

      const { data, error } = await supabase
        .from('user_boards')
        .select('*')
        .eq('user_id', user.id)
        .order('updated_at', { ascending: false });

      if (error) {
        return {
          success: false,
          errorMessage: error.message
        };
      }

      return {
        success: true,
        boards: data || []
      };
    } catch (error) {
      return {
        success: false,
        errorMessage: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  async deleteBoard(boardId: string): Promise<BoardResponse> {
    try {
      const user = await this.getCurrentUser();
      if (!user) {
        return {
          success: false,
          errorMessage: 'User not authenticated'
        };
      }

      const { error } = await supabase
        .from('user_boards')
        .delete()
        .eq('id', boardId)
        .eq('user_id', user.id);

      if (error) {
        return {
          success: false,
          errorMessage: error.message
        };
      }

      return {
        success: true
      };
    } catch (error) {
      return {
        success: false,
        errorMessage: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  isValidEmail(email: string): boolean {
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return emailRegex.test(email);
  }

  isValidPassword(password: string): boolean {
    return password.length >= 6;
  }

  // Listen to auth state changes
  onAuthStateChange(callback: (event: string, session: any) => void) {
    return supabase.auth.onAuthStateChange(callback);
  }
}