#include <cstdlib>
#include <fcntl.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <stdio.h>

#include "config.pb.h"

config::RendererConfig conf;

bool parse_config() {
  // Verify that the version of the library that we linked
  // against is compatible with the version of the headers we
  // compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  int fd = open("config.txt", O_RDONLY);
  if (fd < 0) {
    // TODO: Log error message.
    return false;
  }
  google::protobuf::io::FileInputStream fileInput(fd);
  fileInput.SetCloseOnDelete( true );

  if (!google::protobuf::TextFormat::Parse(&fileInput, &conf)) {
    // protobuf prints error message
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  if (!parse_config()) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
