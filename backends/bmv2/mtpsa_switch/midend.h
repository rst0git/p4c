#ifndef BACKENDS_BMV2_MTPSA_SWITCH_MIDEND_H_
#define BACKENDS_BMV2_MTPSA_SWITCH_MIDEND_H_

#include "ir/ir.h"
#include "frontends/common/options.h"
#include "midend/convertEnums.h"
#include "backends/bmv2/common/midend.h"
#include "backends/bmv2/common/options.h"

namespace BMV2 {

class MtPsaSwitchMidEnd : public MidEnd {
 public:
    // If p4c is run with option '--listMidendPasses', outStream is used for printing passes names
    explicit MtPsaSwitchMidEnd(CompilerOptions& options, std::ostream* outStream = nullptr);
};

}  // namespace BMV2

#endif /* BACKENDS_BMV2_MTPSA_SWITCH_MIDEND_H_ */
