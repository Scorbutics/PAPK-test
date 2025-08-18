import { WebPlugin } from '@capacitor/core';

import type { RubyVMPlugin } from './definitions';

export class RubyVMWeb extends WebPlugin implements RubyVMPlugin {
  async echo(options: { value: string }): Promise<{ value: string }> {
    console.log('ECHO', options);
    return options;
  }
}
