-- set up for supabase 
DROP TABLE IF EXISTS user_boards;

-- Create user_boards table
CREATE TABLE user_boards (
    id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES auth.users(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    board_data JSONB NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(user_id, name)
);

-- Enable Row Level Security
ALTER TABLE user_boards ENABLE ROW LEVEL SECURITY;

-- Create policies for users to only access their own boards
CREATE POLICY "Users can view their own boards" ON user_boards
    FOR SELECT USING (auth.uid() = user_id);

CREATE POLICY "Users can insert their own boards" ON user_boards
    FOR INSERT WITH CHECK (auth.uid() = user_id);

CREATE POLICY "Users can update their own boards" ON user_boards
    FOR UPDATE USING (auth.uid() = user_id);

CREATE POLICY "Users can delete their own boards" ON user_boards
    FOR DELETE USING (auth.uid() = user_id);

-- Create function to automatically update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Create trigger to automatically update updated_at
CREATE TRIGGER update_user_boards_updated_at 
    BEFORE UPDATE ON user_boards 
    FOR EACH ROW 
    EXECUTE FUNCTION update_updated_at_column();