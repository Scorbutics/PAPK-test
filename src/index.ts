import { registerPlugin } from '@capacitor/core';

import type { RubyVMPlugin } from './definitions';

const RubyVM = registerPlugin<RubyVMPlugin>('RubyVM', {
  web: () => import('./web').then((m) => new m.RubyVMWeb()),
});

export * from './definitions';
export { RubyVM };
