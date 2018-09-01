#include "utils.h"

int main(int argc, char *argv[]) {
  LOG(INFO) << std::getenv("DB_URL");
  return 0;
}
