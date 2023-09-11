#include <gflags/gflags.h>

#include "tests/transport_util.hh"

#include "src/rpc/mod.hh"
#include "src/transport/rdma_ud_t.hh"

#include "r2/src/thread.hh"

#include "../load_data.hh"
#include "../reporter.hh"

#include "../../rolex/rolex_client.hpp"

namespace bench {

using namespace test;

using namespace xstore::rpc;
using namespace xstore::transport;

// prepare the sender transport
using SendTrait = UDTransport;
using RecvTrait = UDRecvTransport<2048>;
using SManager = UDSessionManager<2048>;

} // namespace bench

using namespace bench;
// using namespace xstore::bench;

DEFINE_int64(client_threads, 2, "num client thread used");
// DEFINE_int64(coros, 1, "num client coroutine used per threads");
DEFINE_string(addr, "localhost:8888", "server address");

using XThread = ::r2::Thread<usize>;

volatile bool running = true;
::xstore::util::PBarrier* bar;
std::vector<rolex::bench::Statics> statics(FLAGS_client_threads);

int main(int argc, char **argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  bar = new ::xstore::util::PBarrier(FLAGS_client_threads);

  rolex::bench::load_benchmark_config();
  rolex::load_data();

  std::vector<std::unique_ptr<XThread>> workers;


  workers = rolex::rolex_client_worker(FLAGS_client_threads);

  for (auto &w : workers) {
    w->start();
  }

  rolex::bench::Reporter::report_thpt(statics, 10);

  for (auto &w : workers) {
    w->join();
  }

  LOG(4) << "rolex client finishes";
  return 0;
}
