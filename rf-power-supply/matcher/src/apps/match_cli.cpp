/**
 * C++ CLI for matcher bring-up (replaces former Python host/sim).
 *
 *   ./build/match_cli --cs 200 --cp 800 --target 0.05
 */
#include "match/search.hpp"
#include "stub_actuator.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

struct Args {
  int cs{200};
  int cp{800};
  float target{0.05f};
  bool verbose{true};
};

Args parse(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--cs") == 0 && i + 1 < argc) {
      a.cs = std::atoi(argv[++i]);
    } else if (std::strcmp(argv[i], "--cp") == 0 && i + 1 < argc) {
      a.cp = std::atoi(argv[++i]);
    } else if (std::strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
      a.target = static_cast<float>(std::atof(argv[++i]));
    } else if (std::strcmp(argv[i], "--quiet") == 0) {
      a.verbose = false;
    } else if (std::strcmp(argv[i], "-h") == 0 ||
               std::strcmp(argv[i], "--help") == 0) {
      std::printf("usage: match_cli [--cs N] [--cp N] [--target G] [--quiet]\n");
      std::exit(0);
    }
  }
  return a;
}

}  // namespace

int main(int argc, char** argv) {
  using namespace rf::match;

  const Args args = parse(argc, argv);

  StubActuator hw(100.f);
  hw.setPos(args.cs, args.cp);

  Config cfg;
  cfg.target_gamma = args.target;
  cfg.settle_ms = 0;
  cfg.step0 = 32;

  Search search(hw, cfg);
  search.setMechanicsPos(args.cs, args.cp);
  search.setPreset(args.cs, args.cp);
  search.setEnable(true);

  std::printf("start Cs=%d Cp=%d |G| target=%.3f\n", args.cs, args.cp,
              args.target);

  for (int i = 0; i < 80; ++i) {
    search.tick();
    if (args.verbose) {
      std::printf("i=%02d state=%d |G|=%.3f VSWR=%.2f Cs=%d Cp=%d step=%d\n", i,
                  static_cast<int>(search.state()), search.lastGamma(),
                  search.lastVswr(), hw.pos(Axis::Cs), hw.pos(Axis::Cp),
                  search.step());
    }
    if (search.state() == State::Track) {
      std::printf("OK converged |G|=%.3f VSWR=%.2f Cs=%d Cp=%d\n",
                  search.lastGamma(), search.lastVswr(), hw.pos(Axis::Cs),
                  hw.pos(Axis::Cp));
      return 0;
    }
    if (search.state() == State::Fault) {
      std::printf("FAIL fault=0x%x |G|=%.3f\n", search.fault(),
                  search.lastGamma());
      return 1;
    }
  }

  std::printf("FAIL timeout |G|=%.3f\n", search.lastGamma());
  return 1;
}
