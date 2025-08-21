#define FIFO_LOGS_NAME "psdk_fifo_logs"
#define FIFO_COMMANDS_NAME "psdk_fifo_commands"


#define MAX_PATH_LENGTH 512
#define MAX_CONTENT_SIZE 65536

#define ENV_RUBY_VM_ADDITIONAL_PARAM "RUBY_VM_ADDITIONAL_PARAM"
#define ENV_RUBY_VM_BINARY_PATH "PSDK_BINARY_PATH"
#define ENV_RUBY_VM_COMMAND_SOCKET "ANDROID_COMMAND_SOCKET"

#define FIFO_INTERPRETER_SCRIPT "require 'socket'" \
"begin" \
"  # Expect the C side (or whoever starts us) to give us the fd of the socket" \
"  # via ENV, or we can create it ourselves if fully self-contained." \
"  #" \
"  # Convention:" \
"  #   ENV['ANDROID_COMMAND_SOCKET'] = fd number for ruby side" \
"  #" \
"  ruby_fd = ENV['ANDROID_COMMAND_SOCKET'].to_i" \
"  raise 'ANDROID_COMMAND_SOCKET not set!' if ruby_fd <= 0" \
"  # Wrap the fd in an IO object" \
"  socket = IO.for_fd(ruby_fd)" \
"  loop do" \
"    # protocol: first line = command length" \
"    line = socket.gets" \
"    break if line.nil?" \
"    command = line.strip" \
"    begin" \
"      eval command + '\\n'" \
"      STDOUT.flush" \
"      socket.puts '0'" \
"    rescue Exception => error" \
"      STDOUT.flush" \
"      STDERR.puts error" \
"      STDERR.puts error.backtrace.join('\\n\\t')" \
"      socket.puts '1'" \
"    end" \
"  end" \
"rescue Exception => error" \
"  STDOUT.flush" \
"  STDERR.puts error" \
"  STDERR.puts error.backtrace.join('\\n\\t')" \
"end"
