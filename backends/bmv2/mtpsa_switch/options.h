#ifndef BACKENDS_BMV2_PSA_SWITCH_OPTIONS_H_
#define BACKENDS_BMV2_PSA_SWITCH_OPTIONS_H_

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
                "[PsaSwitch back-end] Lists exact name of all midend passes.\n");
    }
};

using MtPsaSwitchContext = P4CContextWithOptions<MtPsaSwitchOptions>;

};  // namespace BMV2

#endif /* BACKENDS_BMV2_PSA_SWITCH_OPTIONS_H_ */
