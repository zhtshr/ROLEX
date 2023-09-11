#include <gflags/gflags.h>

#include "tests/transport_util.hh"

#include "src/rpc/mod.hh"

#include "src/transport/rdma_ud_t.hh"

#include "r2/src/thread.hh"

#include "rolex/rolex_server.hpp"
#include "rolex/remote_memory.hh"

#include "../load_data.hh"


volatile bool running = true;

namespace bench {

using namespace xstore::rpc;
using namespace xstore::transport;
using namespace test;

// prepare the sender transport
using SendTrait = UDTransport;
using RecvTrait = UDRecvTransport<2048>;
using SManager = UDSessionManager<2048>;



// this benchmark simply returns a null reply
void bench_callback(const Header &rpc_header, const MemBlock &args,
                   SendTrait *replyc) {
  // sanity check the requests
  ASSERT(args.sz == sizeof(u64)) << "args sz:" << args.sz;
  auto val = *((u64 *)args.mem_ptr);
  ASSERT(val == 73);
  // LOG(4) << "in tes tcallback !"; sleep(1);

  // send the reply
  RPCOp op;
  char reply_buf[64];
  op.set_msg(MemBlock(reply_buf, 64))
      .set_reply()
      .set_corid(rpc_header.cor_id)
      .add_arg<u64>(73 + 1);
  auto ret = op.execute(replyc);
  ASSERT(ret == IOCode::Ok);
}

// this benchmark simply returns a null reply
void bench_stop(const Header &rpc_header, const MemBlock &args,
                    SendTrait *replyc) {
  running = false;
}


}

using namespace ::bench;
using namespace rolex;

DEFINE_int64(server_threads, 2, "num server thread used");
DEFINE_int64(port, 8888, "Server listener (UDP) port.");
DEFINE_int64(leaf_num, 10000000, "The number of registed leaves.");

using XThread = ::r2::Thread<usize>;


::rdmaio::RCtrl* ctrl;
volatile bool init;
::xstore::util::PBarrier* bar;
rolex::rolex_t *rolex_index;

int main(int argc, char **argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<std::unique_ptr<XThread>> workers;

  init = false;
  bar = new ::xstore::util::PBarrier(FLAGS_server_threads);

  rolex::bench::load_benchmark_config();

  const usize MB = 1024 * 1024;
  ctrl = new RCtrl(FLAGS_port);

  workers = rolex_server_workers(FLAGS_server_threads);

  RM_config conf(ctrl, 1024 * MB, FLAGS_leaf_num*sizeof(leaf_t), FLAGS_reg_leaf_region, FLAGS_leaf_num);
  remote_memory_t* RM = new remote_memory_t(conf);


  load_data();
  LOG(2) << "[processing data]";
  std::sort(exist_keys.begin(), exist_keys.end());
  exist_keys.erase(std::unique(exist_keys.begin(), exist_keys.end()), exist_keys.end());
  std::sort(exist_keys.begin(), exist_keys.end());
  for(size_t i=1; i<exist_keys.size(); i++){
      assert(exist_keys[i]>=exist_keys[i-1]);
  }
  rolex_index = new rolex_t(RM, exist_keys, exist_keys);
  // rolex_index->print_data();

  RDMA_LOG(2) << "Data distribution bench server started!";
  RM->start_daemon();

  init = true;

  for (auto &w : workers) {
    w->start();
  }

  while(10);

  for (auto &w : workers) {
    w->join();
  }

  LOG(4) << "rolex server finishes";
  return 0;
}
