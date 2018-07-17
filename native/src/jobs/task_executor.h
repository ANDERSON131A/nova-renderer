/*!
 * \author ddubois 
 * \date 15-Jul-18.
 */

#ifndef NOVA_RENDERER_TASK_EXECUTOR_H
#define NOVA_RENDERER_TASK_EXECUTOR_H

#include <jobxx/queue.h>

namespace nova {
    /*!
     * \brief Wrapper around jobxx::queue that provides the threads
     */
    class task_executor {
    public:
        explicit task_executor(int num_threads);
    private:
        jobxx::queue _queue;
    };
}


#endif //NOVA_RENDERER_TASK_EXECUTOR_H
