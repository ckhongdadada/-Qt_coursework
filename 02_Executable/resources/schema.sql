PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS courses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    code TEXT,
    credits REAL DEFAULT 0,
    semester TEXT,
    category TEXT DEFAULT 'Required',
    score REAL,
    grade_point REAL,
    status TEXT DEFAULT 'Planned',
    teacher TEXT,
    location TEXT,
    description TEXT,
    tags TEXT,
    major_name TEXT,
    section TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS target_jobs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    company TEXT,
    location TEXT,
    salary_range TEXT,
    description TEXT,
    requirements TEXT,
    is_active INTEGER DEFAULT 1,
    priority TEXT DEFAULT 'Medium',
    source TEXT,
    url TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT DEFAULT 'Other',
    organization TEXT,
    description TEXT,
    start_date TEXT,
    end_date TEXT,
    is_active INTEGER DEFAULT 1,
    achievements TEXT,
    contact TEXT,
    supervisor TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS achievements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT DEFAULT 'Other',
    level TEXT DEFAULT 'Other',
    organization TEXT,
    description TEXT,
    date TEXT,
    certificate TEXT,
    related_course TEXT,
    team_members TEXT,
    ranking INTEGER DEFAULT 0,
    prize TEXT,
    verified INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS experiences (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT DEFAULT 'Other',
    organization TEXT,
    role TEXT,
    description TEXT,
    start_date TEXT,
    end_date TEXT,
    is_ongoing INTEGER DEFAULT 0,
    technologies TEXT,
    achievements TEXT,
    supervisor TEXT,
    contact TEXT,
    location TEXT,
    url TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS activities (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    category TEXT,
    start_date TEXT,
    end_date TEXT,
    is_favorite INTEGER DEFAULT 0,
    is_active INTEGER DEFAULT 1,
    tags TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS goals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    category TEXT DEFAULT 'General',
    description TEXT,
    target_value REAL NOT NULL DEFAULT 0,
    current_value REAL DEFAULT 0,
    unit TEXT DEFAULT '项',
    deadline TEXT,
    priority TEXT DEFAULT 'Medium',
    status TEXT DEFAULT 'In Progress',
    milestones TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS peer_benchmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    major TEXT,
    semester TEXT,
    gpa REAL DEFAULT 0,
    credits REAL DEFAULT 0,
    achievements_count INTEGER DEFAULT 0,
    experiences_count INTEGER DEFAULT 0,
    note TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    display_name TEXT,
    role TEXT DEFAULT 'user',
    is_active INTEGER DEFAULT 1,
    last_login_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS job_requirements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    job_id INTEGER NOT NULL,
    skill_name TEXT NOT NULL,
    importance TEXT DEFAULT 'Required',
    proficiency TEXT DEFAULT 'Intermediate',
    FOREIGN KEY (job_id) REFERENCES target_jobs(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS curriculum_requirements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    major_name TEXT NOT NULL,
    section_name TEXT NOT NULL,
    section_type TEXT NOT NULL DEFAULT 'Required',
    required_credits REAL DEFAULT 0,
    completed_credits REAL DEFAULT 0,
    total_credits REAL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 为已有表添加新字段（SQLite 忽略已存在的列）
ALTER TABLE courses ADD COLUMN major_name TEXT;
ALTER TABLE courses ADD COLUMN section TEXT;
