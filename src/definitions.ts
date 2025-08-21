import { PluginListenerHandle } from "@capacitor/core";

export interface RubyVMPlugin {
  create(options: { executionLocation: string, archiveLocation: string }): Promise<{ interpreter: number }>;
  addListener(eventName: 'log', listenerFunc: (data: {message: string}) => void): Promise<PluginListenerHandle>;
  addListener(eventName: 'logError', listenerFunc: (data: {error: string}) => void): Promise<PluginListenerHandle>;
  execute(options: { interpreter: number }): Promise<{ result: number}>;
  remove(options: { interpreter: number }): Promise<{ index: number } | void>;
  removeAllListeners(): Promise<void>;
}
