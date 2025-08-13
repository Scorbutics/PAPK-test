
declare global {
  interface Window {
    api: {
      platform: string
    };
  }
}

window.api = {
  platform: 'web'
};

console.log('Platform:', window.api.platform);
