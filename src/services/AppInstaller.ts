import { AssetManager } from '@capawesome/capacitor-asset-manager';
import { Preferences } from '@capacitor/preferences';
import { Filesystem, Directory } from '@capacitor/filesystem';
import { BlobReader, BlobWriter, ZipReader } from '@zip.js/zip.js';
import write_blob from 'capacitor-blob-writer';

const INSTALL_NEEDED = 'APP_INSTALL_NEEDED';

export class AppInstaller {
  /**
   * List all files recursively in an asset directory
   */
  static async recursivelyListAllFiles(dirOrFile: string): Promise<string[]> {
    // Attempt to list directory contents
    try {
      const { files } = await AssetManager.list({ path: dirOrFile });
      // No files: must be a file, not a folder
      if (files.length === 0) {
        return [dirOrFile];
      }
      // Directory: recursively list contents
      let allFiles: string[] = [];
      for (const file of files) {
        const subpath = dirOrFile.endsWith('/') ? dirOrFile + file : `${dirOrFile}/${file}`;
        const subFiles = await AppInstaller.recursivelyListAllFiles(subpath);
        allFiles = allFiles.concat(subFiles);
      }
      return allFiles;
    } catch {
      // Path is a file (not a folder) or invalid
      return [dirOrFile];
    }
  }


  /**
   * Unpack/copy all game files from assets to writable location (if not already done)
   */
  static async unpackToStartGameIfRelease(
    releaseFolderInAssets: string,
    releaseLocation: string
  ): Promise<boolean> {
    const gameFiles = await AppInstaller.recursivelyListAllFiles(releaseFolderInAssets);
    if (gameFiles.length === 0) {
      return false;
    }
    for (const file of gameFiles) {
      // toPath: mirror asset subpath structure in releaseLocation
      const subFile = file.replace(`${releaseFolderInAssets}/`, '');
      const dest = releaseLocation.endsWith('/')
        ? `${releaseLocation}${subFile}`
        : `${releaseLocation}/${subFile}`;
      await AssetManager.copy({from : file, to: dest});
    }
    return true;
  }

  static async unzipFile(content: string | Blob, dest: Directory) {
        if (typeof content === 'string') {
            content = new Blob([content], { type: "text/plain" })
        }
        const reader = new ZipReader(new BlobReader(content));
        const entries = await reader.getEntries(); // Get a list of files within the ZIP

        for (const entry of entries) {
            if (!entry.directory) { // Process only files, not directories
                const blob = await entry.getData(new BlobWriter()); // Extract the file content as a Blob
                write_blob({
                    path: entry.filename, // The path within the directory
                    directory: dest,    // The root directory (e.g., app's data directory)
                    blob: blob,           // The Blob to write
                    recursive: true,              // Creates intermediate directories if needed
                    fast_mode: true,              // Recommended for web platform performance
                    on_fallback(error) {
                        console.error("Error in fallback mode:", error);
                    }
                }).then(() => {
                    console.log("Blob successfully written to file.");
                }).catch((error) => {
                    console.error("Failed to write Blob:", error);
                });
            }
        }
        await reader.close(); // Close the reader when done
    }

  /**
   * Unpacks extra assets (like a .zip) if needed, based on install flag
   */
  static async unpackExtraAssetsIfNeeded(): Promise<void> {
    const { value } = await Preferences.get({ key: INSTALL_NEEDED });
    if (value !== 'false') {
      const targetFolder = Directory.Data;
      const sourceAssetsFolder = 'public';
      const destinationZip = `${targetFolder}/ruby-stdlib.zip`;

      // 1. Copy the archive file out
      await AssetManager.copy({from: `${sourceAssetsFolder}/ruby-stdlib.zip`, to: destinationZip});

      // 2. Unzip outside of AssetManager using @zip.js
      const file = await Filesystem.readFile({ path: destinationZip })
      await AppInstaller.unzipFile(file.data, targetFolder);

      // Copy extra files if needed
      await AssetManager.copy({from: `${sourceAssetsFolder}/ruby_physfs_patch.rb`, to: `${targetFolder}/ruby_physfs_patch.rb`});

      await Preferences.set({ key: INSTALL_NEEDED, value: 'false' });
    }
  }
}
