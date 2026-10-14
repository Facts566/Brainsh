// Node 18 has no global File (added in Node 20) but undici, a vsce dependency,
// requires it. Preload this with `node -r` to patch that.
if (typeof global.File === 'undefined') {
  global.File = class File extends global.Blob {
    constructor(parts, name, options = {}) {
      super(parts, options);
      this.name = name;
      this.lastModified = options.lastModified ?? Date.now();
    }
  };
}
if (typeof global.navigator === 'undefined') {
  global.navigator = { userAgent: 'node' };
}