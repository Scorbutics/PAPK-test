import { SQLiteDBConnection } from '@capacitor-community/sqlite';
import { Project } from '../entities/Project';

export class ProjectDao {
    constructor(private db: SQLiteDBConnection) {}

    async getAllByDirectory(rootDirectoryId: string): Promise<Project[]> {
        const res = await this.db.query(
            'SELECT * FROM project WHERE rootDirectoryId = ?;',
            [rootDirectoryId]
        );
        return res.values as Project[] || [];
    }

    async findById(projectId: string): Promise<Project | null> {
        const res = await this.db.query(
            'SELECT * FROM project WHERE id = ?;',
            [projectId]
        );
        if (res.values && res.values.length > 0) {
            return res.values[0] as Project;
        }
        return null;
    }

    async insertAll(...projects: Project[]): Promise<void> {
        const stmt =
            'INSERT INTO project (id, name, directory, rootDirectoryId) VALUES (?, ?, ?, ?);';
        for (const project of projects) {
            await this.db.run(stmt, [
                project.id,
                project.name,
                project.directory,
                project.rootDirectoryId
            ]);
        }
    }

    async delete(project: Project): Promise<void> {
        await this.db.run(
            'DELETE FROM project WHERE id = ?;',
            [project.id]
        );
    }
}
