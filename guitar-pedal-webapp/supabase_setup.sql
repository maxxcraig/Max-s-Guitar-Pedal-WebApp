-- Create pedal_boards table for storing user pedal board configurations

CREATE TABLE IF NOT EXISTS pedal_boards (
    id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    user_id UUID REFERENCES auth.users(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    board_data JSONB NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(user_id, name)
);

-- Enable RLS (Row Level Security)
ALTER TABLE pedal_boards ENABLE ROW LEVEL SECURITY;

-- Create policies
CREATE POLICY "Users can view their own boards"
    ON pedal_boards FOR SELECT
    USING (auth.uid() = user_id);

CREATE POLICY "Users can insert their own boards"
    ON pedal_boards FOR INSERT
    WITH CHECK (auth.uid() = user_id);

CREATE POLICY "Users can update their own boards"
    ON pedal_boards FOR UPDATE
    USING (auth.uid() = user_id);

CREATE POLICY "Users can delete their own boards"
    ON pedal_boards FOR DELETE
    USING (auth.uid() = user_id);

-- Create an index for faster queries
CREATE INDEX IF NOT EXISTS idx_pedal_boards_user_id ON pedal_boards(user_id);
CREATE INDEX IF NOT EXISTS idx_pedal_boards_updated_at ON pedal_boards(updated_at DESC);