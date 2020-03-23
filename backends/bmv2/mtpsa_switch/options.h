#ifndef BACKENDS_BMV2_MTPSA_SWITCH_OPTIONS_H_
#define BACKENDS_BMV2_MTPSA_SWITCH_OPTIONS_H_

#include "backends/bmv2/common/options.h"
#include "backends/bmv2/mtpsa_switch/midend.h"

namespace BMV2 {

class MtPsaSwitchOptions : public BMV2Options {
 public:
    MtPsaSwitchOptions() {
        registerOption("--listMidendPasses", nullptr,
                [this](const char*) {
                    listMidendPasses = true;
                    loadIRFromJson = false;
                    MtPsaSwitchMidEnd midEnd(*this, outStream);
                    exit(0);
                    return false; },
                "[MtPsaSwitch back-end] Lists exact name of all midend passes.\n");
        registerOption("--user", "user_id",
                [this](const char* arg) { userProgram = true; userProgramID = arg; return true; },
                "[MtPsaSwitch back-end] Compile user program.\n");
    }
};

using MtPsaSwitchContext = P4CContextWithOptions<MtPsaSwitchOptions>;

};  // namespace BMV2

#endif /* BACKENDS_BMV2_MTPSA_SWITCH_OPTIONS_H_ */
