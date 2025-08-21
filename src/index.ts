import { registerPlugin } from '@capacitor/core';

import type { RubyVMPlugin } from './definitions';

const RubyVM = registerPlugin<RubyVMPlugin>('RubyVM');

RubyVM.addListener('log', (info: { message: string }) => {
  console.log(info.message);
});

RubyVM.addListener('logError', (info: { error: string }) => {
  console.error(info.error);
});

export * from './definitions';
export { RubyVM };
