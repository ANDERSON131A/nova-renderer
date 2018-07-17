/*!
 * \author ddubois 
 * \date 15-Jul-18.
 */

#include "task_executor.h"

namespace nova {
    task_executor::task_executor(int num_threads) {
        _queue.work_forever();
    }
}
