import { CapacitorSQLite, SQLiteConnection, SQLiteDBConnection } from '@capacitor-community/sqlite';

export class Database {
    private static instance: Database;
    private db: SQLiteDBConnection | null = null;

    private constructor() {}

    public static getInstance(): Database {
        if (!Database.instance) {
            Database.instance = new Database();
        }
        return Database.instance;
    }

    public async init() {
        try {
            const sqlite = new SQLiteConnection(CapacitorSQLite);
            this.db = await sqlite.createConnection('database', false, 'no-encryption', 1, false);
            await this.db.open();
            // TODO move to Project entity
            await this.db.execute(
                `CREATE TABLE IF NOT EXISTS project (
                    id TEXT PRIMARY KEY,
                    name TEXT NOT NULL,
                    directory TEXT NOT NULL,
                    rootDirectoryId TEXT NOT NULL
                );`
            );
        } catch (error) {
            console.error('DB init error:', error);
            throw error;
        }
    }

    public get dbConnection(): SQLiteDBConnection | null {
        return this.db;
    }
}
