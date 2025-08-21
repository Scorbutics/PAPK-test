const fs = require('fs');
const path = require('path');
const os = require('os');
const { error } = require('console');

const args = process.argv.slice(2);
const removeMode = args.includes('--remove');

const config = {
  sharedFolder: path.resolve(__dirname, '../shared/src/main'),
  targets: {
    android: {
        basepath: path.resolve(__dirname, '../android/src/main'),
        folders: {
            'assets': 'content',
            'cpp': 'content',
            'ruby': 'folder',
        }
    },
    ios: {
        disabled: true,
        basepath: path.resolve(__dirname, '../ios/Sources'),
        folders: {
            'assets': 'content',
            'cpp': 'content',
            'ruby': 'folder',
        }
    }
  },
};

if (!fs.existsSync(config.sharedFolder)) {
    throw new Error(`Shared folder does not exist: ${config.sharedFolder}`);
}

function isWindows() {
  return os.platform() === 'win32';
}

function getSymlinkType(sourcePath) {
  try {
    const stats = fs.statSync(sourcePath);
    return stats.isDirectory() ? (isWindows() ? 'junction' : 'dir') : 'file';
  } catch {
    return 'file';
  }
}

function safeSymlink(target, linkPath) {
  try {
    if (fs.existsSync(linkPath)) {
      const stats = fs.lstatSync(linkPath);
      if (stats.isSymbolicLink()) {
        const resolved = fs.readlinkSync(linkPath);
        if (resolved === target) return;
      }
      fs.rmSync(linkPath, { recursive: true, force: true });
    }
    const type = getSymlinkType(target);
    fs.symlinkSync(target, linkPath, type);
    console.log(`Symlinked: ${linkPath} → ${target} (${type})`);
  } catch (err) {
    console.error(`Failed to symlink ${linkPath} → ${target}:`, err);
    return err;
  }
}

const mappings = Object.values(config.targets)
    .filter(targetConfig => !targetConfig.disabled)
    .flatMap(targetConfig => 
        Object.entries(targetConfig.folders)
            .flatMap(([folder, linkType]) => { 
                const relativePath = path.join(targetConfig.basepath, folder);
                const relativeTarget = path.join(config.sharedFolder, folder);
                switch (linkType) {
                case 'content':
                    const files = fs.readdirSync(relativeTarget); 
                    return files.map(file => ({
                        target: path.join(relativeTarget, file),
                        linkPath: path.join(relativePath, file),
                        origin: linkType,
                    }));
                case 'folder':
                    return [{
                        target: relativeTarget,
                        linkPath: relativePath,
                        origin: linkType,
                    }];
                }
                throw new Error(`Unknown link type: ${linkType} for folder: ${folder}`);
            } ))
    .filter(mapping => !!mapping.linkPath && fs.existsSync(mapping.linkPath) !== undefined);

if (mappings.length === 0) {
    console.log('No symlinks to create. All shared folders are already linked.');
    process.exit(0);
}


// Ensure a path is listed in .gitignore, no duplicates
function ensureGitignore(entry) {
  const gitignorePath = path.resolve(__dirname, '../.gitignore');
  let lines = [];
  if (fs.existsSync(gitignorePath)) {
    lines = fs.readFileSync(gitignorePath, 'utf8')
      .split(/\r?\n/)
      .map(l => l.trim())
      .filter(l => l.length > 0);
  }
  // Normalize entry to relative path from repo root
  const repoRoot = path.resolve(__dirname, '..');
  let relEntry = path.relative(repoRoot, entry);
  // Always use forward slashes for .gitignore entries
  relEntry = relEntry.replace(/\\/g, '/');

  if (!lines.includes(relEntry)) {
    lines.push(relEntry);
    fs.writeFileSync(gitignorePath, lines.join('\n') + '\n', 'utf8');
    console.log(`Added to .gitignore: ${relEntry}`);
  }
}


function removeSymlink(linkPath) {
  try {
    if (fs.existsSync(linkPath)) {
      const stats = fs.lstatSync(linkPath);
      if (stats.isSymbolicLink()) {
        fs.rmSync(linkPath, { recursive: true, force: true });
        console.log(`Removed symlink: ${linkPath}`);
      } else {
        console.log(`Not a symlink, skipping removal: ${linkPath}`);
      }
    } else {
      // Does not exist
      console.log(`Link path does not exist: ${linkPath}`);
    }
  } catch (err) {
    console.error(`Failed to remove symlink ${linkPath}:`, err.message);
    return err;
  }
}

const errors = mappings.map(mapping => {
    const linkDir = path.dirname(mapping.linkPath);
    if (!fs.existsSync(linkDir)) {
        fs.mkdirSync(linkDir, { recursive: true });
    }

    let error;
     if (removeMode) {
        error = removeSymlink(mapping.linkPath);
      } else {
        error = safeSymlink(mapping.target, mapping.linkPath);
      }

    if (!error) {
        ensureGitignore(mapping.linkPath);
    }
    return error;
})
.filter(error => !!error);

if (errors.some(err => err)) {
    console.error('Some symlinks could not be created.');
    process.exit(1);
}
