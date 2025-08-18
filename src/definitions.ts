export interface RubyVMPlugin {
  echo(options: { value: string }): Promise<{ value: string }>;
}
